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

#include "KroneckerGenerator.h" 

namespace dgmark {

	KroneckerGenerator::KroneckerGenerator(Intracomm *comm) :
	RandomGenerator(comm)
	{
	}

	KroneckerGenerator::~KroneckerGenerator()
	{
	}

	void KroneckerGenerator::generateInternal(Graph* graph)
	{
		GraphDistributor *distributor = new GraphDistributor(comm, graph);
		distributor->open();

		for (Vertex LocalVertex = 0; LocalVertex < graph->numLocalVertex; ++LocalVertex) {
			for (int i = 0; i < graph->density; ++i) {
				Edge * edge = generateEdge(graph);
				while (edge->from == edge->to) {
					delete edge;
					edge = generateEdge(graph);
				}
				//log << edge->from << " | " << edge->to << "\n";
				distributor->sendEdge(edge->from, edge->to);
				delete edge;
				
				//log << "size: " << graph->edges->size() << "\n";
			}
		}

		distributor->close();
		delete distributor;
	}

	Edge* KroneckerGenerator::generateEdge(Graph *graph)
	{
		Vertex fromL = 0;
		Vertex fromR = graph->numGlobalVertex - 1;

		Vertex toL = 0;
		Vertex toR = graph->numGlobalVertex - 1;

		int toDest, fromDest;

		while (fromR - fromL != 1 && toR - toL != 1) {
			//log << fromL << " | " << toL << " generating\n";
			getKroneckerDest(fromDest, toDest);
			moveBinary(fromL, fromR, fromDest);
			moveBinary(toL, toR, toDest);
		}

		getKroneckerDest(fromDest, toDest);
		fromL += fromDest;
		toL += toDest;

		return new Edge(revert(fromL, graph->grade),
				revert(toL, graph->grade));
	}

	void KroneckerGenerator::moveBinary(Vertex &left, Vertex &right, int dest)
	{
		if (dest == 1) {
			left += (right - left) / 2;
		} else {
			right -= (right - left) / 2;
		}
	}

	Vertex KroneckerGenerator::revert(Vertex source, int grade)
	{
		Vertex result = 0;
		for (int i = 0; i < grade; ++i) {
			//log << result << " | " << source << " reverting\n";
			result = result << 1 | source & 1;
			source = source >> 1;
		}
		return result;
	}

	void KroneckerGenerator::getKroneckerDest(int &fromDest, int &toDest)
	{

		double prob = random->nextDouble();

		if (prob < KPStrongToStrong) {
			fromDest = 0;
			toDest = 0;
		} else if (prob < KPStrongToWeak) {
			fromDest = 0;
			toDest = 1;
		} else if (prob < KPWeakToStrong) {
			fromDest = 1;
			toDest = 0;
		} else {
			fromDest = 1;
			toDest = 1;
		}
	}


}
