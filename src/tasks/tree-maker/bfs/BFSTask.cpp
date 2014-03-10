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

#include "BFSTask.h"
#include "../../../base/RMAWindow.cpp" //to prevent link errors

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
        size_t parentBytesSize = (numLocalVertex + 1) * vertexBytesSize;

        /**
         * queue is a queue of vertex (local).
         * Traversed vertex adds to the end of the queue.
         * When BFS performs, it looks on the first vertex (at queue[0] index)
         * queue[0] is a start index.
         * queue[1] is an index after the end.
         */

        RMAWindow<Vertex> *qWin = new RMAWindow<Vertex>(comm, queueSize, VERTEX_TYPE);
        Vertex *queue = qWin->getData();
        queue[0] = 2;
        queue[1] = 2;

        /**
         * parent is an array, which associates vertex with it parent (global) in tree.
         * parent[root] is always must be root.
         * parent[visited] >= 0 and \<= numGlobalVertex
         * parent[initially] == numGlobalVertex
         * Note: contains local vertex only.
         */

        RMAWindow<Vertex> *pWin = new RMAWindow<Vertex>(comm, numLocalVertex, VERTEX_TYPE);
        Vertex *parent = pWin->getData();
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
        while (performBFS(qWin, pWin));

        qWin->clean();
        delete qWin;
        delete pWin;

        double taskRunTime = Wtime() - startTime;
        ParentTree *parentTree = new ParentTree(comm, root, parent, graph, taskRunTime);
        log << "BFS time: " << taskRunTime << " s\n";
        return parentTree;
    }

    bool BFSTask::performBFS(RMAWindow<Vertex> *qWin, RMAWindow<Vertex> *pWin) {
        bool isQueueEnlarged = false;

        for (int node = 0; node < size; ++node) {
            if (rank == node) {
                //BFS from current node
                isQueueEnlarged = performBFSActualStep(qWin, pWin);
            } else {
                //RMA synchronization for all other nodes
                performBFSSynchRMA(qWin, pWin);
            }
            comm->Barrier();
        }

        comm->Allreduce(IN_PLACE, &isQueueEnlarged, 1, BOOL, LOR); //finds OR for "isQueueEnlarged" in all processes.
        return isQueueEnlarged;
    }

    bool BFSTask::performBFSActualStep(RMAWindow<Vertex> *qWin, RMAWindow<Vertex> *pWin) {
        bool isQueueEnlarged = false;
        vector<Edge*> *edges = graph->edges;
        Vertex *queue = qWin->getData();

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
                    isQueueEnlarged |= processLocalChild(qWin, pWin, graph->vertexToGlobal(currVertex), child);
                } else {
                    //vertex is in the other process
                    isQueueEnlarged |= processGlobalChild(qWin, pWin, graph->vertexToGlobal(currVertex), child);
                }
            }
            ++queue[0]; // shrinking queue.
        }
        alignQueue(queue);
        pWin->sendIsFenceNeeded(false, BFS_SYNCH_TAG); //no fence is needed more.
        return isQueueEnlarged;
    }

    void BFSTask::performBFSSynchRMA(RMAWindow<Vertex> *qWin, RMAWindow<Vertex> *pWin) {
        while (true) {
            if (pWin->recvIsFenceNeeded(BFS_SYNCH_TAG)) {
                pWin->fenceOpen(MODE_NOPUT); //allow read parent
                pWin->fenceClose(MODE_NOSTORE);

                if (pWin->recvIsFenceNeeded(BFS_SYNCH_TAG)) {
                    pWin->fenceOpen(MODE_NOPUT); //allow to write to the parent
                    pWin->fenceClose(MODE_NOSTORE);
                    qWin->fenceOpen(MODE_NOPUT); //allow to read queue
                    qWin->fenceClose(MODE_NOSTORE);
                    qWin->fenceOpen(MODE_NOPUT); //allow to put to the queue
                    qWin->fenceClose(MODE_NOSTORE);
                    qWin->fenceOpen(MODE_NOPUT); //allow to accumulate queue
                    qWin->fenceClose(MODE_NOSTORE);
                }
            } else { //if fence is not neaded mode
                break;
            }
        }
    }

    bool BFSTask::processLocalChild(RMAWindow<Vertex> *qWin, RMAWindow<Vertex> *pWin, Vertex currVertex, Vertex child) {
        Vertex childLocal = graph->vertexToLocal(child);
        Vertex *queue = qWin->getData();
        Vertex *parent = pWin->getData();

        if (parent[childLocal] == graph->numGlobalVertex) {
            parent[childLocal] = currVertex;
            queue[queue[1]] = childLocal;
            ++queue[1];
            return true;
        } else {
            return false;
        }
    }

    bool BFSTask::processGlobalChild(RMAWindow<Vertex> *qWin, RMAWindow<Vertex> *pWin, Vertex currVertex, Vertex child) {
        Vertex childLocal = graph->vertexToLocal(child);
        int childRank = graph->vertexRank(child);
        Vertex parentOfChild;
        //printf("%d: Getting parent of child\n", rank);

        pWin->sendIsFenceNeeded(true, BFS_SYNCH_TAG); //fence is needed now
        pWin->fenceOpen(MODE_NOPUT);
        pWin->get(&parentOfChild, 1, childRank, childLocal);
        pWin->fenceClose(0);

        //printf("%d: Parent of child is %ld\n", rank, parentOfChild, numLocalVertex);
        assert(0 <= parentOfChild && parentOfChild <= graph->numGlobalVertex);


        bool isInnerFenceNeeded = (parentOfChild == graph->numGlobalVertex);
        pWin->sendIsFenceNeeded(isInnerFenceNeeded, BFS_SYNCH_TAG); // call for inner fence if it is needed

        if (isInnerFenceNeeded) {
            //printf("%d: Putting child to the parent\n", rank);
            pWin->fenceOpen(0);
            pWin->put(&currVertex, 1, childRank, childLocal);
            pWin->fenceClose(MODE_NOSTORE);

            //Updating queue
            Vertex queueLastIndex;
            //printf("%d: Getting last queue index\n", rank);
            qWin->fenceOpen(MODE_NOPUT);
            qWin->get(&queueLastIndex, 1, childRank, 1); // get queue[1]
            qWin->fenceClose(0);

            //printf("%d: Last queue index is %ld\n", rank, queueLastIndex);
            assert(0 <= queueLastIndex && queueLastIndex <= getQueueSize());

            //printf("%d: Putting child to the queue\n", rank);
            qWin->fenceOpen(0);
            qWin->put(&childLocal, 1, childRank, queueLastIndex);
            qWin->fenceClose(MODE_NOSTORE);

            //printf("%d: Incrementing left edge of the queue\n", rank);
            Vertex one = 1;
            qWin->fenceOpen(0);
            qWin->accumulate(&one, 1, childRank, 1, SUM); // queue[1] += 1
            qWin->fenceClose(0);

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

    Vertex BFSTask::getQueueSize() {
        return numLocalVertex * 3 + 2;
    }
}
