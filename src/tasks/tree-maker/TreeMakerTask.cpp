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

#include "TreeMakerTask.h"

namespace dgmark {

    TreeMakerTask::TreeMakerTask(Intracomm *comm, Graph *graph, Vertex root) :
    comm(comm), graph(graph), root(root) {
    }

    TreeMakerTask::TreeMakerTask(const TreeMakerTask& orig) :
    comm(orig.comm), graph(orig.graph), root(orig.root) {
    }

    TreeMakerTask::~TreeMakerTask() {
    }

}

