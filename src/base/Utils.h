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

namespace dgmark {

    class Utils : Communicable {
    public:

        Utils(Intracomm *comm) : Communicable(comm) {
        }

        Utils(const Utils& orig) : Communicable(orig.comm) {
        }

        virtual ~Utils() {
        }

        static void initialize(Intracomm *comm, int grade) {
            instance = new Utils(comm);
            instance->grade = grade;

            //check later, that size is 2^n; and not zero.
            int commSize = instance->size;
            instance->commGrade = 0;
            while (commSize != 1) {
                commSize >>= 1;
                ++instance->commGrade;
            }
            //check, that commGrade > grade
            instance->diffGrade = instance->grade - instance->commGrade;

            instance->numLocalVertex = 1 << (instance->diffGrade);
            instance->numGlobalVertex = 1 << (instance->grade); //number of vertex globally.
        }

        static Vertex vertexToLocal(Vertex globalVertex) {
            int rank = globalVertex >> instance->diffGrade;
            return (rank << instance->diffGrade) ^ globalVertex;
        }

        static Vertex vertexToGlobal(Vertex localVertex) {
            return vertexToGlobal(instance->rank, localVertex);
        }

        static Vertex vertexToGlobal(int rank, Vertex localVertex) {
            return (rank << instance->diffGrade) | localVertex;
        }

        static Vertex getNumLocalVertex() {
            return instance->numLocalVertex;
        }

        static Vertex getNumGlobalVertex() {
            return instance->numGlobalVertex;
        }
        
        static int getRank() {
            return instance->rank;
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

