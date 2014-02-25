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

#include "SortedGraph.h"
#include "Utils.h"

namespace dgmark {

    SortedGraph::SortedGraph(Graph *graph) : Graph(graph) {
        startIndex = new vector<size_t>(numLocalVertex + 1); // the last one is numLocalVertex.
        sort();
    }

    SortedGraph::SortedGraph(const SortedGraph& orig) :
    Graph(orig.edges, orig.numLocalVertex), startIndex(orig.startIndex) {
    }

    SortedGraph::~SortedGraph() {
        startIndex->clear();
        delete startIndex;
    }

    bool compareEdge(Edge *first, Edge *second) {
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

    void SortedGraph::sort() {
        std::sort(edges->begin(), edges->end(), compareEdge);

        Vertex prev = 0;
        for (size_t index = 0; index < edges->size();) {
            Vertex localVertex = Utils::vertexToLocal(edges->at(index)->from);
            //printf("%d: v = %ld, size = %ld\n", Utils::getRank(), localVertex, startIndex->size());

            for (Vertex skipped = prev + 1; skipped < localVertex; ++skipped) {
                startIndex->at(skipped) = startIndex->at(prev);
            }

            startIndex->at(localVertex) = index;
            while (index < edges->size() && Utils::vertexToLocal(edges->at(index)->from) == localVertex) {
                ++index;
            }
            prev = localVertex;
        }
        startIndex->at(numLocalVertex) = numLocalVertex;
    }

    size_t SortedGraph::getStartIndex(Vertex localVertex) {
        return startIndex->at(localVertex);
    }

    size_t SortedGraph::getEndIndex(Vertex localVertex) {
        startIndex->at(localVertex + 1);
    }
}
