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

#include "BFSdgmark.h"

namespace dgmark {

	BFSdgmark::BFSdgmark(Intracomm *comm) : SearchTask(comm)
	{
	}

	BFSdgmark::BFSdgmark(const BFSdgmark& orig) : SearchTask(orig.comm)
	{
	}

	BFSdgmark::~BFSdgmark()
	{
	}

	ParentTree* BFSdgmark::run()
	{
		log << "Running BFS (" << getName() << ") from " << root << "\n";
		comm->Barrier();
		double startTime = Wtime();

		resetQueueParent();

		if (graph->vertexRank(root) == rank) {
			//root is my vertex, put it into queue.
			Vertex rootLocal = graph->vertexToLocal(root);
			parent[rootLocal] = root;
			queue[1] = rootLocal;
			queue[0] = 1;
		}

		//main loop
		stepCount = 0;
		while (isNextStepNeeded()) {
			performBFS();
			comm->Barrier();
			stepCount++;
		}

		log << "Finished in " << stepCount << " steps\n";

		comm->Barrier();
		double taskRunTime = Wtime() - startTime;

		Vertex *resultParent = new Vertex[numLocalVertex];
		for (int i = 0; i < numLocalVertex; ++i) {
			resultParent[i] = parent[i];
		}

		ParentTree *parentTree = new ParentTree(comm, root, resultParent, graph, taskRunTime);
		log << "BFS time: " << taskRunTime << " s\n";
		return parentTree;
	}

	inline void BFSdgmark::performBFSActualStep()
	{
		vector<Edge*> *edges = graph->edges;
		const size_t queueEnd = queue[0];

		for (size_t queueIndex = 1; queueIndex <= queueEnd; ++queueIndex) {
			const Vertex currVertex = queue[queueIndex];

			const size_t childStartIndex = graph->getStartIndex(currVertex);
			const size_t childEndIndex = graph->getEndIndex(currVertex);
			for (size_t childIndex = childStartIndex; childIndex < childEndIndex; ++childIndex) {
				const Vertex child = edges->at(childIndex)->to;
				const Vertex childLocal = graph->vertexToLocal(child);
				const int childRank = graph->vertexRank(child);
				//if(stepCount == 0)
				//printf("%d: %ld -> %ld\n", rank, currVertex, child);
				if (childRank == rank) {
					processLocalChild(graph->vertexToGlobal(currVertex), childLocal);
				} else {
					processGlobalChild(graph->vertexToGlobal(currVertex), child);
				}
			}
			++queue[0];
		}
	}

	inline void BFSdgmark::processLocalChild(Vertex parentVertexGlobal, Vertex childVertexLocal)
	{
		if (parent[childVertexLocal] == graph->numGlobalVertex) {
			parent[childVertexLocal] = parentVertexGlobal;
			nextQueue[++nextQueue[0]] = childVertexLocal;
		}
	}

	inline void BFSdgmark::swapQueues()
	{
		//clean current queue
		queue[0] = 0;

		//swap queues
		Vertex *temp = queue;
		queue = nextQueue;
		nextQueue = temp;
	}

	inline void BFSdgmark::resetQueueParent()
	{
		memset(queue, 0, getQueueSize() * sizeof(Vertex));
		queue[0] = 0;
		nextQueue[0] = 0;

		for (size_t i = 0; i < numLocalVertex; ++i) {
			parent[i] = graph->numGlobalVertex;
		}
	}

	inline Vertex BFSdgmark::getQueueSize()
	{
		return numLocalVertex;
	}

	bool BFSdgmark::isNextStepNeeded()
	{
		//finds OR for "isQueueEnlarged" in all processes.
		bool isQueueEnlarged = queue[0] > 0;
		comm->Allreduce(IN_PLACE, &isQueueEnlarged, 1, BOOL, LOR);
		return isQueueEnlarged;
	}

}
