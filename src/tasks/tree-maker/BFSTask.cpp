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

#include <string.h>
#include <assert.h>

#include "BFSTask.h"
#include "../../base/Utils.h"

namespace dgmark {

    BFSTask::BFSTask(Intracomm *comm) : TreeMakerTask(comm) {
    }

    BFSTask::BFSTask(const BFSTask& orig) : TreeMakerTask(orig.comm) {
    }

    BFSTask::~BFSTask() {
    }

    ParentTree* BFSTask::run() {
        log << "Running BFS from " << root << "\n";
        double startTime = Wtime();

        Vertex queueSize = getQueueSize();
        size_t vertexBytesSize = sizeof (Vertex);
        size_t queueBytesSize = queueSize * vertexBytesSize;
        size_t parentBytesSize = (numLocalVertex + 1) * vertexBytesSize;

        /**
         * queue is a queue of vertex (local).
         * Traversed vertex adds to the end of the queue.
         * When BFS performs, it looks on the first vertex (at queue[0] index)
         * queue[0] is a start index.
         * queue[1] is an index after the end.
         */

        Vertex *queue = (Vertex*) Alloc_mem(queueBytesSize, INFO_NULL);
        Win queueWin = Win::Create(queue, queueBytesSize, vertexBytesSize, INFO_NULL, *comm);
        memset(queue, 0, queueBytesSize);
        queue[0] = 2;
        queue[1] = 2;

        /**
         * parent is an array, which associates vertex with it parent in tree.
         * parent[root] is always must be root.
         * parent[visited] >= 0 and \<= numLocalVertex
         * parent[initially] == numLocalVertex
         * Note: contains local vertex only.
         */
        Vertex *parent = (Vertex*) Alloc_mem(parentBytesSize, INFO_NULL);
        Win parentWin = MPI::Win::Create(parent, parentBytesSize, vertexBytesSize, INFO_NULL, *comm);
        memset(parent, 0, parentBytesSize);
        for (size_t i = 0; i < numLocalVertex; ++i) {
            parent[i] = numLocalVertex;
        }

        if (Utils::getVertexRank(root) == rank) {
            //root is my vertex, put it into queue.
            queue[queue[1]] = Utils::vertexToLocal(root);
            ++queue[1];
        }

        //main loop
        while (performBFS(queue, parent, queueWin, parentWin));

        parentWin.Free();
        queueWin.Free();
        Free_mem(queue);

        double taskRunTime = Wtime() - startTime;
        ParentTree *parentTree = new ParentTree(comm, parent, numLocalVertex, taskRunTime);
        log << "BFS time: " << taskRunTime << " s\n";
        return parentTree;
    }

    bool BFSTask::performBFS(Vertex *queue, Vertex *parent, Win qWin, Win pWin) {
        bool isQueueEnlarged = false;

        for (int node = 0; node < size; ++node) {
            if (rank == node) {
                //BFS from current node
                isQueueEnlarged = performBFSActualStep(queue, parent, qWin, pWin);
            } else {
                //RMA synchronization for all other nodes
                performBFSSynchRMA(qWin, pWin);
            }
            comm->Barrier();
        }

        comm->Allreduce(IN_PLACE, &isQueueEnlarged, 1, BOOL, LOR); //finds OR for "isQueueEnlarged" in all processes.
        return isQueueEnlarged;
    }

    bool BFSTask::performBFSActualStep(Vertex* queue, Vertex* parent, Win qWin, Win pWin) {
        bool isQueueEnlarged = false;
        vector<Edge*> *edges = graph->edges;

        size_t queueEnd = queue[1];
        while (queue[0] < queueEnd) {
            Vertex currVertex = queue[queue[0]];

            for (size_t childIndex = graph->getStartIndex(currVertex);
                    childIndex < graph->getEndIndex(currVertex); ++childIndex) {
                //iterate through all childs of queued vertex
                Vertex child = edges->at(childIndex)->to;
                int childRank = Utils::getVertexRank(child);
                //printf("%d: currVert = %ld, child = %ld (in %d), qLen = %ld\n", rank, currVertex, Utils::vertexToLocal(child), childRank, queue[1]);
                if (childRank == rank) {
                    //vertex is mine
                    isQueueEnlarged |= processLocalChild(queue, parent, currVertex, child);
                } else {
                    //vertex is in the other process
                    isQueueEnlarged |= processGlobalChild(queue, parent, qWin, pWin, currVertex, child);
                }
            }
            ++queue[0]; // shrinking queue.
        }
        alignQueue(queue);
        sendIsFenceNeeded(false); //no fence is needed more.
        return isQueueEnlarged;
    }

    void BFSTask::performBFSSynchRMA(Win qWin, Win pWin) {
        while (true) {
            if (recvIsFenceNeeded()) {
                pWin.Fence(MODE_NOPUT | MODE_NOPRECEDE); //allow read parent
                pWin.Fence(MODE_NOSTORE | MODE_NOSUCCEED);

                if (recvIsFenceNeeded()) {
                    pWin.Fence(MODE_NOPUT | MODE_NOPRECEDE); //allow to write to the parent
                    pWin.Fence(MODE_NOSTORE | MODE_NOSUCCEED);
                    qWin.Fence(MODE_NOPUT | MODE_NOPRECEDE); //allow to read queue
                    qWin.Fence(MODE_NOSTORE | MODE_NOSUCCEED);
                    qWin.Fence(MODE_NOPUT | MODE_NOPRECEDE); //allow to put to the queue
                    qWin.Fence(MODE_NOSTORE | MODE_NOSUCCEED);
                    qWin.Fence(MODE_NOPUT | MODE_NOPRECEDE); //allow to accumulate queue
                    qWin.Fence(MODE_NOSTORE | MODE_NOSUCCEED);
                }
            } else { //if fence is not neaded mode
                break;
            }
        }
    }

    bool BFSTask::processLocalChild(Vertex *queue, Vertex *parent, Vertex currVertex, Vertex child) {
        Vertex childLocal = Utils::vertexToLocal(child);
        if (parent[childLocal] == numLocalVertex) {
            parent[childLocal] = currVertex;
            queue[queue[1]] = childLocal;
            ++queue[1];
            return true;
        } else {
            return false;
        }
    }

    bool BFSTask::processGlobalChild(Vertex *queue, Vertex *parent, Win qWin, Win pWin, Vertex currVertex, Vertex child) {
        Vertex childLocal = Utils::vertexToLocal(child);
        int childRank = Utils::getVertexRank(child);
        Vertex parentOfChild;
        //printf("%d: Getting parent of child\n", rank);

        sendIsFenceNeeded(true); //fence is needed now
        pWin.Fence(MODE_NOPRECEDE | MODE_NOPUT);
        pWin.Get(&parentOfChild, 1, VERTEX_TYPE, childRank, childLocal, 1, VERTEX_TYPE);
        pWin.Fence(MODE_NOSUCCEED);

        //printf("%d: Parent of child is %ld\n", rank, parentOfChild, numLocalVertex);
        assert(0 <= parentOfChild && parentOfChild <= numLocalVertex);


        bool isInnerFenceNeeded = (parentOfChild == numLocalVertex);
        sendIsFenceNeeded(isInnerFenceNeeded); // call for inner fence if it is needed

        if (isInnerFenceNeeded) {
            //printf("%d: Putting child to the parent\n", rank);
            pWin.Fence(MODE_NOPRECEDE);
            pWin.Put(&childLocal, 1, VERTEX_TYPE, childRank, childLocal, 1, VERTEX_TYPE);
            pWin.Fence(MODE_NOSUCCEED | MODE_NOSTORE);

            //Updating queue
            Vertex queueLastIndex;
            //printf("%d: Getting last queue index\n", rank);
            qWin.Fence(MODE_NOPRECEDE | MODE_NOPUT);
            qWin.Get(&queueLastIndex, 1, VERTEX_TYPE, childRank, 1, 1, VERTEX_TYPE);
            qWin.Fence(MODE_NOSUCCEED);

            //printf("%d: Last queue index is %ld\n", rank, queueLastIndex);
            assert(0 <= queueLastIndex && queueLastIndex <= getQueueSize());

            //printf("%d: Putting child to the queue\n", rank);
            qWin.Fence(MODE_NOPRECEDE);
            qWin.Put(&childLocal, 1, VERTEX_TYPE, childRank, queueLastIndex, 1, VERTEX_TYPE);
            qWin.Fence(MODE_NOSUCCEED | MODE_NOSTORE);

            //printf("%d: Incrementing left edge of the queue\n", rank);
            Vertex one = 1;
            qWin.Fence(MODE_NOPRECEDE);
            qWin.Accumulate(&one, 1, VERTEX_TYPE, childRank, 1, 1, VERTEX_TYPE, SUM);
            qWin.Fence(MODE_NOSUCCEED);

            return true; // queue is enlarged
        } else {
            return false; //queue was not enlarged
        }
    }

    void BFSTask::alignQueue(Vertex *queue) {
        for (size_t qIndex = queue[0]; qIndex < queue[1]; ++qIndex) {
            queue[qIndex - queue[0] + 2] = queue[qIndex];
        }
        queue[1] = queue[1] - queue[0] + 2;
        queue[0] = 2;
    }

    void BFSTask::sendIsFenceNeeded(bool value) {
        for (int node = 0; node < size; ++node) {
            if (rank != node) {
                comm->Send(&value, 1, BOOL, node, RMA_SYNCH_TAG);
            }
        }
    }

    bool BFSTask::recvIsFenceNeeded() {
        bool value;
        comm->Recv(&value, 1, BOOL, ANY_SOURCE, RMA_SYNCH_TAG);
        return value;
    }

    Vertex BFSTask::getQueueSize() {
        return numLocalVertex * 3 + 2;
    }
}
