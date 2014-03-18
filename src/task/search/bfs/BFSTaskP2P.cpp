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

    BFSTaskP2P::BFSTaskP2P(Intracomm *comm) : BFSdgmark(comm) {
    }

    BFSTaskP2P::BFSTaskP2P(const BFSTaskP2P& orig) : BFSdgmark(orig.comm) {
    }

    BFSTaskP2P::~BFSTaskP2P() {
    }

    string BFSTaskP2P::getName() {
        return "dgmark_BFS_P2P";
    }

    ParentTree* BFSTaskP2P::run() {
        log << "Running BFS (" << getName() << ") from " << root << "\n";
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
                isQueueEnlarged = performBFSActualStep(queue, parent);
            } else {
                performBFSSynch(queue, parent);
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

            const size_t childStartindex = graph->getStartIndex(currVertex);
            const size_t childEndIndex = graph->getEndIndex(currVertex);
            for (size_t childIndex = childStartindex; childIndex < childEndIndex; ++childIndex) {
                Vertex child = edges->at(childIndex)->to;
                int childRank = graph->vertexRank(child);
                if (childRank == rank) {
                    isQueueEnlarged |= processLocalChild(queue, parent, graph->vertexToGlobal(currVertex), child);
                } else {
                    isQueueEnlarged |= processGlobalChild(queue, parent, graph->vertexToGlobal(currVertex), child);
                }
            }
            ++queue[0]; // shrinking queue.
        }
        alignQueue(queue);
        endSynch(BFS_SYNCH_TAG);
        return isQueueEnlarged;
    }

    void BFSTaskP2P::performBFSSynch(Vertex *queue, Vertex *parent) {
        Status status;
        Vertex memory[2] = {0};

        while (true) {
            if (waitSynch(BFS_SYNCH_TAG, status)) {
                comm->Recv(&memory[0], 2, VERTEX_TYPE, status.Get_source(), BFS_DATA_TAG);
                if (parent[memory[0]] == graph->numGlobalVertex) {
                    parent[memory[0]] = memory[1];
                    queue[queue[1]] = memory[0];
                    ++queue[1];
                    requestSynch(true, status.Get_source(), BFS_SYNCH_2_TAG);
                } else {
                    requestSynch(false, status.Get_source(), BFS_SYNCH_2_TAG);
                }
            } else { //if fence is not neaded mode
                break;
            }
        }
    }

    bool BFSTaskP2P::processGlobalChild(Vertex *queue, Vertex *parent, Vertex currVertex, Vertex child) {
        Vertex childLocal = graph->vertexToLocal(child);
        int childRank = graph->vertexRank(child);
        Vertex memory[2] = {childLocal, currVertex};

        requestSynch(true, childRank, BFS_SYNCH_TAG);
        comm->Send(&memory[0], 2, VERTEX_TYPE, childRank, BFS_DATA_TAG);
        bool isQueueEnlarged = waitSynch(BFS_SYNCH_2_TAG, childRank);
        return isQueueEnlarged;
    }

}
