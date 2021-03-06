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

#include "UniformGenerator.h"
#include "../util/Utils.h"

namespace dgmark {

	UniformGenerator::UniformGenerator(Intracomm *comm) :
	RandomGenerator(comm)
	{
	}

	UniformGenerator::~UniformGenerator()
	{
	}

	void UniformGenerator::generateInternal(Graph *graph)
	{
		//Here we create density/2 oriented edges from each of local
		for (Vertex vertex = 0; vertex < graph->numLocalVertex; ++vertex) {
			addEdgeFromVertex(graph, vertex, graph->density);
		}
	}

	void UniformGenerator::addEdgeFromVertex(Graph *graph, Vertex localVertex, size_t numEdges)
	{
		vector<Edge *> * const edges = graph->edges;
		const Vertex globalVertexFrom = graph->vertexToGlobal(localVertex);
		for (int newEdgeIndex = 0; newEdgeIndex < numEdges; ++newEdgeIndex) {
			const uint64_t rankTo = random->next(0, size);
			const Vertex localVertexTo = random->next(0, graph->numGlobalVertex);
			const Vertex globalVertexTo = graph->vertexToGlobal(rankTo, localVertexTo);

			//prevent self-loops
			if (globalVertexFrom != globalVertexTo) {
				edges->push_back(new Edge(globalVertexFrom, globalVertexTo));
			} else {
				--newEdgeIndex;
				continue;
			}
		}
	}
}
