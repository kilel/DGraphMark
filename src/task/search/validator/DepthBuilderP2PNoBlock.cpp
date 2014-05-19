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

#include "DepthBuilderP2PNoBlock.h"
#include <assert.h>

namespace dgmark {

	DepthBuilderP2PNoBlock::DepthBuilderP2PNoBlock(Intracomm *comm, Graph *graph)
	: Communicable(comm), DepthBuilder(graph)
	{
	}

	DepthBuilderP2PNoBlock::~DepthBuilderP2PNoBlock()
	{
	}

	void DepthBuilderP2PNoBlock::buildNextStep()
	{
		//printf("%d: step\n\n", rank);
		buildState = buildStateSuccess;

		for (size_t localVertex = 0; localVertex < graph->numLocalVertex; ++localVertex) {
			const Vertex currParent = parent[localVertex];

			//was not visited
			if (currParent == graph->numGlobalVertex) {
				continue;
			}

			Vertex &currDepth = depth[localVertex];

			const Vertex parentDepth = getDepth(currParent);
			if (parentDepth == graph->numGlobalVertex) {
				continue;
			}

			if (currDepth == graph->numGlobalVertex) {
				currDepth = parentDepth + 1;
				buildState = min(buildState, buildStateNextStepRequired);
			} else if (parentDepth != 0 && currDepth - parentDepth != 1) {
				buildState = buildStateError;
				break;
			}
			synchAction();
		}

		waitForOthersToEnd();

		comm->Allreduce(IN_PLACE, &buildState, 1, SHORT, MIN);
	}

	void DepthBuilderP2PNoBlock::prepare(Vertex root)
	{
		DepthBuilder::prepare(root);

		if (graph->vertexRank(root) == rank) {
			Vertex rootLocal = graph->vertexToLocal(root);
			depth[rootLocal] = 0;
		}
	}

	void DepthBuilderP2PNoBlock::waitForOthersToEnd()
	{
		requestSynch(true, SYNCH_END_TAG); // tell all processes, that you had been stopped;
		int endedProcesses = 1;
		while (endedProcesses < size) {
			synchAction();
			while (probeSynch(SYNCH_END_TAG)) {
				++endedProcesses;
			}
		}
	}

	Vertex DepthBuilderP2PNoBlock::getDepth(Vertex tgtVertex)
	{
		const int tgtRank = graph->vertexRank(tgtVertex);
		const Vertex tgtLocal = graph->vertexToLocal(tgtVertex);
		Vertex tgtDepth;

		if (tgtRank == rank) {
			tgtDepth = depth[tgtLocal];
		} else {
			sendVertex(tgtLocal, tgtRank, LOCAL_SEND_TAG);
			Request recvReq = comm->Irecv(&tgtDepth, 1, VERTEX_TYPE, tgtRank, DEPTH_SEND_TAG);

			while (!recvReq.Test()) {
				synchAction();
			}

			assert(0 <= tgtDepth && tgtDepth <= graph->numGlobalVertex);
		}
		return tgtDepth;
	}

	void DepthBuilderP2PNoBlock::synchAction()
	{
		Status status;

		if (isRecvActive && recvRequest.Test(status)) {
			//printf("%d: write depth [ %ld] to %d\n", rank, depth[requestedVertex], status.Get_source());
			sendVertex(depth[requestedVertex], status.Get_source(), DEPTH_SEND_TAG);
			isRecvActive = false;
		}

		startRecv();
	}

	void DepthBuilderP2PNoBlock::startRecv()
	{
		if (!isRecvActive) {
			recvRequest = comm->Irecv(&requestedVertex,
						1,
						VERTEX_TYPE,
						ANY_SOURCE,
						LOCAL_SEND_TAG);
			isRecvActive = true;
		}
	}


}

