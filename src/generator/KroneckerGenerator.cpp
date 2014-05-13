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

    KroneckerGenerator::KroneckerGenerator(Intracomm *comm) : SimpleGenerator(comm) {
    }
    
    KroneckerGenerator::~KroneckerGenerator() {
    }
    
    void KroneckerGenerator::addEdgeFromVertex(Graph *graph, Vertex localVertex, size_t numEdges) {
        vector<Edge*> *edges = graph->edges;
        Vertex globalVertexFrom = graph->vertexToGlobal(localVertex);
        for (int i = 0; i < numEdges; ++i) {
            uint64_t rankTo = random->next(0, size);
            uint64_t localVertexTo = random->next(0, graph->numGlobalVertex);

            Vertex globalVertexTo = graph->vertexToGlobal(rankTo, localVertexTo);
            if (globalVertexFrom != globalVertexTo) {
                edges->push_back(new Edge(globalVertexFrom, globalVertexTo));
            } else {
                --i;
                continue;
            }
        }
    }
}
