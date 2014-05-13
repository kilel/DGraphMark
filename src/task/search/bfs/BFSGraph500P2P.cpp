/* Copyright (C) 2010 The Trustees of Indiana University.                  */
/*                                                                         */
/* Use, modification and distribution is subject to the Boost Software     */
/* License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at */
/* http://www.boost.org/LICENSE_1_0.txt)                                   */
/*                                                                         */
/*  Authors: Jeremiah Willcock                                             */
/*           Andrew Lumsdaine                                              */

#include "BFSGraph500P2P.h"
#include <assert.h>

namespace dgmark {

    BFSGraph500P2P::BFSGraph500P2P(Intracomm *comm) : SearchTask(comm)
    {
    }

    BFSGraph500P2P::BFSGraph500P2P(const BFSGraph500P2P& orig) : SearchTask(orig.comm)
    {
    }

    BFSGraph500P2P::~BFSGraph500P2P()
    {
    }

    void BFSGraph500P2P::open(Graph *newGraph)
    {
	SearchTask::open(newGraph);
	const size_t nlocalverts = newGraph->numLocalVertex;
	g_oldq = (int64_t*) Alloc_mem(nlocalverts * sizeof(int64_t), INFO_NULL);
	g_newq = (int64_t*) Alloc_mem(nlocalverts * sizeof(int64_t), INFO_NULL);
	const int ulong_bits = sizeof(unsigned long) * CHAR_BIT;
	int64_t visited_size = (nlocalverts + ulong_bits - 1) / ulong_bits;
	g_visited = (unsigned long*) Alloc_mem(visited_size * sizeof(unsigned long), INFO_NULL);
	g_outgoing = (int64_t*) Alloc_mem(coalescing_size * size * 2 * sizeof(int64_t), INFO_NULL);
	g_outgoing_counts = (size_t*) Alloc_mem(size * sizeof(size_t), INFO_NULL) /* 2x actual count */;
	g_outgoing_reqs = (MPI_Request*) Alloc_mem(size * sizeof(MPI_Request), INFO_NULL);
	g_outgoing_reqs_active = (int*) Alloc_mem(size * sizeof(int), INFO_NULL);
	g_recvbuf = (int64_t*) Alloc_mem(coalescing_size * 2 * sizeof(int64_t), INFO_NULL);
    }

    void BFSGraph500P2P::close()
    {
	Free_mem(g_oldq);
	Free_mem(g_newq);
	Free_mem(g_visited);
	Free_mem(g_outgoing);
	Free_mem(g_outgoing_counts);
	Free_mem(g_outgoing_reqs);
	Free_mem(g_outgoing_reqs_active);
	Free_mem(g_recvbuf);
	SearchTask::close();
    }

    string BFSGraph500P2P::getName()
    {
	return "Graph500_BFS_RMA";
    }

    ParentTree* BFSGraph500P2P::run()
    {
	log << "Running BFS (Graph500) from " << root << "\n";
	double startTime = Wtime();
	size_t parentBytesSize = (numLocalVertex) * sizeof(int64_t);
	int64_t *parent = (int64_t*) Alloc_mem(parentBytesSize, INFO_NULL);
	run_bfs(root, parent);

	double taskRunTime = Wtime() - startTime;
	ParentTree *parentTree = new ParentTree(comm, root, (Vertex*) parent, graph, taskRunTime);
	log << "BFS time: " << taskRunTime << " s\n";
	return parentTree;
    }

    /* This version is the traditional level-synchronized BFS using two queues.  A
     * bitmap is used to indicate which vertices have been visited.  Messages are
     * sent and processed asynchronously throughout the code to hopefully overlap
     * communication with computation. */
    void BFSGraph500P2P::run_bfs(Vertex root, int64_t* pred)
    {
	const size_t nlocalverts = graph->numLocalVertex;

	/* Set up the queues. */
	int64_t* oldq = g_oldq;
	int64_t* newq = g_newq;
	size_t oldq_count = 0;
	size_t newq_count = 0;

	/* Set up the visited bitmap. */
	const int ulong_bits = sizeof(unsigned long) * CHAR_BIT;
	int64_t visited_size = (nlocalverts + ulong_bits - 1) / ulong_bits;
	unsigned long* visited = g_visited;
	memset(visited, 0, visited_size * sizeof(unsigned long));
#define SET_VISITED(v) do {visited[graph->vertexToLocal((v)) / ulong_bits] |= (1UL << (graph->vertexToLocal((v)) % ulong_bits));} while (0)
#define TEST_VISITED(v) ((visited[graph->vertexToLocal((v)) / ulong_bits] & (1UL << (graph->vertexToLocal((v)) % ulong_bits))) != 0)

	/* Set up buffers for message coalescing, MPI requests, etc. for
	 * communication. */
	const int coalescing_size = 256;
	int64_t* outgoing = g_outgoing;
	size_t* outgoing_counts = g_outgoing_counts;
	MPI_Request* outgoing_reqs = g_outgoing_reqs;
	int* outgoing_reqs_active = g_outgoing_reqs_active;
	memset(outgoing_reqs_active, 0, size * sizeof(int));
	int64_t* recvbuf = g_recvbuf;
	MPI_Request recvreq;
	int recvreq_active = 0;

	/* Termination counter for each level: this variable counts the number of
	 * ranks that have said that they are done sending to me in the current
	 * level.  This rank can stop listening for new messages when it reaches
	 * size. */
	int num_ranks_done;

	/* Set all vertices to "not visited." */
	{
	    size_t i;
	    for (i = 0; i < nlocalverts; ++i) pred[i] = -1;
	}

	/* Mark the root and put it into the queue. */
	if (graph->vertexRank(root) == rank) {
	    SET_VISITED(root);
	    pred[graph->vertexToLocal(root)] = root;
	    oldq[oldq_count++] = root;
	}

#define CHECK_MPI_REQS \
  /* Check all MPI requests and handle any that have completed. */ \
  do { \
    /* Test for incoming vertices to put onto the queue. */ \
    while (recvreq_active) { \
      int flag; \
      MPI_Status st; \
      MPI_Test(&recvreq, &flag, &st); \
      if (flag) { \
        recvreq_active = 0; \
        int count; \
        MPI_Get_count(&st, MPI_INT64_T, &count); \
        /* count == 0 is a signal from a rank that it is done sending to me
         * (using MPI's non-overtaking rules to keep that signal after all
         * "real" messages. */ \
        if (count == 0) { \
          ++num_ranks_done; \
        } else { \
          int j; \
          for (j = 0; j < count; j += 2) { \
            int64_t tgt = recvbuf[j]; \
            int64_t src = recvbuf[j + 1]; \
            /* Process one incoming edge. */ \
            assert (graph->vertexRank(tgt) == rank); \
            if (!TEST_VISITED(tgt)) { \
              SET_VISITED(tgt); \
              pred[graph->vertexToLocal(tgt)] = src; \
              newq[newq_count++] = tgt; \
            } \
          } \
        } \
        /* Restart the receive if more messages will be coming. */ \
        if (num_ranks_done < size) { \
          MPI_Irecv(recvbuf, coalescing_size * 2, MPI_INT64_T, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &recvreq); \
          recvreq_active = 1; \
        } \
      } else break; \
    } \
    /* Mark any sends that completed as inactive so their buffers can be
     * reused. */ \
    int c; \
    for (c = 0; c < size; ++c) { \
      if (outgoing_reqs_active[c]) { \
        int flag; \
        MPI_Test(&outgoing_reqs[c], &flag, MPI_STATUS_IGNORE); \
        if (flag) outgoing_reqs_active[c] = 0; \
      } \
    } \
  } while (0)

	while (1) {
	    memset(outgoing_counts, 0, size * sizeof(size_t));
	    num_ranks_done = 1; /* I never send to myself, so I'm always done */

	    /* Start the initial receive. */
	    if (num_ranks_done < size) {
		MPI_Irecv(recvbuf, coalescing_size * 2, MPI_INT64_T, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &recvreq);
		recvreq_active = 1;
	    }

	    /* Step through the current level's queue. */
	    size_t i;
	    for (i = 0; i < oldq_count; ++i) {
		CHECK_MPI_REQS;
		assert(graph->vertexRank(oldq[i]) == rank);
		assert(pred[graph->vertexToLocal(oldq[i])] >= 0
			&& pred[graph->vertexToLocal(oldq[i])] < graph->numGlobalVertex);
		int64_t src = oldq[i];
		/* Iterate through its incident edges. */
		size_t j, j_end = graph->getEndIndex(graph->vertexToLocal(oldq[i]));
		for (j = graph->getStartIndex(graph->vertexToLocal(oldq[i])); j < j_end; ++j) {
		    int64_t tgt = graph->edges->at(j)->to;
		    int owner = graph->vertexRank(tgt);
		    /* If the other endpoint is mine, update the visited map, predecessor
		     * map, and next-level queue locally; otherwise, send the target and
		     * the current vertex (its possible predecessor) to the target's owner.
		     * */
		    if (owner == rank) {
			if (!TEST_VISITED(tgt)) {
			    SET_VISITED(tgt);
			    pred[graph->vertexToLocal(tgt)] = src;
			    newq[newq_count++] = tgt;
			}
		    } else {
			while (outgoing_reqs_active[owner]) CHECK_MPI_REQS; /* Wait for buffer to be available */
			size_t c = outgoing_counts[owner];
			outgoing[owner * coalescing_size * 2 + c] = tgt;
			outgoing[owner * coalescing_size * 2 + c + 1] = src;
			outgoing_counts[owner] += 2;
			if (outgoing_counts[owner] == coalescing_size * 2) {
			    MPI_Isend(&outgoing[owner * coalescing_size * 2], coalescing_size * 2, MPI_INT64_T, owner, 0, MPI_COMM_WORLD, &outgoing_reqs[owner]);
			    outgoing_reqs_active[owner] = 1;
			    outgoing_counts[owner] = 0;
			}
		    }
		}
	    }
	    /* Flush any coalescing buffers that still have messages. */
	    int offset;
	    for (offset = 1; offset < size; ++offset) {
		int dest = (rank + offset) & (size - 1);
		if (outgoing_counts[dest] != 0) {
		    while (outgoing_reqs_active[dest]) CHECK_MPI_REQS;
		    MPI_Isend(&outgoing[dest * coalescing_size * 2], outgoing_counts[dest], MPI_INT64_T, dest, 0, MPI_COMM_WORLD, &outgoing_reqs[dest]);
		    outgoing_reqs_active[dest] = 1;
		    outgoing_counts[dest] = 0;
		}
		/* Wait until all sends to this destination are done. */
		while (outgoing_reqs_active[dest]) CHECK_MPI_REQS;
		/* Tell the destination that we are done sending to them. */
		MPI_Isend(&outgoing[dest * coalescing_size * 2], 0, MPI_INT64_T, dest, 0, MPI_COMM_WORLD, &outgoing_reqs[dest]); /* Signal no more sends */
		outgoing_reqs_active[dest] = 1;
		while (outgoing_reqs_active[dest]) CHECK_MPI_REQS;
	    }
	    /* Wait until everyone else is done (and thus couldn't send us any more
	     * messages). */
	    while (num_ranks_done < size) CHECK_MPI_REQS;

	    /* Test globally if all queues are empty. */
	    int64_t global_newq_count;
	    MPI_Allreduce(&newq_count, &global_newq_count, 1, MPI_INT64_T, MPI_SUM, MPI_COMM_WORLD);

	    /* Quit if they all are empty. */
	    if (global_newq_count == 0) break;

	    /* Swap old and new queues; clear new queue for next level. */
	    {
		int64_t* temp = oldq;
		oldq = newq;
		newq = temp;
	    }
	    oldq_count = newq_count;
	    newq_count = 0;
	}
#undef CHECK_MPI_REQS

	for (int i = 0; i < nlocalverts; ++i) if (pred[i] < 0) pred[i] = graph->numGlobalVertex;
    }

}