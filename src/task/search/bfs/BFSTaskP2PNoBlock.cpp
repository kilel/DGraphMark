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
	requests = new Request[size];
	isRequestActive = new bool[size];
	sendDataBuffer = new Vertex*[size];
	sendDataCount = new size_t[size];
	recvBuffer = (Vertex*) Alloc_mem(itemsToSendCount * sizeof(Vertex), INFO_NULL);
	for (int i = 0; i < size; ++i) {
	    sendDataBuffer[i] = (Vertex*) Alloc_mem(itemsToSendCount * sizeof(Vertex), INFO_NULL);
	}
    }

    BFSTaskP2PNoBlock::BFSTaskP2PNoBlock(const BFSTaskP2PNoBlock& orig) : BFSTaskP2P(orig.comm)
    {
    }

    BFSTaskP2PNoBlock::~BFSTaskP2PNoBlock()
    {
	delete[] requests;
	delete[] isRequestActive;
	delete sendDataCount;
	Free_mem(recvBuffer);
	for (int i = 0; i < size; ++i) {
	    Free_mem(sendDataBuffer[i]);
	}
	delete[] sendDataBuffer;
    }

    string BFSTaskP2PNoBlock::getName()
    {
	return "dgmark_BFS_P2P_no_block";
    }

    bool BFSTaskP2PNoBlock::performBFS()
    {
	for (int i = 0; i < size; ++i) {
	    isRequestActive[i] = false;
	    sendDataCount[i] = 0;
	}
	isRecvActive = false;

	bool isQueueEnlarged = performBFSActualStep();

	//send remaining data buffers
	for (int reqIndex = 0; reqIndex < size; ++reqIndex) {
	    if (!isRequestActive[reqIndex] && sendDataCount[reqIndex] > 0) {
		sendData(reqIndex);
	    }
	    while (isRequestActive[reqIndex]) {
		isQueueEnlarged |= probeBFSSynch();
	    }
	}

	requestSynch(true, BFS_END_SYNCH_TAG); // tell all processes, that you had been stopped;

	int endedProcesses = 1;
	while (endedProcesses < size) {
	    isQueueEnlarged |= probeBFSSynch();
	    while (probeSynch(BFS_END_SYNCH_TAG)) {
		++endedProcesses;
	    }
	}

	isQueueEnlarged |= probeBFSSynch();
	
	if(isRecvActive) {
	    recvReq.Cancel();
	}

	comm->Allreduce(IN_PLACE, &isQueueEnlarged, 1, BOOL, LOR); //finds OR for "isQueueEnlarged" in all processes.
	swapQueues();
	return isQueueEnlarged;
    }

    inline bool BFSTaskP2PNoBlock::processLocalChild(Vertex currVertex, Vertex child)
    {
	bool isQueueChanged = probeBFSSynch();
	return BFSdgmark::processLocalChild(currVertex, child) || isQueueChanged;
    }

    inline bool BFSTaskP2PNoBlock::processGlobalChild(Vertex currVertex, Vertex child)
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

	return probeBFSSynch();
    }

    inline void BFSTaskP2PNoBlock::sendData(int toRank)
    {
	requests[toRank] = comm->Isend(&sendDataBuffer[toRank][0], sendDataCount[toRank], VERTEX_TYPE, toRank, BFS_DATA_TAG);
	sendDataCount[toRank] = 0;
	isRequestActive[toRank] = true;
    }

    inline bool BFSTaskP2PNoBlock::probeBFSSynch()
    {
	Status status;
	bool isQueueChanged = false;

	if (isRecvActive && recvReq.Test(status)) {
	    isRecvActive = false;
	    int elementsCount = status.Get_count(VERTEX_TYPE);
	    if (elementsCount >= 2) {
		//Vertex parentRank = status.Get_source();
		//comm->Recv(&recvBuffer[0], elementsCount, VERTEX_TYPE, parentRank, BFS_DATA_TAG);

		for (size_t dataIndex = 0; dataIndex < elementsCount; dataIndex += 2) {
		    Vertex currLocal = recvBuffer[dataIndex];
		    Vertex parentGlobal = recvBuffer[dataIndex + 1];

		    if (parent[currLocal] == graph->numGlobalVertex) {
			parent[currLocal] = parentGlobal;
			nextQueue[nextQueue[1]++] = currLocal;
			//assert(nextQueue[1] < getQueueSize());
			isQueueChanged = true;
		    }
		}
	    }
	}
	
	if(!isRecvActive) {
	    recvReq = comm->Irecv(&recvBuffer[0], itemsToSendCount, VERTEX_TYPE, ANY_SOURCE, BFS_DATA_TAG);
	    isRecvActive = true;
	}

	//test all active request
	for (int reqIndex = 0; reqIndex < size; ++reqIndex) {
	    if (isRequestActive[reqIndex]) {
		//if request completed, active state must be "false"
		isRequestActive[reqIndex] &= !requests[reqIndex].Test();
	    }
	}

	return isQueueChanged;
    }

}
