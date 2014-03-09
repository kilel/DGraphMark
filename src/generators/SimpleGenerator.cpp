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

#include "SimpleGenerator.h"
#include <cstdlib>
#include <assert.h>

namespace dgmark {

    SimpleGenerator::SimpleGenerator(Intracomm *comm, int grade, int density) :
    GraphGenerator(comm), grade(grade), density(density), log(comm) {
    }

    SimpleGenerator::~SimpleGenerator() {
    }

    Graph* SimpleGenerator::generate() {
        log << "Generating graph... ";
        comm->Barrier();
        double startTime = Wtime();


        Graph* graph = new Graph(comm, grade, density);
        vector<Edge*> *edges = graph->edges;
        Vertex numLocalVertex = graph->numLocalVertex;

        int numEdgesPerVertex = density / 2;
        int additionalEdgeFlag = density % 2;

        //Here we create density/2 oriented edges from each of local
        for (Vertex vertex = 0; vertex < numLocalVertex; ++vertex) {
            Vertex globalVertexFrom = graph->vertexToGlobal(vertex);
            int numEdges = numEdgesPerVertex + (vertex & 1) * additionalEdgeFlag;

            for (int i = 0; i < numEdges; ++i) {
                uint64_t rankTo = rand() % size;
                uint64_t localVertexTo = rand() % numLocalVertex;

                Vertex globalVertexTo = graph->vertexToGlobal(rankTo, localVertexTo);
                if (globalVertexFrom != globalVertexTo) {
                    edges->push_back(new Edge(globalVertexFrom, globalVertexTo));
                } else {
                    --i;
                    continue;
                }
            }
        }
        comm->Barrier();
        generationTime = Wtime();
        log << generationTime - startTime << " s\n";

        log << "Distributing graph... ";
        //Here we make graph unoriended, by transfering edges between nodes.
        graph->distribute();

        distributionTime = Wtime() - generationTime;
        generationTime = generationTime - startTime;
        log << distributionTime << " s\n";
        return graph;
    }

    double SimpleGenerator::getGenerationTime() {
        return generationTime;
    }

    double SimpleGenerator::getDistributionTime() {
        return distributionTime;
    }

    int SimpleGenerator::getGrade() {
        return grade;
    }

    int SimpleGenerator::getDensity() {
        return density;
    }

}
