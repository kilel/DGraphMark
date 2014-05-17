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

	ParentTreeValidatorP2P::ParentTreeValidatorP2P(Intracomm *comm, Graph *graph) :
	ParentTreeValidator(comm, graph)
	{
	}

	ParentTreeValidatorP2P::ParentTreeValidatorP2P(const ParentTreeValidatorP2P& orig) :
	ParentTreeValidator(orig.comm, orig.graph)
	{
	}

	ParentTreeValidatorP2P::~ParentTreeValidatorP2P()
	{
	}

	bool ParentTreeValidatorP2P::validateDepth(ParentTree *parentTree)
	{
		Vertex *depths = buildDepth(parentTree);
		bool isValid = doValidateDepth(parentTree, depths);
		delete[] depths;
		return isValid;
	}

	Vertex* ParentTreeValidatorP2P::buildDepth(ParentTree *parentTree)
	{
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
			bool errorsFound = false;

			for (int node = 0; node < size; ++node) {
				if (node == rank) {
					for (size_t localVertex = 0; localVertex < parentSize; ++localVertex) {
						const Vertex currParent = parent[localVertex];
						const Vertex currDepth = depths[localVertex];
						const Vertex parentDepth = getDepth(graph, depths, currParent);

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
					endSynch(VALIDATOR_SYNCH_TAG);
				} else {
					synchAction(depths);
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
		return depths;
	}

	Vertex ParentTreeValidatorP2P::getDepth(Graph *graph, Vertex* depths, Vertex currVertex)
	{
		const size_t depthsMaxValue = graph->numGlobalVertex;
		Vertex currRank = graph->vertexRank(currVertex);
		Vertex currLocal = graph->vertexToLocal(currVertex);
		Vertex currDepth;

		if (currRank == rank) {
			currDepth = depths[currLocal];
		} else {
			requestSynch(true, currRank, VALIDATOR_SYNCH_TAG);
			comm->Send(&currLocal, 1, VERTEX_TYPE, currRank, VALIDATOR_LOCAL_SEND_TAG);
			comm->Recv(&currDepth, 1, VERTEX_TYPE, currRank, VALIDATOR_DEPTH_SEND_TAG);
			assert(0 <= currDepth && currDepth <= depthsMaxValue);
		}
	}

	void ParentTreeValidatorP2P::synchAction(Vertex* depths)
	{
		while (true) {
			if (waitSynch(VALIDATOR_SYNCH_TAG)) {
				Vertex currParentLocal;
				Status status;
				comm->Recv(&currParentLocal, 1, VERTEX_TYPE, ANY_SOURCE, VALIDATOR_LOCAL_SEND_TAG, status);
				comm->Send(&depths[currParentLocal], 1, VERTEX_TYPE, status.Get_source(), VALIDATOR_DEPTH_SEND_TAG);
			} else {
				break;
			}
		}
	}
}
