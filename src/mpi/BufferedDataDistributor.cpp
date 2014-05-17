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

#include "BufferedDataDistributor.h"

namespace dgmark {

	BufferedDataDistributor::BufferedDataDistributor(Intracomm *comm,
		size_t elementSize, size_t bufferedElementsCount) :
	Communicable(comm),
	elementSize(elementSize),
	sendPackageSize(elementSize * bufferedElementsCount)
	{
		sendBuffer = new Vertex*[size];
		for (int reqIndex = 0; reqIndex < size; ++reqIndex) {
			sendBuffer[reqIndex] = (Vertex*) Alloc_mem(sendPackageSize * sizeof(Vertex), INFO_NULL);
		}
		recvBuffer = (Vertex*) Alloc_mem(sendPackageSize * sizeof(Vertex), INFO_NULL);
		countToSend = new Vertex[size];
		sendRequest = new Request[size];
		isSendRequestActive = new bool[size];
		isRecvRequestActive = false;
	}

	BufferedDataDistributor::~BufferedDataDistributor()
	{
	}

	void BufferedDataDistributor::sendData(int toRank)
	{
		while (isSendRequestActive[toRank]) {
			probeReadData();
		}

		//printf("%d: sending %ld data to %d\n", rank, countToSend[toRank], toRank);

		sendRequest[toRank] = comm->Isend(
			&sendBuffer[toRank][0],
			countToSend[toRank],
			VERTEX_TYPE,
			toRank,
			DISTRIBUTION_TAG);

		countToSend[toRank] = 0;
		isSendRequestActive[toRank] = true;
	}

	void BufferedDataDistributor::probeReadData()
	{
		Status status;

		if (isRecvRequestActive && recvRequest.Test(status)) {
			isRecvRequestActive = false;
			size_t dataCount = status.Get_count(VERTEX_TYPE);
			//printf("%d: reading %ld data from %d\n", rank, dataCount, status.Get_source());
			if (dataCount > 0) {
				processRecvData(dataCount);
			} else {
				++countEnded;
			}
		}

		if (!isRecvRequestActive) {
			isRecvRequestActive = true;
			recvRequest = comm->Irecv(
				recvBuffer,
				sendPackageSize,
				VERTEX_TYPE,
				ANY_SOURCE,
				DISTRIBUTION_TAG);
		}

		updateRequestsActivity();
	}

	void BufferedDataDistributor::prepareBuffers()
	{
		for (int reqIndex = 0; reqIndex < size; ++reqIndex) {
			countToSend[reqIndex] = 0;
			isSendRequestActive[reqIndex] = false;
		}
		isRecvRequestActive = false;
		countEnded = 0;
	}

	void BufferedDataDistributor::flushBuffers()
	{
		//printf("%d: flush\n", rank);
		for (int reqIndex = 0; reqIndex < size; ++reqIndex) {
			if (reqIndex == rank) {
				continue;
			}

			//send all pending items
			if (!isSendRequestActive[reqIndex] && countToSend[reqIndex] > 0) {
				sendData(reqIndex);
			}
			
			//printf("%d: send empty\n", rank);
			//send empty message to guarantee end of communication
			sendData(reqIndex);

			while (isSendRequestActive[reqIndex]) {
				probeReadData();
			}

		}
	}

	void BufferedDataDistributor::updateRequestsActivity()
	{
		for (int reqIndex = 0; reqIndex < size; ++reqIndex) {
			if (isSendRequestActive[reqIndex]) {
				isSendRequestActive[reqIndex] &= !sendRequest[reqIndex].Test();
			}
		}
	}

	void BufferedDataDistributor::waitForOthersToEnd()
	{
		//printf("%d: wait end\n", rank);
		// tell all processes, that you had been stopped;
		//requestSynch(true, END_TAG);
		++countEnded;
		//int endedProcesses = 1;
		while (countEnded < size) {
			probeReadData();
			//			while (probeSynch(END_TAG)) {
			//				++endedProcesses;
			//			}
		}
		probeReadData();
	}
}
