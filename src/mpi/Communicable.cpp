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

#include "Communicable.h"

namespace dgmark {

    Communicable::Communicable(Intracomm *comm) : comm(comm) {
        if (comm != 0) {
            rank = comm->Get_rank();
            size = comm->Get_size();
        } else {
            rank = size = 0;
        }
    }

    Communicable::Communicable(const Communicable& orig) : comm(orig.comm) {
        if (comm != 0) {
            rank = comm->Get_rank();
            size = comm->Get_size();
        } else {
            rank = size = 0;
        }
    }

    Communicable::~Communicable() {
    }

    void Communicable::requestSynch(bool isSynchNeeded, int toRank, int synchTag) {
        comm->Send(&isSynchNeeded, 1, BOOL, toRank, synchTag);
    }

    void Communicable::requestSynch(bool isSynchNeeded, int synchTag) {
        for (int node = 0; node < size; ++node) {
            if (node != rank) {
                comm->Send(&isSynchNeeded, 1, BOOL, node, synchTag);
            }
        }
    }

    void Communicable::endSynch(int synchTag) {
        requestSynch(false, synchTag);
    }

    bool Communicable::waitSynch(int synchTag, int fromRank, Status &status) {
        bool value;
        comm->Recv(&value, 1, BOOL, ANY_SOURCE, synchTag, status);
        return value;
    }

    bool Communicable::waitSynch(int synchTag, int fromRank) {
        Status status;
        return waitSynch(synchTag, fromRank, status);
    }

    bool Communicable::waitSynch(int synchTag, Status &status) {
        return waitSynch(synchTag, ANY_SOURCE, status);
    }

    bool Communicable::waitSynch(int synchTag) {
        Status status;
        return waitSynch(synchTag, ANY_SOURCE, status);
    }

    bool Communicable::probeSynch(int synchTag, Status &status) {
        if (comm->Iprobe(ANY_SOURCE, synchTag, status)) {
            if (status.Get_count(BOOL) > 0) {
                return waitSynch(synchTag, status.Get_source(), status);
            }
        }
        return false;
    }

    bool Communicable::probeSynch(int synchTag) {
        Status status;
        return probeSynch(synchTag, status);
    }

    void Communicable::sendVertex(Vertex vertex, int toRank, int tag) {
        comm->Send(&vertex, 1, VERTEX_TYPE, toRank, tag);
    }

    Vertex Communicable::waitVertex(int fromRank, int tag, Status &status) {
        Vertex vertex;
        comm->Recv(&vertex, 1, VERTEX_TYPE, fromRank, tag, status);
        return vertex;
    }

    Vertex Communicable::waitVertex(int fromRank, int tag) {
        Status status;
        return waitVertex(fromRank, tag, status);
    }

    Vertex Communicable::waitVertex(int tag, Status &status) {
        return waitVertex(ANY_SOURCE, tag, status);
    }

    Vertex Communicable::waitVertex(int tag) {
        return waitVertex(ANY_SOURCE, tag);
    }

}
