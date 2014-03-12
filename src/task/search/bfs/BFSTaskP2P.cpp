/*
 *   Copyright 2014 Kislitsyn Ilya
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

#include <assert.h>
#include <string.h>

#include "BFSTaskP2P.h"

namespace dgmark {

    BFSTaskP2P::BFSTaskP2P(Intracomm *comm) : SearchTask(comm) {
    }

    BFSTaskP2P::BFSTaskP2P(const BFSTaskP2P& orig) : SearchTask(orig.comm) {
    }

    BFSTaskP2P::~BFSTaskP2P() {
    }

    string BFSTaskP2P::getName() {
        return "dgmark_BFS_RMA_P2P";
    }

    ParentTree* BFSTaskP2P::run() {
        log << "Running BFS (P2P) from " << root << "\n";
        double startTime = Wtime();

        Vertex queueSize = getQueueSize();
        size_t vertexBytesSize = sizeof (Vertex);
        size_t parentBytesSize = (numLocalVertex + 1) * vertexBytesSize;

        /**
         * queue is a queue of vertex (local).
         * Traversed vertex adds to the end of the queue.
         * When BFS performs, it looks on the first vertex (at queue[0] index)
         * queue[0] is a start index.
         * queue[1] is an index after the end.
         */

        Vertex *queue = new Vertex[queueSize];
        memset(queue, 0, queueSize * vertexBytesSize);
        queue[0] = 2;
        queue[1] = 2;

        /**
         * parent is an array, which associates vertex with it parent (global) in tree.
         * parent[root] is always must be root.
         * parent[visited] >= 0 and \<= numGlobalVertex
         * parent[initially] == numGlobalVertex
         * Note: contains local vertex only.
         */

        Vertex *parent = new Vertex[numLocalVertex];
        for (size_t i = 0; i < numLocalVertex; ++i) {
            parent[i] = graph->numGlobalVertex;
        }

        if (graph->vertexRank(root) == rank) {
            //root is my vertex, put it into queue.
            Vertex rootLocal = graph->vertexToLocal(root);
            queue[queue[1]] = rootLocal;
            parent[rootLocal] = root;
            ++queue[1];
        }

        //main loop
        while (performBFS(queue, parent));

        delete queue;

        double taskRunTime = Wtime() - startTime;
        ParentTree *parentTree = new ParentTree(comm, root, parent, graph, taskRunTime);
        log << "BFS time: " << taskRunTime << " s\n";
        return parentTree;
    }

    bool BFSTaskP2P::performBFS(Vertex *queue, Vertex *parent) {
        bool isQueueEnlarged = false;

        for (int node = 0; node < size; ++node) {
            if (rank == node) {
                //BFS from current node
                isQueueEnlarged = performBFSActualStep(queue, parent);
            } else {
                //RMA synchronization for all other nodes
                performBFSSynchRMA(queue, parent);
            }
            comm->Barrier();
        }

        comm->Allreduce(IN_PLACE, &isQueueEnlarged, 1, BOOL, LOR); //finds OR for "isQueueEnlarged" in all processes.
        return isQueueEnlarged;
    }

    bool BFSTaskP2P::performBFSActualStep(Vertex *queue, Vertex *parent) {
        bool isQueueEnlarged = false;
        vector<Edge*> *edges = graph->edges;

        size_t queueEnd = queue[1];
        //while (queue[0] < queue[1]) {
        while (queue[0] < queueEnd) { //is it better?
            Vertex currVertex = queue[queue[0]];

            for (size_t childIndex = graph->getStartIndex(currVertex);
                    childIndex < graph->getEndIndex(currVertex); ++childIndex) {
                //iterate through all childs of queued vertex
                Vertex child = edges->at(childIndex)->to;
                int childRank = graph->vertexRank(child);
                //printf("%d: currVert = %ld, child = %ld (in %d), qLen = %ld\n", rank, currVertex, graph->vertexToLocal(child), childRank, queue[1]);
                if (childRank == rank) {
                    //vertex is mine
                    isQueueEnlarged |= processLocalChild(queue, parent, graph->vertexToGlobal(currVertex), child);
                } else {
                    //vertex is in the other process
                    isQueueEnlarged |= processGlobalChild(queue, parent, graph->vertexToGlobal(currVertex), child);
                }
            }
            ++queue[0]; // shrinking queue.
        }
        alignQueue(queue);
        sendSynchEnd(); //no fence is needed more.
        return isQueueEnlarged;
    }

    void BFSTaskP2P::performBFSSynchRMA(Vertex *queue, Vertex *parent) {
        while (true) {
            if (waitSynch()) {
                Vertex childLocal;
                Status status;
                comm->Recv(&childLocal, 1, VERTEX_TYPE, ANY_SOURCE, BFS_SYNCH_TAG, status);
                comm->Send(&parent[childLocal], 1, VERTEX_TYPE, status.Get_source(), BFS_SYNCH_TAG);

                if (waitSynch()) {
                    comm->Recv(&parent[childLocal], 1, VERTEX_TYPE, ANY_SOURCE, BFS_SYNCH_TAG, status);
                    queue[queue[1]] = childLocal;
                    ++queue[1];
                }
            } else { //if fence is not neaded mode
                break;
            }
        }
    }

    bool BFSTaskP2P::processLocalChild(Vertex *queue, Vertex *parent, Vertex currVertex, Vertex child) {
        Vertex childLocal = graph->vertexToLocal(child);

        if (parent[childLocal] == graph->numGlobalVertex) {
            parent[childLocal] = currVertex;
            queue[queue[1]] = childLocal;
            ++queue[1];
            return true;
        } else {
            return false;
        }
    }

    bool BFSTaskP2P::processGlobalChild(Vertex *queue, Vertex *parent, Vertex currVertex, Vertex child) {
        Vertex childLocal = graph->vertexToLocal(child);
        int childRank = graph->vertexRank(child);

        //printf("%d: Getting parent of child\n", rank);

        sendSynch(true, childRank);
        Vertex parentOfChild;
        comm->Send(&childLocal, 1, VERTEX_TYPE, childRank, BFS_SYNCH_TAG);
        comm->Recv(&parentOfChild, 1, VERTEX_TYPE, childRank, BFS_SYNCH_TAG);

        //printf("%d: Parent of child is %ld\n", rank, parentOfChild, numLocalVertex);
        assert(0 <= parentOfChild && parentOfChild <= graph->numGlobalVertex);

        bool isInnerSynchNeeded = (parentOfChild == graph->numGlobalVertex);
        sendSynch(isInnerSynchNeeded, childRank);

        if (isInnerSynchNeeded) {
            //printf("%d: Putting child to the parent\n", rank);
            comm->Send(&currVertex, 1, VERTEX_TYPE, childRank, BFS_SYNCH_TAG);
            return true; // queue is enlarged
        } else {
            return false; //queue was not enlarged
        }
    }

    void BFSTaskP2P::alignQueue(Vertex *queue) {
        for (size_t qIndex = queue[0]; qIndex < queue[1]; ++qIndex) {
            queue[qIndex - queue[0] + 2] = queue[qIndex];
        }
        queue[1] = queue[1] - queue[0] + 2;
        queue[0] = 2;
    }

    Vertex BFSTaskP2P::getQueueSize() {
        return numLocalVertex * 3 + 2;
    }

    void BFSTaskP2P::sendSynch(bool value, int toRank) {
        comm->Send(&value, 1, BOOL, toRank, BFS_SYNCH_TAG);
    }

    void BFSTaskP2P::sendSynchEnd() {
        bool value = false;
        for (int node = 0; node < size; ++node) {
            if (node != rank) {
                comm->Send(&value, 1, BOOL, node, BFS_SYNCH_TAG);
            }
        }
    }

    bool BFSTaskP2P::waitSynch() {
        bool value;
        comm->Recv(&value, 1, BOOL, ANY_SOURCE, BFS_SYNCH_TAG);
        return value;
    }
}
