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

#ifndef LOG_H
#define	LOG_H

#include "Communicable.h"
#include "Edge.h"
#include <cstdio>
#include <iostream>


namespace dgmark {
    using namespace std;

    class Log : public Communicable {
    public:

        Log(Intracomm *comm) : Communicable(comm), rank(comm->Get_rank()) {
        }

        Log(const Log& orig) : Communicable(orig.comm), rank(orig.comm->Get_rank()) {
        }

        virtual ~Log() {
        }

        Log& operator<<(char* data) {
            if (rank == 0)
                cout << data;
            return *this;
        }
        
        Log& operator<<(const char data[]) {
            if (rank == 0)
                cout << data;
            return *this;
        }

        Log& operator<<(Vertex data) {
            if (rank == 0)
                cout << data;
            return *this;
        }

        Log& operator<<(int data) {
            if (rank == 0)
                cout << data;
            return *this;
        }

        Log& operator<<(double data) {
            if (rank == 0)
                cout << data;
            return *this;
        }


    private:
        int rank;

    };

}

#endif	/* LOG_H */

