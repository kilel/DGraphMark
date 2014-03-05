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

#ifndef UTILS_H
#define	UTILS_H

#include "Edge.h"
#include <assert.h>

namespace dgmark {

    class Utils : Communicable {
    public:

        Utils(Intracomm *comm) : Communicable(comm) {
            size = comm->Get_size();
            rank = comm->Get_rank();
            if (((size - 1) & size) != 0) {
                if (rank == 0) {
                    printf("Number of MPI nodes must be 2^n. %d is not.\n", size);
                }
                assert(false);
            }
            int commSize = size;
            commGrade = 0;
            while (commSize != 1) {
                commSize >>= 1;
                ++commGrade;
            }

        }

        Utils(const Utils& orig) : Communicable(orig.comm) {
        }

        virtual ~Utils() {
        }

        static void initialize(Intracomm *comm) {
            instance = new Utils(comm);
        }

        static void parseArguments(int *grade, int *density, int *numStarts, int argc, char** argv) {
            if (instance->rank == 0) {
                *grade = 8;
                *density = 16;
                *numStarts = 16;

                if (argc >= 2) {
                    *grade = atoi(argv[1]);
                }
                if (argc >= 3) {
                    *density = atoi(argv[2]);
                }
                if (argc >= 4) {
                    *numStarts = atoi(argv[3]);
                }
            }

            instance->comm->Bcast(grade, 1, INT, 0);
            instance->comm->Bcast(density, 1, INT, 0);
            instance->comm->Bcast(numStarts, 1, INT, 0);

            instance->setGrade(*grade);
        }

        static void printGraph(Graph *graph) {
            vector<Edge*> *edges = graph->edges;

            for (size_t i = 0; i < edges->size(); ++i) {
                Edge *edge = edges->at(i);
                printf("%d: %ld -> %ld\n", instance->rank, edge->from, edge->to);
            }
        }

        static inline Vertex vertexToLocal(Vertex globalVertex) {
            int rank = getVertexRank(globalVertex);
            return (rank << instance->diffGrade) ^ globalVertex;
        }

        static inline Vertex vertexToGlobal(Vertex localVertex) {
            return vertexToGlobal(instance->rank, localVertex);
        }

        static inline Vertex vertexToGlobal(int rank, Vertex localVertex) {
            return (rank << instance->diffGrade) | localVertex;
        }

        static inline Vertex getVertexRank(Vertex globalVertex) {
            return globalVertex >> instance->diffGrade;
        }

        Vertex getNumLocalVertex() {
            return numLocalVertex;
        }

        Vertex getNumGlobalVertex() {
            return numGlobalVertex;
        }

        void setGrade(int newGrade) {
            assert(commGrade < newGrade);

            grade = newGrade;
            diffGrade = grade - commGrade;

            numLocalVertex = 1 << (diffGrade);
            numGlobalVertex = 1 << (grade); //number of vertex globally.
        }

    private:
        static Utils *instance;
        int grade;
        int commGrade;
        int diffGrade;
        Vertex numLocalVertex;
        Vertex numGlobalVertex;

    };

}
#endif	/* UTILS_H */

