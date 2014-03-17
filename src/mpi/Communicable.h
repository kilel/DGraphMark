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

        Communicable(Intracomm *comm);
        Communicable(const Communicable& orig);
        virtual ~Communicable();
        
        void requestSynch(bool isSynchNeeded, int synchTag);
        void requestSynch(bool isSynchNeeded, int toRank, int synchTag);
        void endSynch(int synchTag);
        bool waitSynch(int synchTag);
        bool probeSynch(int synchTag);

    protected:
        Intracomm *comm;
        int rank;
        int size;
    };
}

#endif	/* COMMUNICABLE_H */

