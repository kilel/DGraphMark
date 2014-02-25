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

#ifndef GRAPH_H
#define	GRAPH_H

#include <vector>
#include <algorithm>
#include "Edge.h"

namespace dgmark {
    using namespace std;

    /**
     * Describes an oriented graph.
     */
    class Graph {
    public:
        vector<Edge*> *edges;
        size_t numLocalVertex;

        /**
         * Creates graph with no edges.
         */
        Graph(size_t numLocalVertex) : numLocalVertex(numLocalVertex) {
            edges = new vector<Edge*>();
        }

        /**
         * Creates graph from edges list. 
         * @param edgesList list of edges to initialize with.
         */
        Graph(vector<Edge*> *edgesList, size_t numLocalVertex) :
        edges(edgesList), numLocalVertex(numLocalVertex) {
        }

        /**
         * Copying graph. 
         * Note, that graph copies reference to original edgelist.
         * @param orig Original graph.
         */
        Graph(const Graph& orig) : edges(orig.edges), numLocalVertex(orig.numLocalVertex) {
        }
        
        /**
         * Copying graph. 
         * Note, that graph copies reference to original edgelist.
         * @param orig Original graph.
         */
        Graph(const Graph *orig) : edges(orig->edges), numLocalVertex(orig->numLocalVertex) {
        }

        virtual ~Graph() {
        }

        /**
         * Clears edgelist.
         */
        virtual void clear() {
            edges->clear();
            delete edges;
        }

    };
}

#endif	/* GRAPH_H */
