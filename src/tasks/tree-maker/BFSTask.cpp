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

#include "BFSTask.h"

namespace dgmark {

    BFSTask::BFSTask(Intracomm *comm) : TreeMakerTask(comm) {
    }

    BFSTask::BFSTask(const BFSTask& orig) : TreeMakerTask(orig.comm) {
    }

    BFSTask::~BFSTask() {
    }

    ParentTree* BFSTask::run() {
        log << "Running BFS from " << root << "\n";
        vector<Vertex> *parent = new vector<Vertex>();
        double startTime = Wtime();


        double taskRunTime = Wtime() - startTime;

        ParentTree *parentTree = new ParentTree(comm, parent, taskRunTime);
        log << "BFS time: " << taskRunTime << " s\n";
        return parentTree;
    }
}
