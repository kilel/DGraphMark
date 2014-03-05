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

#include "Graph.h"
#include "Utils.h"
#include "assert.h"

namespace dgmark {

    Graph::Graph(Intracomm *comm, int grade, int density) : Communicable(comm),
    grade(grade), density(density) {
        Utils util(comm);
        util.setGrade(grade);
        numLocalVertex = util.getNumLocalVertex();
        numGlobalVertex = util.getNumGlobalVertex();
        distributedEdges = 0;
        edges = new vector<Edge*>();
    }

    Graph::Graph(const Graph& orig) : Communicable(orig.comm), edges(orig.edges),
    grade(orig.grade), density(orig.density), numLocalVertex(orig.numLocalVertex),
    numGlobalVertex(orig.numGlobalVertex) {
    }

    Graph::Graph(const Graph *orig) : Communicable(orig->comm), edges(orig->edges),
    grade(orig->grade), density(orig->density), numLocalVertex(orig->numLocalVertex),
    numGlobalVertex(orig->numGlobalVertex) {
    }

    Graph::~Graph() {
    }

    void Graph::clear() {
        edges->clear();
        delete edges;
    }

    void Graph::distribute() {
        size_t initialEdgesCount = edges->size();
        Vertex *edgeMemory = new Vertex[2];

        for (size_t i = distributedEdges; i < initialEdgesCount; ++i) {
            //send edge
            sendEdge(edges->at(i), edgeMemory);
            //try to recieve edge (by probe)
            while (probeReadEdge(edgeMemory));
        }
        comm->Barrier(); //important. Wait, till all sends are done.
        //recieve all other edges.
        while (probeReadEdge(edgeMemory));

        delete edgeMemory;
        distributedEdges = edges->size();
        comm->Barrier(); //important. Waits to complete distribution.
    }

    void Graph::sendEdge(Edge* edge, Vertex* memory) {
        memory[0] = edge->to;
        memory[1] = edge->from;
        int rankTo = Utils::getVertexRank(memory[0]);
        if (rankTo == rank) {
            edges->push_back(new Edge(memory[0], memory[1]));
        } else {
            comm->Send(memory, 2, VERTEX_TYPE, rankTo, 0);
        }
    }

    bool Graph::probeReadEdge(Vertex* memory) {
        Status status;
        comm->Iprobe(ANY_SOURCE, ANY_TAG, status);

        int inputCount = status.Get_count(VERTEX_TYPE);
        if (inputCount > 0) {
            assert(inputCount == 2);
            comm->Recv(memory, 2, VERTEX_TYPE, status.Get_source(), 0, status);
            edges->push_back(new Edge(memory[0], memory[1]));
            return true;
        } else {
            return false;
        }
    }
}
