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
    class Graph : public Communicable {
    public:
        vector<Edge*> *edges;
        const int grade;
        const int density;
        Vertex numLocalVertex;
        Vertex numGlobalVertex;

        /**
         * Creates graph.
         * @param comm communicator.
         * @param grade grade of graph (log[2] numGlobalVertices)
         * @param density density of graph (mean num of edges per vertice)
         */
        Graph(Intracomm *comm, int grade, int density);

        /**
         * Copying graph. 
         * Note, that graph copies reference to original edgelist.
         * 
         * @param orig Original graph.
         */
        Graph(const Graph& orig);

        /**
         * Copying graph. 
         * Note, that graph copies reference to original edgelist.
         * 
         * @param orig Original graph.
         */
        Graph(const Graph *orig);

        virtual ~Graph();

        /**
         * Clears edgelist.
         */
        virtual void clear();

        /**
         * Distributes graph. 
         * Each edge (myRank|localNode -> hisRank|hisLocal) sends to hisRank as
         * (hisRank|hisLocal -> myRank|localNode).
         */
        virtual void distribute();

        /**
         * Finds, if graph is distributed, or not;
         * 
         * @return true, is graph is distribuded.
         */
        bool isDistributed();

        /**
         * Converts global vertex to local one.
         * 
         * @param globalVertex Global vertex
         * @return local vertex
         */
        Vertex vertexToLocal(Vertex globalVertex);

        /**
         * Converts local vertex to global one. (Assumes, that vertex from current process).
         * 
         * @param localVertex local vertex from current process
         * @return global vertex.
         */
        Vertex vertexToGlobal(Vertex localVertex);

        /**
         * Converts local vertex to global one.
         * 
         * @param rank rank of process.
         * @param localVertex local vertex of the process.
         * @return global vertex.
         */
        Vertex vertexToGlobal(int rank, Vertex localVertex);
        
        /**
         * Returns rank of the global vertex.
         * @param globalVertex global vertex.
         * @return rank of the global vertex.
         */
        Vertex vertexRank(Vertex globalVertex);

    private:
        int diffGrade;
        size_t distributedEdges;

        void initialize();

        /**
         * Transfers edge to it's destination.
         * @param edge Edge
         * @param memory allocated memory to send edge.
         */
        void sendEdge(Edge *edge, Vertex *memory);

        /**
         * Tries to read edge from communicator.
         * @param memory allocated memory to read edge.
         * @return true, if edge is readed, false if there are nothig to read.
         */
        bool probeReadEdge(Vertex *memory);
    };
}

#endif	/* GRAPH_H */
