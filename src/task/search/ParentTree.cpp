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

#include "ParentTree.h"
#include "../../util/Log.h"

namespace dgmark {

	ParentTree::ParentTree(Intracomm *comm,
			Vertex root,
			Vertex *parent,
			const CSRGraph *graph,
			double taskRunTime) :
	Result(comm),
	root(root),
	parent(parent),
	graph(graph),
	taskRunTime(taskRunTime),
	traversedEdges(calculateTraversedEdges(parent, graph))
	{
	}

	ParentTree::ParentTree(const ParentTree& orig) :
	Result(orig.comm),
	root(orig.root),
	parent(orig.parent),
	graph(orig.graph),
	taskRunTime(orig.taskRunTime),
	traversedEdges(orig.traversedEdges)
	{
	}

	ParentTree::~ParentTree()
	{
		Free_mem(parent);
	}

	Vertex ParentTree::getRoot()
	{
		return root;
	}

	const Vertex* ParentTree::getParent()
	{
		return parent;
	}

	size_t ParentTree::getParentSize()
	{
		return graph->numLocalVertex;
	}

	const CSRGraph* ParentTree::getInitialGraph()
	{
		return graph;
	}

	double ParentTree::getMark()
	{
		return traversedEdges / taskRunTime;
	}

	double ParentTree::getTaskRunTime()
	{
		return taskRunTime;
	}

	TaskType ParentTree::getTaskType()
	{
		return SEARCH;
	}

	double ParentTree::getTraversedEdges()
	{
		return traversedEdges;
	}

	double ParentTree::calculateTraversedEdges(const Vertex * const parent,
						const CSRGraph * const graph)
	{
		double realTraversedEdges = 0;
		//Log log(comm);
		//log << "calc traversed :\n";
		for (Vertex localVertex = 0; localVertex < graph->numLocalVertex; ++localVertex) {
			const size_t traversedMax = graph->getEndIndex(localVertex) - graph->getStartIndex(localVertex);
			//log << "add " << traversedMax << " visited == " << parent[localVertex] << "\n";
			realTraversedEdges += parent[localVertex] != graph->numGlobalVertex ? traversedMax : 0;
		}

		comm->Allreduce(IN_PLACE, &realTraversedEdges, 1, DOUBLE, SUM);
		return realTraversedEdges;
	}
}

