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

#include <cstdlib>
#include <time.h>
#include <mpi.h>
#include "generators/SimpleGenerator.h"

using namespace std;
using namespace MPI;
using namespace dgmark;


void printGraph(Graph *graph, Intracomm *comm) {
    vector<Edge*> *edges = graph->edges;
    int rank = comm->Get_rank();
    
    for(size_t i = 0; i < edges->size(); ++i) {
        Edge *edge = edges->at(i);
        printf("%d: %ld -> %ld\n", rank, edge->from, edge->to);
    }
}

/**
 * @param argc
 * @param argv
 * @return sdf
 */
int main(int argc, char** argv) {
    Init();
    
    Intracomm *comm = &COMM_WORLD;
    srand(time(0) + comm->Get_rank());
    
    int grade = 2;
    int density = 3;
    
    SimpleGenerator *gen = new SimpleGenerator(comm, grade, density);
    Graph* graph = gen->generate();
    printGraph(graph, comm);

    Finalize();
    return 0;
}