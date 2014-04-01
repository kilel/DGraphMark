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

#include "BFSTaskRMAFetch.h"
#include "../../../mpi/RMAWindow.cpp" //to prevent link errors

namespace dgmark {

    BFSTaskRMAFetch::BFSTaskRMAFetch(Intracomm *comm) : BFSdgmark(comm) {
    }

    BFSTaskRMAFetch::BFSTaskRMAFetch(const BFSTaskRMAFetch& orig) : BFSdgmark(orig.comm) {
    }

    BFSTaskRMAFetch::~BFSTaskRMAFetch() {
    }

    void BFSTaskRMAFetch::open(Graph *newGraph) {
        SearchTask::open(newGraph);

        qWin = new RMAWindow<Vertex>(comm, getQueueSize(), VERTEX_TYPE);
        pWin = new RMAWindow<Vertex>(comm, numLocalVertex, VERTEX_TYPE);
        queue = qWin->getData();
        parent = pWin->getData();
    }

    void BFSTaskRMAFetch::close() {
        qWin->clean();
        pWin->clean();
        delete qWin;
        delete pWin;
        SearchTask::close();
    }

    string BFSTaskRMAFetch::getName() {
        return "dgmark_BFS_RMA_Fetch";
    }

    bool BFSTaskRMAFetch::performBFS() {
        bool isQueueEnlarged = false;

        for (int node = 0; node < size; ++node) {
            if (rank == node) {
                isQueueEnlarged = performBFSActualStep();
            } else {
                performBFSSynchRMA();
            }
            comm->Barrier();
        }

        comm->Allreduce(IN_PLACE, &isQueueEnlarged, 1, BOOL, LOR); //finds OR for "isQueueEnlarged" in all processes.
        return isQueueEnlarged;
    }

    void BFSTaskRMAFetch::performBFSSynchRMA() {
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

    bool BFSTaskRMAFetch::processGlobalChild(Vertex currVertex, Vertex child) {
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

    bool BFSTaskRMAFetch::probeBFSSynch() {
        return false;
    }

    void BFSTaskRMAFetch::endActualStepAction() {
        endSynch(BFS_SYNCH_TAG);
    }
}
