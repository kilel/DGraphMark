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
#include <string.h>

#include "BFSTaskP2P.h"

namespace dgmark {

	BFSTaskP2P::BFSTaskP2P(Intracomm *comm) : BFSdgmark(comm)
	{
	}

	BFSTaskP2P::BFSTaskP2P(const BFSTaskP2P& orig) : BFSdgmark(orig.comm)
	{
	}

	BFSTaskP2P::~BFSTaskP2P()
	{
	}

	string BFSTaskP2P::getName()
	{
		return "dgmark_BFS_P2P";
	}

	bool BFSTaskP2P::performBFS()
	{
		bool isQueueEnlarged = false;

		for (int node = 0; node < size; ++node) {
			if (rank == node) {
				isQueueEnlarged |= performBFSActualStep();
				endSynch(BFS_SYNCH_TAG);
			} else {
				isQueueEnlarged |= performBFSSynch();
			}
			comm->Barrier();
		}

		comm->Allreduce(IN_PLACE, &isQueueEnlarged, 1, BOOL, LOR); //finds OR for "isQueueEnlarged" in all processes.
		swapQueues();
		return isQueueEnlarged;
	}

	inline bool BFSTaskP2P::processGlobalChild(Vertex currVertex, Vertex child)
	{
		Vertex childLocal = graph->vertexToLocal(child);
		int childRank = graph->vertexRank(child);
		Vertex memory[2] = {childLocal, currVertex};

		requestSynch(true, childRank, BFS_SYNCH_TAG);
		comm->Send(&memory[0], 2, VERTEX_TYPE, childRank, BFS_DATA_TAG);
		return false;
	}

	bool BFSTaskP2P::performBFSSynch()
	{
		Status status;
		Vertex memory[2] = {0};
		bool isQueueChanged = false;

		while (true) {
			if (waitSynch(BFS_SYNCH_TAG, status)) {
				comm->Recv(&memory[0], 2, VERTEX_TYPE, status.Get_source(), BFS_DATA_TAG);
				const Vertex currLocal = memory[0];
				const Vertex parentGlobal = memory[1];
				if (parent[currLocal] == graph->numGlobalVertex) {
					parent[currLocal] = parentGlobal;
					nextQueue[nextQueue[1]++] = currLocal;
					isQueueChanged = true;
				}
			} else {
				//synchronization is not neaded mode
				break;
			}
		}
		return isQueueChanged;
	}

	void BFSTaskP2P::open(Graph *newGraph)
	{
		SearchTask::open(newGraph);

		queue = new Vertex[getQueueSize()];
		nextQueue = new Vertex[getQueueSize()];
		parent = new Vertex[numLocalVertex];
	}

	void BFSTaskP2P::close()
	{
		delete queue;
		delete parent;
		SearchTask::close();
	}

}
