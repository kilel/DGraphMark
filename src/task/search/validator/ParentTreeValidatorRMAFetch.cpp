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
#include "ParentTreeValidatorRMAFetch.h"
#include "../../../mpi/RMAWindow.cpp" //to prevent link errors

namespace dgmark {

	ParentTreeValidatorRMAFetch::ParentTreeValidatorRMAFetch(Intracomm *comm, Graph *graph) :
	ParentTreeValidator(comm, graph)
	{
	}

	ParentTreeValidatorRMAFetch::ParentTreeValidatorRMAFetch(const ParentTreeValidatorRMAFetch& orig) :
	ParentTreeValidator(orig.comm, orig.graph)
	{
	}

	ParentTreeValidatorRMAFetch::~ParentTreeValidatorRMAFetch()
	{
	}

	bool ParentTreeValidatorRMAFetch::validateDepth(ParentTree *parentTree)
	{
		RMAWindow<Vertex> *dWin = buildDepth(parentTree);
		Vertex *depths = dWin->getData();
		bool isValid = doValidateDepth(parentTree, depths);
		dWin->clean();
		delete dWin;
		return isValid;
	}

	RMAWindow<Vertex>* ParentTreeValidatorRMAFetch::buildDepth(ParentTree *parentTree)
	{
		Graph *graph = parentTree->getInitialGraph();
		Vertex root = parentTree->getRoot();
		Vertex *parent = parentTree->getParent();
		size_t parentSize = parentTree->getParentSize();
		size_t depthsMaxValue = graph->numGlobalVertex;

		RMAWindow<Vertex> *dWin = new RMAWindow<Vertex>(comm, parentSize, VERTEX_TYPE);
		Vertex *depths = dWin->getData();

		for (size_t localVertex = 0; localVertex < parentSize; ++localVertex) {
			depths[localVertex] = depthsMaxValue;
		}
		if (graph->vertexRank(root) == rank) {
			depths[graph->vertexToLocal(root)] = 0;
		}

		while (true) {
			bool isDepthsChanged = false;
			bool errorsFound = false;

			//synchroPhase
			for (int node = 0; node < size; ++node) {
				if (node == rank) {
					for (size_t localVertex = 0; localVertex < parentSize; ++localVertex) {
						const Vertex currParent = parent[localVertex];
						const Vertex currDepth = depths[localVertex];
						const Vertex currParentRank = graph->vertexRank(currParent);
						const Vertex currParentLocal = graph->vertexToLocal(currParent);
						Vertex parentDepth;

						if (graph->vertexRank(currParent) == rank) {
							parentDepth = depths[currParentLocal];
						} else {
							dWin->sendIsFenceNeeded(true, VALIDATOR_SYNCH_TAG);
							dWin->fenceOpen(MODE_NOPUT);
							dWin->get(&parentDepth, 1, currParentRank, currParentLocal);
							dWin->fenceClose(0);
							assert(0 <= parentDepth && parentDepth <= depthsMaxValue);
						}

						if (currDepth == depthsMaxValue && parentDepth != depthsMaxValue) {
							depths[localVertex] = parentDepth + 1;
							isDepthsChanged = true;
						} else if (currDepth != depthsMaxValue && parentDepth != depthsMaxValue) {
							//detect cycle
							//root depth == 0;
							if (currDepth - parentDepth != 1 && currDepth != 0) {
								log << "\nError validating: cycle detected\n";
								errorsFound = true;
							}
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
			
			comm->Allreduce(IN_PLACE, &errorsFound, 1, BOOL, LOR);
			if (errorsFound) {
				//to create error on dephts validation step
				depths[0] = depthsMaxValue;
				break;
			}

			comm->Allreduce(IN_PLACE, &isDepthsChanged, 1, BOOL, LOR);

			if (!isDepthsChanged) {
				break;
			}
		}
		return dWin;
	}
}
