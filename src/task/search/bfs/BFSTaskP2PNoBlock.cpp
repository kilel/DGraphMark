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

#include "BFSTaskP2PNoBlock.h"

namespace dgmark {

    BFSTaskP2PNoBlock::BFSTaskP2PNoBlock(Intracomm *comm) : BFSTaskP2P(comm) {
    }

    BFSTaskP2PNoBlock::BFSTaskP2PNoBlock(const BFSTaskP2PNoBlock& orig) : BFSTaskP2P(orig.comm) {
    }

    BFSTaskP2PNoBlock::~BFSTaskP2PNoBlock() {
    }

    string BFSTaskP2PNoBlock::getName() {
        return "dgmark_BFS_P2P_no_block";
    }

    bool BFSTaskP2PNoBlock::performBFS() {
        bool isQueueEnlarged = performBFSActualStep();
        requestSynch(true, BFS_END_SYNCH_TAG); // tell all processes, that you had been stopped;

        int endedProcesses = 1;
        while (endedProcesses < size) {
            isQueueEnlarged |= probeBFSSynch();
            while (probeSynch(BFS_END_SYNCH_TAG)) {
                ++endedProcesses;
            }
        }
        comm->Barrier();
        comm->Allreduce(IN_PLACE, &isQueueEnlarged, 1, BOOL, LOR); //finds OR for "isQueueEnlarged" in all processes.
        return isQueueEnlarged;
    }

    bool BFSTaskP2PNoBlock::processGlobalChild(Vertex currVertex, Vertex child) {
        const Vertex childLocal = graph->vertexToLocal(child);
        const int childRank = graph->vertexRank(child);
        Vertex memory[2] = {childLocal, currVertex};
        comm->Send(&memory[0], 2, VERTEX_TYPE, childRank, BFS_DATA_TAG);
        return false;
    }

    bool BFSTaskP2PNoBlock::probeBFSSynch() {
        Status status;
        bool isQueueChanged = false;
        Vertex memory[2] = {0};

        while (comm->Iprobe(ANY_SOURCE, BFS_DATA_TAG, status)) {
            if (status.Get_count(VERTEX_TYPE) >= 2) {
                comm->Recv(&memory[0], 2, VERTEX_TYPE, status.Get_source(), BFS_DATA_TAG);

                if (parent[memory[0]] == graph->numGlobalVertex) {
                    parent[memory[0]] = memory[1];
                    queue[queue[1]] = memory[0];
                    ++queue[1];
                    isQueueChanged = true;
                }
            }
        }
        return isQueueChanged;
    }

    void BFSTaskP2PNoBlock::endActualStepAction() {
        return;
    }

}
