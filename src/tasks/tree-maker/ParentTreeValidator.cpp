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
#include "ParentTreeValidator.h"
#include "../../base/RMAWindow.cpp" //to prevent link errors
#include "../../base/Utils.h"
#include "../../base/RMAWindow.h"

namespace dgmark {

    ParentTreeValidator::ParentTreeValidator(Intracomm *comm) : Validator(comm), log(comm) {
    }

    ParentTreeValidator::ParentTreeValidator(const ParentTreeValidator& orig) :
    Validator(orig.comm), log(orig.comm) {
    }

    ParentTreeValidator::~ParentTreeValidator() {
    }

    TaskType ParentTreeValidator::getTaskType() {
        return TaskType::PARENT_TREE;
    }

    double ParentTreeValidator::getValidationTime() {
        return validationTime;
    }

    bool ParentTreeValidator::validate(Result *taskResult) {
        log << "Validating result... ";
        double startTime = Wtime();

        if (taskResult->getTaskType() != getTaskType()) {
            log << "Error.\nInvalid taskType for result!\n";
            return false;
        }

        ParentTree *parentTree = (ParentTree*) taskResult;
        bool isValid = doValidate(parentTree);

        validationTime = Wtime() - startTime;
        if (isValid) {
            log << "Sucess\n";
        }
        log << "Validation time: " << validationTime << " s\n";

        return isValid;
    }

    bool ParentTreeValidator::doValidate(ParentTree *parentTree) {
        if (!validateRanges(parentTree)) {
            return false;
        }

        if (!validateParents(parentTree)) {
            return false;
        }

        RMAWindow<Vertex> *dWin = buildDepth(parentTree);
        bool isValid = validateDepth(parentTree, dWin);

        dWin->clean();
        delete dWin;

        return isValid;
    }

    bool ParentTreeValidator::validateRanges(ParentTree *parentTree) {
        bool isValid = true;
        size_t parentSize = parentTree->getParentSize();
        Vertex *parent = parentTree->getParent();
        Graph *graph = parentTree->getInitialGraph();

        for (size_t i = 0; i < parentSize; ++i) {
            isValid &= (0 <= parent[i] && parent[i] < graph->numGlobalVertex);
        }

        comm->Allreduce(IN_PLACE, &isValid, 1, BOOL, LAND);

        if (!isValid) {
            log << "\nError validating: some vertices are out of tree (perhabs graph is not connected)\n";
        }

        return isValid;
    }

    bool ParentTreeValidator::validateParents(ParentTree *parentTree) {
        bool isValid = true;
        size_t parentSize = parentTree->getParentSize();
        Vertex *parent = parentTree->getParent();
        Graph *graph = parentTree->getInitialGraph();
        Vertex root = parentTree->getRoot();
        Vertex rootLocal = graph->vertexToLocal(root);

        if (graph->vertexRank(root) == rank) {
            if (root != parent[rootLocal]) {
                isValid = false;
                log << "\nError validating: root parent is not root\n";
            }
        }

        if (isValid) {
            for (size_t i = 0; i < parentSize; ++i) {
                isValid &= (parent[i] != graph->vertexToGlobal(i) || i == rootLocal);
            }
        }

        comm->Allreduce(IN_PLACE, &isValid, 1, BOOL, LAND);

        if (!isValid) {
            log << "\nError validating: some vertices are self parents, or root is not a parent it itself\n";
        }

        return isValid;
    }

    bool ParentTreeValidator::validateDepth(ParentTree *parentTree, RMAWindow<Vertex> *dWin) {
        bool isValid = true;

        Graph *graph = parentTree->getInitialGraph();
        Vertex root = parentTree->getRoot();
        Vertex *parent = parentTree->getParent();
        size_t parentSize = parentTree->getParentSize();
        Vertex *depths = dWin->getData();

        for (size_t localVertex = 0; localVertex < parentSize; ++localVertex) {
            if (depths[localVertex] >= DEPTHS_MAX_VALUE) {
                printf("\nError: depths builded not for all verticies\n");
                return false; // not all depths builded.
            }
        }

        return isValid;
    }

    RMAWindow<Vertex>* ParentTreeValidator::buildDepth(ParentTree *parentTree) {
        Graph *graph = parentTree->getInitialGraph();
        Vertex root = parentTree->getRoot();
        Vertex *parent = parentTree->getParent();
        size_t parentSize = parentTree->getParentSize();

        RMAWindow<Vertex> *dWin = new RMAWindow<Vertex>(comm, parentSize, VERTEX_TYPE);
        Vertex *depths = dWin->getData();

        for (size_t localVertex = 0; localVertex < parentSize; ++localVertex) {
            depths[localVertex] = DEPTHS_MAX_VALUE;
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

                        if (graph->vertexRank(currParent) == rank) {
                            parentDepth = depths[currParentLocal];
                        } else {
                            dWin->sendIsFenceNeeded(true, VALIDATOR_SYNCH_TAG);
                            dWin->fenceOpen(MODE_NOPUT);
                            dWin->get(&parentDepth, 1, currParentRank, currParentLocal);
                            dWin->fenceClose(0);
                            assert(0 <= parentDepth && parentDepth <= DEPTHS_MAX_VALUE);
                        }

                        if (depths[localVertex] == DEPTHS_MAX_VALUE && parentDepth != DEPTHS_MAX_VALUE) {
                            depths[localVertex] = parentDepth + 1;
                            isDepthsChanged = true;
                        }
                    }
                    dWin->sendIsFenceNeeded(false, VALIDATOR_SYNCH_TAG);
                } else {
                    while (true) {
                        if (dWin->recvIsFenceNeeded(VALIDATOR_SYNCH_TAG)) {
                            dWin->fenceOpen(MODE_NOPUT);
                            dWin->fenceClose(MODE_NOSTORE);
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
        return dWin;
    }
}
