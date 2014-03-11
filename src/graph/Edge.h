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

#ifndef EDGE_H
#define	EDGE_H

#include <inttypes.h>
#include "../mpi/Communicable.h"

namespace dgmark {

    /**
     * Represents index of vertex of the graph.
     */
    typedef uint64_t Vertex;

    const Datatype VERTEX_TYPE(MPI_UINT64_T);

    /**
     * Represents directed edge of the graph.
     */
    class Edge {
    public:
        Vertex from;
        Vertex to;

        /**
         * Creates directed edge of the graph.
         * @param from Start vertex.
         * @param to End vertex.
         */
        Edge(Vertex from, Vertex to) : from(from), to(to) {
        };

        /**
         * Copying edge from other.
         * @param orig Original edge.
         */
        Edge(const Edge& orig) : from(orig.from), to(orig.to) {
        }

        virtual ~Edge() {
        }
    };

}
#endif	/* EDGE_H */

