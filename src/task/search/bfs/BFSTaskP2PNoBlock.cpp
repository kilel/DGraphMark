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

#include "BFSTaskP2PNoBlock.h"
#include "BFSdgmark.h"

namespace dgmark {

	BFSTaskP2PNoBlock::BFSTaskP2PNoBlock(Intracomm *comm) : BFSTaskP2P(comm)
	{
	}

	BFSTaskP2PNoBlock::BFSTaskP2PNoBlock(const BFSTaskP2PNoBlock& orig) : BFSTaskP2P(orig.comm)
	{
	}

	BFSTaskP2PNoBlock::~BFSTaskP2PNoBlock()
	{
	}

	string BFSTaskP2PNoBlock::getName()
	{
		return "dgmark_BFS_P2P_no_block";
	}

	void BFSTaskP2PNoBlock::performBFS()
	{
		prepareBufers();
		performBFSActualStep();
		flushBuffers();
		waitForOthersToEnd();
		swapQueues();

		if (isRecvActive) {
			recvReq.Cancel();
		}
	}

	inline void BFSTaskP2PNoBlock::processGlobalChild(Vertex currVertex, Vertex child)
	{
		const Vertex childLocal = graph->vertexToLocal(child);
		const int childRank = graph->vertexRank(child);

		while (isRequestActive[childRank]) {
			probeBFSSynch();
		}

		sendDataBuffer[childRank][sendDataCount[childRank]] = childLocal;
		sendDataBuffer[childRank][sendDataCount[childRank] + 1] = currVertex;
		sendDataCount[childRank] += 2;

		if (sendDataCount[childRank] == itemsToSendCount) {
			sendData(childRank);
		}

		probeBFSSynch();
	}

	inline void BFSTaskP2PNoBlock::probeBFSSynch()
	{
		Status status;
		if (isRecvActive && recvReq.Test(status)) {
			isRecvActive = false;
			int elementsCount = status.Get_count(VERTEX_TYPE);

			for (size_t dataIndex = 0; dataIndex < elementsCount; dataIndex += 2) {
				Vertex currLocal = recvBuffer[dataIndex];
				Vertex parentGlobal = recvBuffer[dataIndex + 1];
				processLocalChild(parentGlobal, currLocal);
			}
		}

		if (!isRecvActive) {
			recvReq = comm->Irecv(&recvBuffer[0], itemsToSendCount, VERTEX_TYPE, ANY_SOURCE, BFS_DATA_TAG);
			isRecvActive = true;
		}

		refreshActivityState();
	}

	inline void BFSTaskP2PNoBlock::sendData(int toRank)
	{
		requests[toRank] = comm->Isend(&sendDataBuffer[toRank][0], sendDataCount[toRank], VERTEX_TYPE, toRank, BFS_DATA_TAG);
		sendDataCount[toRank] = 0;
		isRequestActive[toRank] = true;
	}

	inline void BFSTaskP2PNoBlock::prepareBufers()
	{
		for (int i = 0; i < size; ++i) {
			isRequestActive[i] = false;
			sendDataCount[i] = 0;
		}
		isRecvActive = false;
	}

	inline bool BFSTaskP2PNoBlock::flushBuffers()
	{
		bool isQueueEnlarged = false;
		//send remaining data buffers
		for (int reqIndex = 0; reqIndex < size; ++reqIndex) {
			if (!isRequestActive[reqIndex] && sendDataCount[reqIndex] > 0) {
				sendData(reqIndex);
			}
			while (isRequestActive[reqIndex]) {
				probeBFSSynch();
			}
		}
		return isQueueEnlarged;
	}

	inline bool BFSTaskP2PNoBlock::waitForOthersToEnd()
	{
		bool isQueueEnlarged = false;
		// tell all processes, that you had been stopped;
		requestSynch(true, BFS_END_SYNCH_TAG);
		int endedProcesses = 1;
		while (endedProcesses < size) {
			probeBFSSynch();
			while (probeSynch(BFS_END_SYNCH_TAG)) {
				++endedProcesses;
			}
		}

		probeBFSSynch();
		return isQueueEnlarged;
	}

	inline void BFSTaskP2PNoBlock::refreshActivityState()
	{
		//test all active request
		for (int reqIndex = 0; reqIndex < size; ++reqIndex) {
			if (isRequestActive[reqIndex]) {
				//if request completed, active state must be "false"
				isRequestActive[reqIndex] &= !requests[reqIndex].Test();
			}
		}
	}

	void BFSTaskP2PNoBlock::open(Graph* newGraph)
	{
		BFSTaskP2P::open(newGraph);
		requests = new Request[size];
		isRequestActive = new bool[size];
		sendDataBuffer = new Vertex*[size];
		sendDataCount = new size_t[size];
		recvBuffer = (Vertex*) Alloc_mem(itemsToSendCount * sizeof(Vertex), INFO_NULL);
		for (int i = 0; i < size; ++i) {
			sendDataBuffer[i] = (Vertex*) Alloc_mem(itemsToSendCount * sizeof(Vertex), INFO_NULL);
		}
	}

	void BFSTaskP2PNoBlock::close()
	{
		delete[] requests;
		delete[] isRequestActive;
		delete sendDataCount;
		Free_mem(recvBuffer);
		for (int i = 0; i < size; ++i) {
			Free_mem(sendDataBuffer[i]);
		}
		delete[] sendDataBuffer;
		BFSTaskP2P::close();
	}

}
