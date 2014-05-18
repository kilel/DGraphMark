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
#include "../util/Utils.h"

namespace dgmark {

	GraphDistributor::GraphDistributor(Intracomm *comm, Graph *graph) :
	BufferedDataDistributor(comm, ELEMENT_SIZE, BUFFERED_ELEMENTS),
	graph(graph),
	edges(graph->edges)
	{
	}

	GraphDistributor::~GraphDistributor()
	{
	}

	void GraphDistributor::distribute()
	{
		prepareBuffers();
		distributeEdges();
		flushBuffers();
		waitForOthersToEnd();

		if (isRecvRequestActive) {
			recvRequest.Cancel();
		}

		size_t numEdges = edges->size();
		edges->resize(numEdges, 0); //Shrink size.

		comm->Allreduce(IN_PLACE, &numEdges, 1, UNSIGNED_LONG_LONG, SUM);

		//For each vertex there are density edges, each presented by two oriented.
		assert(numEdges == graph->numGlobalVertex * graph->density * 2);
	}

	void GraphDistributor::distributeEdges()
	{
		const size_t initialEdgesCount = edges->size();
		for (size_t edgeIndex = 0; edgeIndex < initialEdgesCount; ++edgeIndex) {
			const Edge * const edge = edges->at(edgeIndex);
			const int rankTo = graph->vertexRank(edge->to);
			if (rankTo == rank) {
				edges->push_back(new Edge(edge->to, edge->from));
			} else {
				sendEdge(edge, rankTo);
			}
			probeSynchData();
		}
	}

	void GraphDistributor::sendEdge(const Edge * const edge, int toRank)
	{
		size_t &currCount = countToSend[toRank];
		Vertex *&currBuffer = sendBuffer[toRank];

		currBuffer[currCount] = edge->to;
		currBuffer[currCount + 1] = edge->from;
		currCount += elementSize;

		if (currCount == sendPackageSize) {
			sendData(toRank);
		}
	}

	void GraphDistributor::processRecvData(size_t countToRead)
	{
		for (size_t index = 0; index < countToRead; index += elementSize) {
			const Vertex from = recvBuffer[index];
			const Vertex to = recvBuffer[index + 1];
			edges->push_back(new Edge(from, to));
		}
	}
}

