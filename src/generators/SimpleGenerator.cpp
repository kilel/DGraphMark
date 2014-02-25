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

        Vertex numLocalVertex = Utils::getNumLocalVertex();
        Graph* graph = new Graph(numLocalVertex);
        vector<Edge*> *edges = graph->edges;

        int numEdgesPerVertex = density / 2;
        int additionalEdgeFlag = density % 2;

        for (Vertex vertex = 0; vertex < numLocalVertex; ++vertex) {
            Vertex globalVertexFrom = Utils::vertexToGlobal(vertex);
            int numEdges = numEdgesPerVertex + (vertex & 1) * additionalEdgeFlag;

            for (int i = 0; i < numEdges; ++i) {
                uint64_t rankTo = rand() % size;
                uint64_t localVertexTo = rand() % numLocalVertex;

                Vertex globalVertexTo = Utils::vertexToGlobal(rankTo, localVertexTo);
                if (globalVertexFrom != globalVertexTo) {
                    edges->push_back(new Edge(globalVertexFrom, globalVertexTo));
                } else {
                    --i;
                    continue;
                }
            }
        }
        comm->Barrier();
        generationTime = Wtime() - startTime;
        log << generationTime << " s\n";
        return graph;
    }

    double SimpleGenerator::getGenerationTime() {
        return generationTime;
    }

    int SimpleGenerator::getGrade() {
        return grade;
    }

    int SimpleGenerator::getDensity() {
        return density;
    }

}
