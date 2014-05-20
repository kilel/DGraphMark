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

#include "DepthBuilder.h"

namespace dgmark {

	DepthBuilder::DepthBuilder(const Graph *graph) :
	graph(graph)
	{
		depth = new Vertex[graph->numLocalVertex];
	}

	DepthBuilder::~DepthBuilder()
	{
		delete[] depth;
	}

	Vertex* DepthBuilder::buildDepth(ParentTree *parentTree)
	{
		prepare(parentTree->getRoot());
		parent = parentTree->getParent();
		csrGraph = parentTree->getInitialGraph();

		while (isNextStepRequired) {
			buildNextStep();
		}

		return depth;
	}

	void DepthBuilder::prepare(Vertex root)
	{
		isNextStepRequired = true;

		for (Vertex localVertex = 0; localVertex < graph->numLocalVertex; ++localVertex) {
			depth[localVertex] = graph->numGlobalVertex;
		}
	}
}

