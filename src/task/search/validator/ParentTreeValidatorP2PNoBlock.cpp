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
#include "ParentTreeValidatorP2PNoBlock.h"

namespace dgmark {

    ParentTreeValidatorP2PNoBlock::ParentTreeValidatorP2PNoBlock(Intracomm *comm) :
    ParentTreeValidator(comm) {
    }

    ParentTreeValidatorP2PNoBlock::ParentTreeValidatorP2PNoBlock(const ParentTreeValidatorP2PNoBlock& orig) :
    ParentTreeValidator(orig.comm) {
    }

    ParentTreeValidatorP2PNoBlock::~ParentTreeValidatorP2PNoBlock() {
    }

    bool ParentTreeValidatorP2PNoBlock::validateDepth(ParentTree *parentTree) {
        Vertex *depths = buildDepth(parentTree);
        bool isValid = true;

        Graph *graph = parentTree->getInitialGraph();
        size_t parentSize = parentTree->getParentSize();
        size_t depthsMaxValue = graph->numGlobalVertex;

        for (size_t localVertex = 0; localVertex < parentSize; ++localVertex) {
            if (depths[localVertex] >= depthsMaxValue) {
                printf("\nError: depths builded not for all verticies\n");
                return false; // not all depths builded.
            }
        }

        delete[] depths;

        return isValid;
    }

    Vertex* ParentTreeValidatorP2PNoBlock::buildDepth(ParentTree *parentTree) {
        Graph * graph = parentTree->getInitialGraph();
        const Vertex root = parentTree->getRoot();
        const Vertex * const parent = parentTree->getParent();
        const size_t parentSize = parentTree->getParentSize();
        const size_t depthsMaxValue = graph->numGlobalVertex;

        Vertex *depths = new Vertex[parentSize];

        for (size_t localVertex = 0; localVertex < parentSize; ++localVertex) {
            depths[localVertex] = depthsMaxValue;
        }
        if (graph->vertexRank(root) == rank) {
            depths[graph->vertexToLocal(root)] = 0;
        }

        while (true) {
            bool isDepthsChanged = false;

            for (size_t localVertex = 0; localVertex < parentSize; ++localVertex) {
                synchAction(depths);

                Vertex currParent = parent[localVertex];
                Vertex parentDepth = getDepth(graph, depths, currParent);

                if (depths[localVertex] == depthsMaxValue && parentDepth != depthsMaxValue) {
                    depths[localVertex] = parentDepth + 1;
                    isDepthsChanged = true;
                }
                synchAction(depths);
            }

            requestSynch(true, VALIDATOR_SYNCH_END_TAG); // tell all processes, that you had been stopped;
            int endedProcesses = 1;
            while (endedProcesses < size) {
                synchAction(depths);
                while (probeSynch(VALIDATOR_SYNCH_END_TAG)) {
                    endedProcesses += 1;
                }
            }

            comm->Allreduce(IN_PLACE, &isDepthsChanged, 1, BOOL, LOR);

            if (!isDepthsChanged) {
                break;
            }
        }
        return depths;
    }

    Vertex ParentTreeValidatorP2PNoBlock::getDepth(Graph *graph, Vertex* depths, Vertex currVertex) {
        const size_t depthsMaxValue = graph->numGlobalVertex;
        Vertex currRank = graph->vertexRank(currVertex);
        Vertex currLocal = graph->vertexToLocal(currVertex);
        Vertex currDepth;

        if (currRank == rank) {
            currDepth = depths[currLocal];
        } else {
            requestSynch(true, VALIDATOR_SYNCH_TAG);
            //printf("%d: send local\n", rank);
            comm->Send(&currLocal, 1, VERTEX_TYPE, currRank, VALIDATOR_LOCAL_SEND_TAG);
            synchAction(depths);
            //printf("%d: recv depth\n", rank);
            comm->Recv(&currDepth, 1, VERTEX_TYPE, currRank, VALIDATOR_DEPTH_SEND_TAG);
            assert(0 <= currDepth && currDepth <= depthsMaxValue);
        }
        return currDepth;
    }

    void ParentTreeValidatorP2PNoBlock::synchAction(Vertex* depths) {
        //printf("%d: start synch\n", rank);
        while (probeSynch(VALIDATOR_SYNCH_TAG)) {
            Vertex currParentLocal;
            Status status;
            //printf("%d: recv local\n", rank);
            comm->Recv(&currParentLocal, 1, VERTEX_TYPE, ANY_SOURCE, VALIDATOR_LOCAL_SEND_TAG, status);
            //printf("%d: send depth\n", rank);
            comm->Send(&depths[currParentLocal], 1, VERTEX_TYPE, status.Get_source(), VALIDATOR_DEPTH_SEND_TAG);
        }
        //printf("%d: releaze synch\n", rank);
    }
}