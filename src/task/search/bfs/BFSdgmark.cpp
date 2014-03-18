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

#include <assert.h>
#include <string.h>

#include "BFSdgmark.h"

namespace dgmark {

    BFSdgmark::BFSdgmark(Intracomm *comm) : SearchTask(comm) {
    }

    BFSdgmark::BFSdgmark(const BFSdgmark& orig) : SearchTask(orig.comm) {
    }

    BFSdgmark::~BFSdgmark() {
    }

    bool BFSdgmark::processLocalChild(Vertex *queue, Vertex *parent, Vertex currVertex, Vertex child) {
        Vertex childLocal = graph->vertexToLocal(child);

        if (parent[childLocal] == graph->numGlobalVertex) {
            parent[childLocal] = currVertex;
            queue[queue[1]] = childLocal;
            ++queue[1];
            return true;
        } else {
            return false;
        }
    }

    void BFSdgmark::alignQueue(Vertex *queue) {
        for (size_t qIndex = queue[0]; qIndex < queue[1]; ++qIndex) {
            queue[qIndex - queue[0] + 2] = queue[qIndex];
        }
        queue[1] = queue[1] - queue[0] + 2;
        queue[0] = 2;
    }

    Vertex BFSdgmark::getQueueSize() {
        return numLocalVertex * 3 + 2;
    }

}
