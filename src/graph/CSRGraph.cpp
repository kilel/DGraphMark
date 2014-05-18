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

#include <algorithm>
#include "CSRGraph.h"
#include "../util/Log.h"

namespace dgmark {

	CSRGraph::CSRGraph(Graph *graph) :
	Graph(graph)
	{
		startIndex = new Vertex[numLocalVertex + 1]; // the last one is numLocalVertex.
		buildCSR();
	}

	CSRGraph::CSRGraph(const CSRGraph& orig) :
	Graph(orig)
	{
		startIndex = new Vertex[numLocalVertex + 1];
		Vertex *origSI = orig.startIndex;
		copy(origSI, origSI + numLocalVertex + 1, startIndex);
	}

	CSRGraph::~CSRGraph()
	{
		delete startIndex;
	}

	bool compareEdge(Edge *first, Edge *second)
	{
		if (first->from < second->from) {
			return true;
		} else if (first->from > second->from) {
			return false;
		} else if (first->to < second->to) {
			return true;
		} else {
			return false;
		}
	}

	void CSRGraph::buildCSR()
	{
		//std::sort(edges->begin(), edges->end(), compareEdge);
		//Array of edges is half sorted. 
		//This kind of sort gives better performance.
		std::stable_sort(edges->begin(), edges->end(), compareEdge);

		Vertex previousVertex = -1;
		for (size_t currEdgeIndex = 0; currEdgeIndex < edges->size(); ++currEdgeIndex) {
			const Edge * const currEdge = edges->at(currEdgeIndex);
			const Vertex localVertex = vertexToLocal(currEdge->from);

			if (previousVertex == localVertex) {
				continue;
			}

			//filling skipped edges with index of current.
			//This makes they endIndex - startIndex == 0
			for (Vertex skipped = previousVertex + 1; skipped < localVertex; ++skipped) {
				startIndex[skipped] = currEdgeIndex;
			}

			startIndex[localVertex] = currEdgeIndex;
			previousVertex = localVertex;
		}
		startIndex[numLocalVertex] = edges->size();
	}

	size_t CSRGraph::getStartIndex(Vertex localVertex) const
	{
		return startIndex[localVertex];
	}

	size_t CSRGraph::getEndIndex(Vertex localVertex) const
	{
		return startIndex[localVertex + 1];
	}
}
