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

#include "RandomGenerator.h"

namespace dgmark {

	RandomGenerator::RandomGenerator(Intracomm *comm) :
	GraphGenerator(comm), log(comm),
	random(Random::getInstance(comm))
	{
	}

	RandomGenerator::~RandomGenerator()
	{
	}

	Graph* RandomGenerator::generate(int grade, int density)
	{
		Graph* graph = new Graph(comm, grade, density);
		doGenerate(graph);

		#ifdef GRAPH_IS_UNORIENTED
		doDistribute(graph);
		#endif

		return graph;
	}

	void RandomGenerator::doGenerate(Graph* graph)
	{
		log << "Generating graph... ";
		comm->Barrier();
		double startTime = Wtime();

		generateInternal(graph);
		graph->edges->resize(graph->edges->size(), 0);

		comm->Barrier();
		generationTime = Wtime() - startTime;
		log << generationTime << " s\n";
	}

	void RandomGenerator::doDistribute(Graph* graph)
	{
		log << "Distributing graph... ";
		comm->Barrier();
		double startTime = Wtime();

		//Here we make graph unoriended, by transfering edges between nodes.
		GraphDistributor *distributor = new GraphDistributor(comm, graph);
		distributor->distribute();
		delete distributor;

		comm->Barrier();
		distributionTime = Wtime() - startTime;
		log << distributionTime << " s\n";
	}

	double RandomGenerator::getGenerationTime()
	{
		return generationTime;
	}

	double RandomGenerator::getDistributionTime()
	{
		return distributionTime;
	}
}
