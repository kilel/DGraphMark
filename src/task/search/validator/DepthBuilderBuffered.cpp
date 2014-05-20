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

#include "DepthBuilderBuffered.h"

namespace dgmark {

	DepthBuilderBuffered::DepthBuilderBuffered(Intracomm *comm,
						Graph *graph) :
	BufferedDataDistributor(comm,
				ELEMENT_SIZE,
				BUFFERED_ELEMENTS),
	DepthBuilder(graph)
	{
		vertexState = new short[graph->numLocalVertex];
	}

	DepthBuilderBuffered::~DepthBuilderBuffered()
	{
		delete[] vertexState;
	}

	void DepthBuilderBuffered::buildNextStep()
	{
		prepareBuffers();
		buildState = buildStateSuccess;

		//main action
		for (Vertex localVertex = 0; localVertex < graph->numLocalVertex; ++localVertex) {
			if (vertexState[localVertex] == stateJustFilled) {
				//printf("%d: dist vertex, depth [%ld]\n", rank, depth[localVertex]);
				distributeVertexDepth(localVertex);
				vertexState[localVertex] = stateSent;
				buildState = min(buildState, buildStateNextStepRequired);
			}

			if (buildState == buildStateError) {
				break;
			}

			probeSynchData();
		}

		flushBuffers();
		waitForOthersToEnd();

		comm->Allreduce(IN_PLACE, &buildState, 1, SHORT, MIN);
	}

	void DepthBuilderBuffered::distributeVertexDepth(Vertex localVertex)
	{
		const size_t startIndex = csrGraph->getStartIndex(localVertex);
		const size_t endIndex = csrGraph->getEndIndex(localVertex);
		for (size_t childIndex = startIndex; childIndex < endIndex; ++childIndex) {
			const Vertex child = csrGraph->edges->at(childIndex)->to;
			const Vertex childLocal = csrGraph->vertexToLocal(child);
			const Vertex childRank = csrGraph->vertexRank(child);
			const Vertex currGlobal = csrGraph->vertexToGlobal(localVertex);

			if (childRank == rank) {
				updateDepth(currGlobal, childLocal, depth[localVertex] + 1);
				continue;
			} else {

				while (isSendRequestActive[childRank]) {
					probeSynchData();
				}

				size_t &currCount = countToSend[childRank];
				Vertex *&currBuffer = sendBuffer[childRank];

				currBuffer[currCount] = currGlobal;
				currBuffer[currCount + 1] = childLocal;
				currBuffer[currCount + 2] = depth[localVertex] + 1;
				currCount += elementSize;

				if (currCount == sendPackageSize) {
					sendData(childRank);
				}
			}
		}
	}

	void DepthBuilderBuffered::processRecvData(size_t countToRead)
	{
		for (size_t index = 0; index < countToRead; index += elementSize) {
			const Vertex parentGlobal = recvBuffer[index];
			const Vertex localChild = recvBuffer[index + 1];
			const Vertex newDepth = recvBuffer[index + 2];
			updateDepth(parentGlobal, localChild, newDepth);
		}
	}

	void DepthBuilderBuffered::updateDepth(Vertex parentGlobal, Vertex localVertex, Vertex newDepth)
	{
		if (parent[localVertex] != parentGlobal) {
			return;
		}

		Vertex &currDepth = depth[localVertex];
		short &currState = vertexState[localVertex];

		if (currState == stateInitial) {
			currDepth = newDepth;
			currState = stateJustFilled;
		} else if (newDepth != 1 && currDepth != newDepth) { // depth == 1 if parent is root.
			buildState = buildStateError;
			printf("%d: depth of %ld  is %ld. New is %ld\n", rank, localVertex, currDepth, newDepth);
		}
	}

	void DepthBuilderBuffered::prepare(Vertex root)
	{
		DepthBuilder::prepare(root);

		for (Vertex localVertex = 0; localVertex < graph->numLocalVertex; ++localVertex) {
			vertexState[localVertex] = stateInitial;
		}

		if (graph->vertexRank(root) == rank) {
			Vertex rootLocal = graph->vertexToLocal(root);
			depth[rootLocal] = 0;
			vertexState[rootLocal] = stateJustFilled;
		}
	}

}

