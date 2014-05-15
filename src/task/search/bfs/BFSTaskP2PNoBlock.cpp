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

	BFSTaskP2PNoBlock::BFSTaskP2PNoBlock(Intracomm *comm) :
	BFSTaskP2P(comm),
	BufferedDataDistributor(comm, 2, 256)
	{
	}

//	BFSTaskP2PNoBlock::BFSTaskP2PNoBlock(const BFSTaskP2PNoBlock& orig) :
//	BFSTaskP2P(orig.comm),
//	BufferedDataDistributor(orig.comm, 2, 256)
//	{
//	}

	BFSTaskP2PNoBlock::~BFSTaskP2PNoBlock()
	{
	}

	string BFSTaskP2PNoBlock::getName()
	{
		return "dgmark_BFS_P2P_no_block";
	}

	void BFSTaskP2PNoBlock::performBFS()
	{
		prepareBuffers();
		performBFSActualStep();
		flushBuffers();
		waitForOthersToEnd();
		swapQueues();

		if (isRecvRequestActive) {
			recvRequest.Cancel();
		}
	}

	inline void BFSTaskP2PNoBlock::processGlobalChild(Vertex currVertex, Vertex child)
	{
		const Vertex childLocal = graph->vertexToLocal(child);
		const int childRank = graph->vertexRank(child);

		while (isSendRequestActive[childRank]) {
			probeReadData();
		}

		size_t &currCount = countToSend[childRank];
		Vertex *&currBuffer = sendBuffer[childRank];

		currBuffer[currCount] = childLocal;
		currBuffer[currCount + 1] = currVertex;
		currCount += 2;

		if (currCount == sendPackageSize) {
			sendData(childRank);
		}

		probeReadData();
	}

	void BFSTaskP2PNoBlock::processRecvData(size_t countToRead)
	{
		for (size_t dataIndex = 0; dataIndex < countToRead; dataIndex += 2) {
			Vertex currLocal = recvBuffer[dataIndex];
			Vertex parentGlobal = recvBuffer[dataIndex + 1];
			processLocalChild(parentGlobal, currLocal);
		}
	}

}
