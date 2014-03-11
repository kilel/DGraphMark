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

#include <string.h>
#include "RMAWindow.h"

namespace dgmark {

    template<class T>
    RMAWindow<T>::RMAWindow(Intracomm *comm, size_t dataSize, Datatype dataType) :
    Communicable(comm), dataSize(dataSize), dataType(dataType) {
        Aint sizeInBytes = dataSize * sizeof (T);
        data = (T*) Alloc_mem(sizeInBytes, INFO_NULL);
        memset(data, 0, sizeInBytes);
        win = new Win();
        *win = Win::Create(data, sizeInBytes, sizeof (T), INFO_NULL, *comm);
    }

    template<class T>
    RMAWindow<T>::RMAWindow(const RMAWindow& orig) :
    Communicable(orig.comm), data(orig.data), dataSize(orig.dataSize), dataType(orig.dataType), win(orig.win) {
    }

    template<class T>
    RMAWindow<T>::~RMAWindow() {
        win->Free();
    }

    template<class T>
    T* RMAWindow<T>::getData() {
        return data;
    }

    template<class T>
    size_t RMAWindow<T>::getDataSize() {
        return dataSize;
    }

    template<class T>
    void RMAWindow<T>::clean() {
        Free_mem(data);
    }

    template<class T>
    void RMAWindow<T>::fenceOpen(int assertType) {
        win->Fence(MODE_NOPRECEDE | assertType);
    }

    template<class T>
    void RMAWindow<T>::fenceClose(int assertType) {
        win->Fence(MODE_NOSUCCEED | assertType);
    }

    template<class T>
    void RMAWindow<T>::get(T* dataToGet, size_t dataLength, int targetRank, size_t shift) {
        win->Get(dataToGet, dataLength, dataType, targetRank, shift, dataLength, dataType);
    }

    template<class T>
    void RMAWindow<T>::put(T* dataToPut, size_t dataLength, int targetRank, size_t shift) {
        win->Put(dataToPut, dataLength, dataType, targetRank, shift, dataLength, dataType);
    }

    template<class T>
    void RMAWindow<T>::accumulate(T* dataToAcc, size_t dataLength, int targetRank, size_t shift, const Op &operation) {
        win->Accumulate(dataToAcc, dataLength, dataType, targetRank, shift, dataLength, dataType, operation);
    }

    template<class T>
    void RMAWindow<T>::sendIsFenceNeeded(bool value, int tag) {
        for (int node = 0; node < size; ++node) {
            if (rank != node) {
                comm->Send(&value, 1, BOOL, node, tag);
            }
        }
    }

    template<class T>
    bool RMAWindow<T>::recvIsFenceNeeded(int tag) {
        bool value;
        comm->Recv(&value, 1, BOOL, ANY_SOURCE, tag);
        return value;
    }

}
