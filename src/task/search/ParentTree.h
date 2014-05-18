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

#ifndef PARENTTREE_H
#define	PARENTTREE_H

#include "../../graph/CSRGraph.h"
#include "../Result.h"

namespace dgmark {

	class ParentTree : public Result {
	public:
		/**
		 * Creates parent tree (result for tree-makers).
		 * @param comm Communacator to work with.
		 * @param root global root vertex.
		 * @param parent Array of parents to verticies.
		 * @param graph initial graph os result.
		 * @param duration Duration of tree-making task in seconds (MPI::Wtime)
		 */
		ParentTree(Intracomm *comm, Vertex root, Vertex *parent, const CSRGraph *graph, double duration);
		ParentTree(const ParentTree& orig);
		virtual ~ParentTree();

		/**
		 * Traversed egdes per second is a mark of this kind of task.
		 * TEPS.
		 * @return mark for task result.
		 */virtual
		double getMark();
		double getTaskRunTime();
		TaskType getTaskType();

		Vertex getRoot();
		const Vertex* getParent();
		size_t getParentSize();
		const CSRGraph* getInitialGraph();
		double getTraversedEdges();
	private:
		Vertex root;
		Vertex * const parent;
		const CSRGraph * const graph;
		double taskRunTime;
		size_t traversedEdges;

		double calculateTraversedEdges(const Vertex * const parent, const CSRGraph * const graph);
	};
}

#endif	/* PARENTTREE_H */

