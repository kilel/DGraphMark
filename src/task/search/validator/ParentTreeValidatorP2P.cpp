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
#include "ParentTreeValidatorP2P.h"

namespace dgmark {

    ParentTreeValidatorP2P::ParentTreeValidatorP2P(Intracomm *comm) :
    ParentTreeValidator(comm) {
    }

    ParentTreeValidatorP2P::ParentTreeValidatorP2P(const ParentTreeValidatorP2P& orig) :
    ParentTreeValidator(orig.comm) {
    }

    ParentTreeValidatorP2P::~ParentTreeValidatorP2P() {
    }

    bool ParentTreeValidatorP2P::validateDepth(ParentTree *parentTree) {
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

    Vertex* ParentTreeValidatorP2P::buildDepth(ParentTree *parentTree) {
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

            //synchroPhase
            for (int node = 0; node < size; ++node) {
                if (node == rank) {
                    for (size_t localVertex = 0; localVertex < parentSize; ++localVertex) {
                        Vertex currParent = parent[localVertex];
                        Vertex currParentRank = graph->vertexRank(currParent);
                        Vertex currParentLocal = graph->vertexToLocal(currParent);
                        Vertex parentDepth;

                        if (currParentRank == rank) {
                            parentDepth = depths[currParentLocal];
                        } else {
                            requestSynch(true, VALIDATOR_SYNCH_TAG);
                            comm->Send(&currParentLocal, 1, VERTEX_TYPE, currParentRank, VALIDATOR_SYNCH_TAG);
                            comm->Recv(&parentDepth, 1, VERTEX_TYPE, currParentRank, VALIDATOR_SYNCH_TAG);
                            assert(0 <= parentDepth && parentDepth <= depthsMaxValue);
                        }

                        if (depths[localVertex] == depthsMaxValue && parentDepth != depthsMaxValue) {
                            depths[localVertex] = parentDepth + 1;
                            isDepthsChanged = true;
                        }
                    }
                    endSynch(VALIDATOR_SYNCH_TAG);
                } else {
                    while (true) {
                        if (waitSynch(VALIDATOR_SYNCH_TAG)) {
                            Vertex currParentLocal;
                            Status status;
                            comm->Recv(&currParentLocal, 1, VERTEX_TYPE, ANY_SOURCE, VALIDATOR_SYNCH_TAG, status);
                            comm->Send(&depths[currParentLocal], 1, VERTEX_TYPE, status.Get_source(), VALIDATOR_SYNCH_TAG);
                        } else {
                            break;
                        }
                    }
                }
                comm->Barrier();
            }

            comm->Allreduce(IN_PLACE, &isDepthsChanged, 1, BOOL, LOR);

            if (!isDepthsChanged) {
                break;
            }
        }
        return depths;
    }
}
