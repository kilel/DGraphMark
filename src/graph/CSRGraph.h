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

#ifndef CSRGRAPH_H
#define	CSRGRAPH_H

#include "Graph.h"

namespace dgmark {

    /*
     * Compressed Sparse Row Graph.
     * Contains vector of edges, sorted by v.form, and then by v.to.
     * Provides start and end indices of an output edges interval for any local vertex.
     */
    class CSRGraph : public Graph {
    public:
        CSRGraph(Graph *graph);
        CSRGraph(const CSRGraph& orig);
        virtual ~CSRGraph();

        size_t getStartIndex(Vertex v);
        size_t getEndIndex(Vertex v);

    private:
        void sort();
        Vertex *startIndex;

    };
}

#endif	/* CSRGRAPH_H */

