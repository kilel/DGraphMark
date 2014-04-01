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

#include "BFSdgmark.h"

namespace dgmark {

    BFSdgmark::BFSdgmark(Intracomm *comm) : SearchTask(comm) {
    }

    BFSdgmark::BFSdgmark(const BFSdgmark& orig) : SearchTask(orig.comm) {
    }

    BFSdgmark::~BFSdgmark() {
    }

    ParentTree* BFSdgmark::run() {
        log << "Running BFS (" << getName() << ") from " << root << "\n";
        double startTime = Wtime();

        resetQueueParent();

        if (graph->vertexRank(root) == rank) {
            //root is my vertex, put it into queue.
            Vertex rootLocal = graph->vertexToLocal(root);
            queue[queue[1]] = rootLocal;
            parent[rootLocal] = root;
            ++queue[1];
        }

        //main loop
        while (performBFS());

        double taskRunTime = Wtime() - startTime;

        Vertex *resultParent = new Vertex[numLocalVertex];
        for (int i = 0; i < numLocalVertex; ++i) {
            resultParent[i] = parent[i];
        }

        ParentTree *parentTree = new ParentTree(comm, root, resultParent, graph, taskRunTime);
        log << "BFS time: " << taskRunTime << " s\n";
        return parentTree;
    }

    bool BFSdgmark::performBFSActualStep() {
        bool isQueueEnlarged = false;
        vector<Edge*> *edges = graph->edges;

        const size_t queueEnd = queue[1];
        while (queue[0] < queueEnd) {
            Vertex currVertex = queue[queue[0]];

            const size_t childStartIndex = graph->getStartIndex(currVertex);
            const size_t childEndIndex = graph->getEndIndex(currVertex);
            for (size_t childIndex = childStartIndex; childIndex < childEndIndex; ++childIndex) {
                const Vertex child = edges->at(childIndex)->to;
                const int childRank = graph->vertexRank(child);

                if (childRank == rank) {
                    isQueueEnlarged |= processLocalChild(graph->vertexToGlobal(currVertex), child);
                } else {
                    isQueueEnlarged |= processGlobalChild(graph->vertexToGlobal(currVertex), child);
                }

                isQueueEnlarged |= probeBFSSynch();
            }
            ++queue[0];
        }
        alignQueue();
        endActualStepAction();
        return isQueueEnlarged;
    }

    bool BFSdgmark::processLocalChild(Vertex currVertex, Vertex child) {
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

    void BFSdgmark::alignQueue() {
        for (size_t qIndex = queue[0]; qIndex < queue[1]; ++qIndex) {
            queue[qIndex - queue[0] + 2] = queue[qIndex];
        }
        queue[1] = queue[1] - queue[0] + 2;
        queue[0] = 2;
    }

    void BFSdgmark::resetQueueParent() {
        memset(queue, 0, getQueueSize() * sizeof (Vertex));
        queue[0] = 2;
        queue[1] = 2;

        for (size_t i = 0; i < numLocalVertex; ++i) {
            parent[i] = graph->numGlobalVertex;
        }
    }

    Vertex BFSdgmark::getQueueSize() {
        return numLocalVertex * 3 + 2;
    }

}
