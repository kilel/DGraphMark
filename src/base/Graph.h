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
         * Creates graph with no edges.
         */
        Graph(Intracomm *comm, int grade, int density);

        /**
         * Copying graph. 
         * Note, that graph copies reference to original edgelist.
         * @param orig Original graph.
         */
        Graph(const Graph& orig);

        /**
         * Copying graph. 
         * Note, that graph copies reference to original edgelist.
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
         * @return true, is graph is distribuded.
         */
        bool isDistributed();
        
    private:
        size_t distributedEdges = 0;
        
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
