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
	BufferedDataDistributor(comm,
				ELEMENT_SIZE,
				BUFFERED_ELEMENTS)
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
		prepareBuffers();
		performBFSActualStep();
		flushBuffers();
		waitForOthersToEnd();
	}

	inline void BFSTaskP2PNoBlock::processGlobalChild(Vertex currVertex, Vertex child)
	{
		const Vertex childLocal = graph->vertexToLocal(child);
		const int childRank = graph->vertexRank(child);

		while (isSendRequestActive[childRank]) {
			probeSynchData();
		}

		size_t &currCount = countToSend[childRank];
		Vertex *&currBuffer = sendBuffer[childRank];

		currBuffer[currCount] = childLocal;
		currBuffer[currCount + 1] = currVertex;
		currCount += 2;

		if (currCount == sendPackageSize) {
			sendData(childRank);
		}

		probeSynchData();
	}

	void BFSTaskP2PNoBlock::processRecvData(size_t countToRead)
	{
		for (size_t dataIndex = 0; dataIndex < countToRead; dataIndex += 2) {
			const Vertex currLocal = recvBuffer[dataIndex];
			const Vertex parentGlobal = recvBuffer[dataIndex + 1];
			processLocalChild(parentGlobal, currLocal);
		}
	}

}
