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

#ifndef COMMUNICABLE_H
#define	COMMUNICABLE_H

#include <mpi.h>

namespace dgmark {

    using namespace MPI;

    class Communicable {
    public:

        Communicable(Intracomm *comm) : comm(comm) {
            if (comm != 0) {
                rank = comm->Get_rank();
                size = comm->Get_size();
            } else {
                rank = size = 0;
            }
        }

        Communicable(const Communicable& orig) : comm(orig.comm) {
            if (comm != 0) {
                rank = comm->Get_rank();
                size = comm->Get_size();
            } else {
                rank = size = 0;
            }
        }

        virtual ~Communicable() {
        }

    protected:
        Intracomm *comm;
        int rank;
        int size;
    };
}

#endif	/* COMMUNICABLE_H */

