/* Copyright (C) 2010 The Trustees of Indiana University.                  */
/*                                                                         */
/* Use, modification and distribution is subject to the Boost Software     */
/* License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at */
/* http://www.boost.org/LICENSE_1_0.txt)                                   */
/*                                                                         */
/*  Authors: Jeremiah Willcock                                             */
/*           Andrew Lumsdaine                                              */
/*  Optimized and refactored by Kislitsyn Ilya                             */

#include "BFSGraph500RMA.h"

namespace dgmark {

	BFSGraph500Optimized::BFSGraph500Optimized(Intracomm *comm) : SearchTask(comm)
	{
	}

	BFSGraph500Optimized::BFSGraph500Optimized(const BFSGraph500Optimized& orig) : SearchTask(orig.comm)
	{
	}

	BFSGraph500Optimized::~BFSGraph500Optimized()
	{
	}

	string BFSGraph500Optimized::getName()
	{
		return "Graph500_BFS_RMA";
	}

	ParentTree* BFSGraph500Optimized::run()
	{
		log << "Running BFS (Graph500 optimized) from " << root << "\n";
		double startTime = Wtime();
		size_t parentBytesSize = (numLocalVertex + 1) * sizeof(int64_t);
		int64_t *parent = (int64_t*) Alloc_mem(parentBytesSize, INFO_NULL);
		run_bfs(root, parent);

		double taskRunTime = Wtime() - startTime;
		ParentTree *parentTree = new ParentTree(comm, root, (Vertex*) parent, graph, taskRunTime);
		log << "BFS time: " << taskRunTime << " s\n";
		return parentTree;
	}

	/* This BFS represents its queues as bitmaps and uses some data representation
	 * tricks to fit with the use of MPI one-sided operations.  It is not much
	 * faster than the standard version on the machines I have tested it on, but
	 * systems that have good RDMA hardware and good MPI one-sided implementations
	 * might get better performance from it.  This code might also be good to
	 * translate to UPC, Co-array Fortran, SHMEM, or GASNet since those systems are
	 * more designed for one-sided remote memory operations. */
	void BFSGraph500Optimized::run_bfs(Vertex root, int64_t* parent)
	{
		/* The queues (old and new) are represented as bitmaps.  Each bit in the
		 * queue bitmap says to check elts_per_queue_bit elements in the predecessor
		 * map for vertices that need to be visited.  In other words, the queue
		 * bitmap is an overapproximation of the actual queue; because MPI_Accumulate
		 * does not get any information on the result of the update, sometimes
		 * elements are also added to the bitmap when they were actually already
		 * black.  Because of this, the predecessor map needs to be checked to be
		 * sure a given vertex actually needs to be processed. */

		const Vertex numLocalVert = graph->numLocalVertex;
		const Vertex numGlobalVert = graph->numGlobalVertex;

		/* Set up a second predecessor map so we can read from one and modify the
		 * other. */
		int64_t* returnParent = parent;
		int64_t* swapParent;

		const int queueDensity = 4; //number elements in each queue element
		const int vertElementSize = sizeof(int64_t);
		const int queueWordSize = sizeof(unsigned long);
		const int queryWordBitSize = queueWordSize * CHAR_BIT;
		const uint64_t queue_nbits = (numLocalVert - 1) / queueDensity + 1;
		const uint64_t queueSize = (queue_nbits - 1) / queryWordBitSize + 1;

		size_t localVertBitSize = numLocalVert * vertElementSize;
		size_t queueBitSize = queueSize * queueWordSize;

		unsigned long *queue, *swapQueue;
		MPI_Win parentWin, swapParentWin, queueWin, swapQueueWin;

		/* 
		//MPI-3 functions
		MPI_Win_allocate(localVertBitSize, vertElementSize, MPI_INFO_NULL, MPI_COMM_WORLD, &parent, &parentWin);
		MPI_Win_allocate(localVertBitSize, vertElementSize, MPI_INFO_NULL, MPI_COMM_WORLD, &swapParent, &swapParentWin);
		MPI_Win_allocate(queueBitSize, queueWordSize, MPI_INFO_NULL, MPI_COMM_WORLD, &queue, &queueWin);
		MPI_Win_allocate(queueBitSize, queueWordSize, MPI_INFO_NULL, MPI_COMM_WORLD, &swapQueue, &swapQueueWin);
		 */
		//MPI-2 functions
		parent = (int64_t*) Alloc_mem(localVertBitSize, INFO_NULL);
		swapParent = (int64_t*) Alloc_mem(localVertBitSize, INFO_NULL);
		queue = (unsigned long*) Alloc_mem(localVertBitSize, INFO_NULL);
		swapQueue = (unsigned long*) Alloc_mem(localVertBitSize, INFO_NULL);
		MPI_Win_create(parent, localVertBitSize, vertElementSize, MPI_INFO_NULL, MPI_COMM_WORLD, &parentWin);
		MPI_Win_create(swapParent, localVertBitSize, vertElementSize, MPI_INFO_NULL, MPI_COMM_WORLD, &swapParentWin);
		MPI_Win_create(queue, queueBitSize, queueWordSize, MPI_INFO_NULL, MPI_COMM_WORLD, &queueWin);
		MPI_Win_create(swapQueue, queueBitSize, queueWordSize, MPI_INFO_NULL, MPI_COMM_WORLD, &swapQueueWin);

		memset(queue, 0, queueBitSize);

		/* List of local vertices (used as sources in MPI_Accumulate). */
		int64_t* localToGlobal = (int64_t*) Alloc_mem(localVertBitSize, INFO_NULL);
		for (int64_t i = 0; i < numLocalVert; ++i) {
			localToGlobal[i] = graph->vertexToGlobal(rank, i);
		}

		/* List of all bit masks for an unsigned long (used as sources in
		 * MPI_Accumulate). */
		unsigned long powerOfTwo[queryWordBitSize];
		for (int i = 0; i < queryWordBitSize; ++i) {
			powerOfTwo[i] = 1UL << i;
		}

		/* Coding of predecessor map:
		 * - White (not visited): nglobalverts
		 * - Grey (in queue): 0 .. numGlobalVert-1
		 * - Black (visited): -numGlobalVert .. -1 */

		/* Set initial predecessor values. */
		for (size_t i = 0; i < numLocalVert; ++i) {
			parent[i] = numGlobalVert; //white
		}

		/* Mark root as grey and add it to the queue. */
		if (graph->vertexRank(root) == rank) {
			Vertex rootLocal = graph->vertexToLocal(root);
			parent[rootLocal] = root;
			size_t queryIndex = rootLocal / queueDensity / queryWordBitSize;
			size_t queryDeg = (rootLocal / queueDensity) % queryWordBitSize;
			queue[queryIndex] |= 1UL << queryDeg;
		}

		while (1) {
			// Clear the next-level queue
			memset(swapQueue, 0, queueBitSize);

			// The pred2 array is pred with all grey vertices changed to black
			memcpy(swapParent, parent, localVertBitSize);
			for (int64_t i = 0; i < (int64_t) numLocalVert; ++i) {
				if (swapParent[i] >= 0 && swapParent[i] < numGlobalVert) {
					swapParent[i] -= numGlobalVert;
				}
			}

			/* Start one-sided operations for this level. */
			MPI_Win_fence(MPI_MODE_NOPRECEDE, swapParentWin);
			MPI_Win_fence(MPI_MODE_NOPRECEDE, swapQueueWin);

			/* Step through the words of the queue bitmap. */
			for (uint64_t wordIndex = 0; wordIndex < queueSize; ++wordIndex) {
				unsigned long queryWord = queue[wordIndex];
				/* Skip any that are all zero. */
				if (!queryWord) {
					continue;
				}
				/* Scan the bits in the word. */
				for (int bitnum = 0; bitnum < queryWordBitSize; ++bitnum) {
					// Skip unmarked blocks
					if (!((queryWord >> bitnum) & 1)) {
						continue;
					}
					size_t firstVertexInBlock = (size_t) ((wordIndex * queryWordBitSize + bitnum) * queueDensity);
					if (firstVertexInBlock >= numLocalVert) {
						break;
					}

					/* Scan the queue elements corresponding to this bit. */
					int qelem_idx;
					for (qelem_idx = 0; qelem_idx < queueDensity; ++qelem_idx) {
						size_t currentVertexLocal = firstVertexInBlock + qelem_idx;
						if (currentVertexLocal >= numLocalVert) break;
						/* Since the queue is an overapproximation, check the predecessor map
						 * to be sure this vertex is grey. */
						if (parent[currentVertexLocal] < 0 || parent[currentVertexLocal] >= numGlobalVert) {
							continue;
						}

						size_t edgeBegin = graph->getStartIndex(currentVertexLocal);
						size_t edgeEnd = graph->getEndIndex(currentVertexLocal);
						/* Walk the incident edges. */

						for (size_t edge = edgeBegin; edge < edgeEnd; ++edge) {
							int64_t nextVertex = graph->edges->at(edge)->to;
							size_t owner = graph->vertexRank(nextVertex);
							size_t shift = graph->vertexToLocal(nextVertex);

							if (owner == rank) {
								if (parent[shift] < 0) {
									continue; // if vertex is visited by me
								}
							}
							//printf("from %ld to %ld\n",localToGlobal[currentVertexLocal], nextVertex);
							//like pred[vertex] = min(current, found) in vertex owner
							MPI_Accumulate(&localToGlobal[currentVertexLocal], 1, MPI_INT64_T, owner, shift, 1, MPI_INT64_T, MPI_MIN, swapParentWin);
							//like queue2[index] |= 2^deg //mark, that vertexies from block
							int queueDeg = (shift / queueDensity) % queryWordBitSize;
							int queueIndex = shift / queueDensity / queryWordBitSize;
							MPI_Accumulate(&powerOfTwo[queueDeg], 1, MPI_UNSIGNED_LONG, owner, queueIndex, 1, MPI_UNSIGNED_LONG, MPI_BOR, swapQueueWin);
						}
					}
				}
			}
			/* End one-sided operations. */
			MPI_Win_fence(MPI_MODE_NOSUCCEED, swapQueueWin);
			MPI_Win_fence(MPI_MODE_NOSUCCEED, swapParentWin);

			/* Test if there are any elements in the next-level queue (globally); stop
			 * if none. */
			int any_set = 0;
			for (uint64_t i = 0; i < queueSize; ++i) {
				if (swapQueue[i] != 0) {
					any_set = 1;
					break;
				}
			}
			MPI_Allreduce(MPI_IN_PLACE, &any_set, 1, MPI_INT, MPI_LOR, MPI_COMM_WORLD);
			if (!any_set) {
				break;
			}

			/* Swap queues and predecessor maps. */
			{
				MPI_Win temp = queueWin;
				queueWin = swapQueueWin;
				swapQueueWin = temp;
			}
			{
				unsigned long* temp = queue;
				queue = swapQueue;
				swapQueue = temp;
			}
			{
				MPI_Win temp = parentWin;
				parentWin = swapParentWin;
				swapParentWin = temp;
			}
			{
				int64_t* temp = parent;
				parent = swapParent;
				swapParent = temp;
			}
		}

		/* Clean up the predecessor map swapping since the surrounding code does not
		 * allow the BFS to change the predecessor map pointer. */
		memcpy(returnParent, swapParent, localVertBitSize);

		MPI_Win_free(&parentWin);
		MPI_Win_free(&swapParentWin);
		MPI_Win_free(&queueWin);
		MPI_Win_free(&swapQueueWin);
		Free_mem(parent);
		Free_mem(swapParent);
		Free_mem(queue);
		Free_mem(swapQueue);
		MPI_Free_mem(localToGlobal);

		/* Change from special coding of predecessor map to the one the benchmark
		 * requires. */
		size_t i;
		for (i = 0; i < numLocalVert; ++i) {
			if (returnParent[i] < 0) {
				returnParent[i] += numGlobalVert;
			} else if (returnParent[i] == numGlobalVert) {
				returnParent[i] = numGlobalVert;
			}
		}
	}
}
