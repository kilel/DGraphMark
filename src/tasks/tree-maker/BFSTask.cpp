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
        Vertex queueSize = numLocalVertex + 2;
        size_t vertexBytesSize = sizeof (Vertex);
        size_t queueBytesSize = queueSize * vertexBytesSize;
        size_t parentBytesSize = numLocalVertex * vertexBytesSize;

        /**
         * queue is a aueue of vertex. 
         * queue[0] is a start index.
         * queue[1] is an index after the end.
         */

        Vertex *queue = (Vertex*) Alloc_mem(queueBytesSize, INFO_NULL);
        Win queueWin = Win::Create(queue, queueBytesSize, vertexBytesSize, INFO_NULL, *comm);
        queue[0] = 2;
        queue[1] = 2;
        for (size_t i = 2; i < queueSize; ++i) {
            queue[i] = 0;
        }

        /**
         * parent is an array, which associates vertex with it parent in tree.
         * parent[root] is always must be root.
         * parent[visited] >= 0
         * parent[initially] == -1
         */
        Vertex *parent = (Vertex*) Alloc_mem(parentBytesSize, INFO_NULL);
        Win parentWin = MPI::Win::Create(parent, parentBytesSize, vertexBytesSize, INFO_NULL, *comm);
        for (size_t i = 0; i < numLocalVertex; ++i) {
            parent[i] = -1;
        }

        if (Utils::getVertexRank(root) == rank) {
            //root is my vertex, put it into queue.
            queue[queue[1]] = Utils::vertexToLocal(root);
            ++queue[1];
        }

        //main loop

        vector<Edge*> *edges = graph->edges;
        
        while (true) {
            bool isQueueEnlarged = false;
            

            parentWin.Fence(MODE_NOPRECEDE);
            queueWin.Fence(MODE_NOPRECEDE);

            //BFS
            size_t queueEnd = queue[1];
            for (size_t qIndex = queue[0]; qIndex < queueEnd; ++qIndex) {
                //iterate through all queue
                Vertex currVertex = queue[qIndex];
                
                for (size_t childIndex = graph->getStartIndex(currVertex);
                        childIndex < graph->getEndIndex(currVertex); ++childIndex) {
                    //iterate through all childs of queued vertex
                    
                    //check, thad toVertex in edge is not added to parent array.
                    //If it is, add to queue and to parent array.
                    
                    //if toVertex is my, check and add to queue locally
                    //If is not, check through RMA
                    
                    //If added to queue, set isQueueEnlarged with true
                    
                }
                ++queue[0]; // shrinking queue.
            }

            parentWin.Fence(MODE_NOPRECEDE);
            queueWin.Fence(MODE_NOPRECEDE);

            comm->Allreduce(IN_PLACE, &isQueueEnlarged, 1, BOOL, LOR);
            if (!isQueueEnlarged) {
                break;
            }
        }
        parentWin.Free();
        queueWin.Free();
        Free_mem(queue);


        double taskRunTime = Wtime() - startTime;
        ParentTree *parentTree = new ParentTree(comm, parent, numLocalVertex, taskRunTime);
        log << "BFS time: " << taskRunTime << " s\n";
        return parentTree;
    }
}
