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

#include "BFSTaskRMAFetch.h"
#include "../../../mpi/RMAWindow.cpp" //to prevent link errors

namespace dgmark {

	BFSTaskRMAFetch::BFSTaskRMAFetch(Intracomm *comm) : BFSdgmark(comm)
	{
	}

	BFSTaskRMAFetch::BFSTaskRMAFetch(const BFSTaskRMAFetch& orig) : BFSdgmark(orig.comm)
	{
	}

	BFSTaskRMAFetch::~BFSTaskRMAFetch()
	{
	}

	void BFSTaskRMAFetch::open(Graph *newGraph)
	{
		SearchTask::open(newGraph);

		qWin = new RMAWindow<Vertex>(comm, getQueueSize(), VERTEX_TYPE);
		nextQWin = new RMAWindow<Vertex>(comm, getQueueSize(), VERTEX_TYPE);
		pWin = new RMAWindow<Vertex>(comm, numLocalVertex, VERTEX_TYPE);
		queue = qWin->getData();
		nextQueue = nextQWin->getData();
		parent = pWin->getData();
	}

	void BFSTaskRMAFetch::close()
	{
		qWin->clean();
		nextQWin->clean();
		pWin->clean();
		delete qWin;
		delete nextQWin;
		delete pWin;
		SearchTask::close();
	}

	string BFSTaskRMAFetch::getName()
	{
		return "dgmark_BFS_RMA_Fetch";
	}

	void BFSTaskRMAFetch::swapQueues()
	{
		//swap RMA windows
		RMAWindow<Vertex> *temp = qWin;
		qWin = nextQWin;
		nextQWin = temp;
		BFSdgmark::swapQueues();
	}

	void BFSTaskRMAFetch::performBFS()
	{
		for (int node = 0; node < size; ++node) {
			if (rank == node) {
				performBFSActualStep();
				endSynch(BFS_SYNCH_TAG);
			} else {
				performBFSSynchRMA();
			}
			comm->Barrier();
		}
	}

	void BFSTaskRMAFetch::performBFSSynchRMA()
	{
		while (true) {
			if (waitSynch(BFS_SYNCH_TAG)) {
				pWin->fenceOpen(MODE_NOPUT); //allow read parent
				pWin->fenceClose(MODE_NOSTORE);

				if (waitSynch(BFS_SYNCH_TAG)) {
					pWin->fenceOpen(MODE_NOPUT); //allow to write to the parent
					pWin->fenceClose(MODE_NOSTORE);
					nextQWin->fenceOpen(MODE_NOPUT); //allow to read queue
					nextQWin->fenceClose(MODE_NOSTORE);
					nextQWin->fenceOpen(MODE_NOPUT); //allow to put to the queue
					nextQWin->fenceClose(MODE_NOSTORE);
				}
			} else { //if fence is not neaded mode
				break;
			}
		}
	}

	inline void BFSTaskRMAFetch::processGlobalChild(Vertex currVertex, Vertex child)
	{
		Vertex childLocal = graph->vertexToLocal(child);
		const int childRank = graph->vertexRank(child);
		Vertex parentOfChild;

		//printf("%d: Getting parent of child\n", rank);
		requestSynch(true, BFS_SYNCH_TAG); //fence is needed now
		pWin->fenceOpen(MODE_NOPUT);
		pWin->get(&parentOfChild, 1, childRank, childLocal);
		pWin->fenceClose(0);

		//printf("%d: Parent of child is %ld\n", rank, parentOfChild, numLocalVertex);
		assert(0 <= parentOfChild && parentOfChild <= graph->numGlobalVertex);

		bool isInnerFenceNeeded = (parentOfChild == graph->numGlobalVertex);
		requestSynch(isInnerFenceNeeded, BFS_SYNCH_TAG); // call for inner fence if it is needed

		if (isInnerFenceNeeded) {
			//printf("%d: Putting child to the parent\n", rank);
			pWin->fenceOpen(0);
			pWin->put(&currVertex, 1, childRank, childLocal);
			pWin->fenceClose(MODE_NOSTORE);

			//Updating queue
			Vertex queueLastIndex;
			//printf("%d: Getting last queue index\n", rank);
			nextQWin->fenceOpen(MODE_NOPUT);
			nextQWin->get(&queueLastIndex, 1, childRank, 0); // get queue[0]
			nextQWin->fenceClose(0);

			//printf("%d: Last queue index is %ld\n", rank, queueLastIndex);
			assert(0 <= queueLastIndex && queueLastIndex <= getQueueSize());

			
			//printf("%d: Updating queue\n", rank);
			nextQWin->fenceOpen(0);
			nextQWin->put(&childLocal, 1, childRank, ++queueLastIndex);
			nextQWin->put(&queueLastIndex, 1, childRank, 0);
			nextQWin->fenceClose(MODE_NOSTORE);
		}
	}
}
