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

#include "Graph.h"
#include "GraphDistributor.h"
#include "assert.h"

namespace dgmark {

	Graph::Graph(Intracomm *comm, int grade, int density) :
	Communicable(comm),
	edges(new vector<Edge*>()),
	grade(grade),
	density(density),
	numGlobalVertex(1 << grade),
	numLocalVertex(1 << grade - getLog2(comm->Get_size()))
	{
		initialize();
	}

	Graph::Graph(const Graph& orig) :
	Communicable(orig.comm),
	edges(orig.edges),
	grade(orig.grade),
	diffGrade(orig.diffGrade),
	density(orig.density),
	numGlobalVertex(orig.numGlobalVertex),
	numLocalVertex(orig.numLocalVertex)
	{
	}

	Graph::Graph(const Graph *orig) :
	Communicable(orig->comm),
	edges(orig->edges),
	grade(orig->grade),
	diffGrade(orig->diffGrade),
	density(orig->density),
	numGlobalVertex(orig->numGlobalVertex),
	numLocalVertex(orig->numLocalVertex)
	{
	}

	Graph::~Graph()
	{
	}

	void Graph::clear()
	{
		edges->clear();
		delete edges;
	}

	void Graph::initialize()
	{
		if (((size - 1) & size) != 0) {
			if (rank == 0) {
				printf("Number of MPI nodes must be 2^n. %d is not.\n", size);
			}
			assert(false);
		}

		int commGrade = getLog2(size);

		assert(commGrade < grade);
		assert(commGrade >= 0);
		assert(grade > 0);
		assert(density > 0);

		diffGrade = grade - commGrade;
	}

}
