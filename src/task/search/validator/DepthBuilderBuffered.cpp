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
				buildState = buildState == buildStateError ?
					buildStateError : buildStateNextStepRequired;
			}

			if (buildState == buildStateError) {
				break;
			}

			probeSynchData();
		}

		flushBuffers();
		waitForOthersToEnd();

		if (isRecvRequestActive) {
			recvRequest.Cancel();
		}

		comm->Allreduce(IN_PLACE, &buildState, 1, SHORT, MIN);
	}

	void DepthBuilderBuffered::distributeVertexDepth(Vertex localVertex)
	{
		for (int toRank = 0; toRank < size; ++toRank) {
			const Vertex currGlobal = graph->vertexToGlobal(localVertex);
			if (toRank == rank) {
				updateDepth(currGlobal, depth[localVertex]);
				continue;
			}
			size_t &currCount = countToSend[toRank];
			Vertex *&currBuffer = sendBuffer[toRank];

			currBuffer[currCount] = currGlobal;
			currBuffer[currCount + 1] = depth[localVertex];
			currCount += elementSize;

			if (currCount == sendPackageSize) {
				sendData(toRank);
			}
		}
	}

	void DepthBuilderBuffered::processRecvData(size_t countToRead)
	{
		for (size_t index = 0; index < countToRead; index += 2) {
			const Vertex currParent = recvBuffer[index];
			const Vertex parentDepth = recvBuffer[index + 1];
			updateDepth(currParent, parentDepth);
		}
	}

	void DepthBuilderBuffered::updateDepth(Vertex currParent, Vertex parentDepth)
	{
		for (Vertex currVertex = 0; currVertex < graph->numLocalVertex; ++currVertex) {
			if (parent[currVertex] != currParent) {
				continue;
			}
			Vertex &currDepth = depth[currVertex];
			short &currState = vertexState[currVertex];

			if (currState == stateInitial) {
				currDepth = parentDepth + 1;
				currState = stateJustFilled;
			} else if (parentDepth != 0 && currDepth - parentDepth != 1) {
				buildState = buildStateError;
				printf("%d: error found cd [%ld], pd [%ld]\n", rank, currDepth, parentDepth);
			}
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

