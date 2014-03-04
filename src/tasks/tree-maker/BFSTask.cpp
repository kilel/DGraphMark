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

        Vertex numLocalVertex = graph->numLocalVertex;
        Vertex queueSize = numLocalVertex * 3 + 2;
        size_t vertexBytesSize = sizeof (Vertex);
        size_t queueBytesSize = queueSize * vertexBytesSize;
        size_t parentBytesSize = (numLocalVertex + 1) * vertexBytesSize;

        /**
         * queue is a aueue of vertex (local).
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
        vector<Edge*> *edges = graph->edges;
        Vertex numLocalVertex = graph->numLocalVertex;

        pWin.Fence(MODE_NOPRECEDE);
        qWin.Fence(MODE_NOPRECEDE);

        //BFS
        size_t queueEnd = queue[1];
        for (size_t qIndex = queue[0]; qIndex < queueEnd; ++qIndex) {
            Vertex currVertex = queue[qIndex];

            for (size_t childIndex = graph->getStartIndex(currVertex);
                    childIndex < graph->getEndIndex(currVertex); ++childIndex) {
                //iterate through all childs of queued vertex
                Vertex child = edges->at(childIndex)->to;
                Vertex childLocal = Utils::vertexToLocal(child);
                int childRank = Utils::getVertexRank(child);
                printf("%d: currVert = %ld, child = %ld (in %d), qLen = %ld\n", rank, currVertex, childLocal, childRank, queue[1]);
                if (childRank == rank && parent[childLocal] == numLocalVertex) {
                    //vertex is mine
                    isQueueEnlarged = setVertexLocally(queue, parent, currVertex, child);
                } else if (childRank != rank) {
                    isQueueEnlarged = setVertexGlobally(queue, parent, qWin, pWin, currVertex, child);
                }
            }
            ++queue[0]; // shrinking queue.
        }

        alignQueue(queue);

        qWin.Fence(MODE_NOSUCCEED);
        pWin.Fence(MODE_NOSUCCEED);

        comm->Allreduce(IN_PLACE, &isQueueEnlarged, 1, BOOL, LOR);
        return isQueueEnlarged;
    }

    bool BFSTask::setVertexLocally(Vertex *queue, Vertex *parent, Vertex currVertex, Vertex child) {
        Vertex childLocal = Utils::vertexToLocal(child);
        parent[childLocal] = currVertex;
        queue[queue[1]] = childLocal;
        ++queue[1];
        return true;
    }

    bool BFSTask::setVertexGlobally(Vertex *queue, Vertex *parent, Win qWin, Win pWin, Vertex currVertex, Vertex child) {
        Vertex childLocal = Utils::vertexToLocal(child);
        int childRank = Utils::getVertexRank(child);
        Vertex numLocalVertex = graph->numLocalVertex;

        Vertex parentOfChild;
        printf("%d: Getting parent of child\n", rank);

        pWin.Get(&parentOfChild, 1, VERTEX_TYPE, childRank, childLocal, 1, VERTEX_TYPE);

        printf("%d: Parent of child is %ld\n", rank, parentOfChild, numLocalVertex);
        assert(0 <= parentOfChild && parentOfChild <= numLocalVertex);
        if (parentOfChild == numLocalVertex) {
            printf("%d: putting child to parent\n", rank);
            pWin.Put(&childLocal, 1, VERTEX_TYPE, childRank, childLocal, 1, VERTEX_TYPE);

            Vertex queueLastIndex;
            printf("%d: Getting last queue index\n", rank);
            qWin.Get(&queueLastIndex, 1, VERTEX_TYPE, childRank, 1, 1, VERTEX_TYPE);
            printf("%d: Last queue index is %ld\n", rank, queueLastIndex);
            assert(0 <= queueLastIndex && queueLastIndex <= numLocalVertex * 2);
            printf("%d: putting information to it\n", rank);
            qWin.Put(&childLocal, 1, VERTEX_TYPE, childRank, queueLastIndex, 1, VERTEX_TYPE);
            Vertex one = 1;
            printf("%d: Adding one to left edge of queue\n", rank);
            qWin.Accumulate(&one, 1, VERTEX_TYPE, childRank, 1, 1, VERTEX_TYPE, SUM);

            return true;
        } else {
            return false;
        }
    }

    void BFSTask::alignQueue(Vertex *queue) {
        for (size_t qIndex = queue[0]; qIndex < queue[1]; ++qIndex) {
            queue[qIndex - queue[0] + 2] = queue[qIndex];
        }
        queue[1] = queue[1] - queue[0] + 2;
        queue[0] = 2;
    }
}
