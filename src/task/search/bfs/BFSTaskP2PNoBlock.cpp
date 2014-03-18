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

    bool BFSTaskP2PNoBlock::performBFS(Vertex *queue, Vertex *parent) {
        bool isQueueEnlarged = false;

        isQueueEnlarged = performBFSActualStep(queue, parent);

        requestSynch(true, BFS_END_SYNCH_TAG); // tell all processes, that you had been stopped;

        int endedProcesses = 1;
        while (endedProcesses < size) {
            probeBFSSynch(queue, parent);
            while (probeSynch(BFS_END_SYNCH_TAG)) {
                ++endedProcesses;
            }
        }
        comm->Barrier();

        comm->Allreduce(IN_PLACE, &isQueueEnlarged, 1, BOOL, LOR); //finds OR for "isQueueEnlarged" in all processes.
        return isQueueEnlarged;
    }

    bool BFSTaskP2PNoBlock::performBFSActualStep(Vertex *queue, Vertex *parent) {
        bool isQueueEnlarged = false;
        vector<Edge*> *edges = graph->edges;

        const size_t queueEnd = queue[1];
        while (queue[0] < queueEnd) {
            Vertex currVertex = queue[queue[0]];

            const size_t childStartindex = graph->getStartIndex(currVertex);
            const size_t childEndIndex = graph->getEndIndex(currVertex);
            for (size_t childIndex = childStartindex; childIndex < childEndIndex; ++childIndex) {
                const Vertex child = edges->at(childIndex)->to;
                const int childRank = graph->vertexRank(child);

                if (childRank == rank) {
                    isQueueEnlarged |= processLocalChild(queue, parent, graph->vertexToGlobal(currVertex), child);
                } else {
                    isQueueEnlarged |= processGlobalChild(queue, parent, graph->vertexToGlobal(currVertex), child);
                }

                probeBFSSynch(queue, parent);
            }
            probeBFSSynch(queue, parent);
            ++queue[0];
        }
        alignQueue(queue);
        return isQueueEnlarged;
    }

    bool BFSTaskP2PNoBlock::processGlobalChild(Vertex *queue, Vertex *parent, Vertex currVertex, Vertex child) {
        const Vertex childLocal = graph->vertexToLocal(child);
        const int childRank = graph->vertexRank(child);
        Vertex memory[2] = {childLocal, currVertex};

        requestSynch(true, childRank, BFS_SYNCH_TAG);
        comm->Send(&memory[0], 2, VERTEX_TYPE, childRank, BFS_DATA_TAG);

        while (!comm->Iprobe(childRank, BFS_SYNCH_2_TAG)) {
            probeBFSSynch(queue, parent);
        }
        bool isQueueEnlarged = waitSynch(BFS_SYNCH_2_TAG, childRank);
        return isQueueEnlarged;
    }

    void BFSTaskP2PNoBlock::probeBFSSynch(Vertex *queue, Vertex * parent) {
        Status status;
        Vertex memory[2] = {0};

        while (probeSynch(BFS_SYNCH_TAG, status)) {
            comm->Recv(&memory[0], 2, VERTEX_TYPE, status.Get_source(), BFS_DATA_TAG);
            bool isQueueChanged = false;
            if (parent[memory[0]] == graph->numGlobalVertex) {
                parent[memory[0]] = memory[1];
                queue[queue[1]] = memory[0];
                ++queue[1];
                isQueueChanged = true;
            }
            requestSynch(isQueueChanged, status.Get_source(), BFS_SYNCH_2_TAG);
        }
    }

}
