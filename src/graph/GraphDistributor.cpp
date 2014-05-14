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

#include "GraphDistributor.h"
#include "assert.h"

namespace dgmark {

	GraphDistributor::GraphDistributor(Intracomm *comm) : Communicable(comm)
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

	GraphDistributor::~GraphDistributor()
	{
	}

	void GraphDistributor::distribute(Graph *graph)
	{
		prepareBuffers();
		vector<Edge*> *edges = graph->edges;
		size_t initialEdgesCount = edges->size();

		for (size_t edgeIndex = 0; edgeIndex < initialEdgesCount; ++edgeIndex) {
			Edge *edge = edges->at(edgeIndex);
			const int rankTo = graph->vertexRank(edge->to);
			if (rankTo == rank) {
				edges->push_back(new Edge(edge->to, edge->from));
			} else {
				sendEdge(edge, rankTo, edges);
			}
			probeReadEdge(edges);
		}

		flushBuffers(edges);
		waitForOthersToEnd(edges);

		if (isRecvRequestActive) {
			recvRequest.Cancel();
		}

		edges->resize(edges->size(), 0); //Shrink size.
	}

	void GraphDistributor::sendEdge(Edge* edge, int toRank, vector<Edge*> *edges)
	{
		size_t &currCount = countToSend[toRank];
		Vertex *&currBuffer = sendBuffer[toRank];
		currBuffer[currCount] = edge->to;
		currBuffer[currCount + 1] = edge->from;
		currCount += 2;

		if (currCount == sendPackageSize) {
			sendEdges(toRank, edges);
		}
	}

	void GraphDistributor::sendEdges(int toRank, vector<Edge*> *edges)
	{
		while (isSendRequestActive[toRank]) {
			probeReadEdge(edges);
		}

		sendRequest[toRank] = comm->Isend(&sendBuffer[toRank][0],
			countToSend[toRank], VERTEX_TYPE, toRank, DISTRIBUTION_TAG);
		countToSend[toRank] = 0;
		isSendRequestActive[toRank] = true;
	}

	bool GraphDistributor::probeReadEdge(vector<Edge*> *edges)
	{
		Status status;

		if (isRecvRequestActive && recvRequest.Test(status)) {
			isRecvRequestActive = false;
			int dataCount = status.Get_count(VERTEX_TYPE);
			for (int index = 0; index < dataCount; index += 2) {
				const Vertex from = recvBuffer[index];
				const Vertex to = recvBuffer[index + 1];
				edges->push_back(new Edge(from, to));
			}
		}

		if (!isRecvRequestActive) {
			recvRequest = comm->Irecv(recvBuffer, sendPackageSize,
				VERTEX_TYPE, ANY_SOURCE, DISTRIBUTION_TAG);
			isRecvRequestActive = true;
		}

		updateRequestsActivity();
	}

	void GraphDistributor::prepareBuffers()
	{
		for (int reqIndex = 0; reqIndex < size; ++reqIndex) {
			countToSend[reqIndex] = 0;
			isSendRequestActive[reqIndex] = false;
		}
		isRecvRequestActive = false;
	}

	void GraphDistributor::flushBuffers(vector<Edge*> *edges)
	{
		for (int reqIndex = 0; reqIndex < size; ++reqIndex) {
			if (!isSendRequestActive[reqIndex] && countToSend[reqIndex] > 0) {
				sendEdges(reqIndex, edges);
			}

			while (isSendRequestActive[reqIndex]) {
				probeReadEdge(edges);
			}
		}
	}

	void GraphDistributor::updateRequestsActivity()
	{
		for (int reqIndex = 0; reqIndex < size; ++reqIndex) {
			if (isSendRequestActive[reqIndex]) {
				isSendRequestActive[reqIndex] &= !sendRequest[reqIndex].Test();
			}
		}
	}

	void GraphDistributor::waitForOthersToEnd(vector<Edge*> *edges)
	{
		// tell all processes, that you had been stopped;
		requestSynch(true, END_TAG);
		int endedProcesses = 1;
		while (endedProcesses < size) {
			probeReadEdge(edges);
			while (probeSynch(END_TAG)) {
				++endedProcesses;
			}
		}

		probeReadEdge(edges);
	}
}

