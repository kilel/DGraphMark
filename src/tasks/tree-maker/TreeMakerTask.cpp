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

    TreeMakerTask::TreeMakerTask(Intracomm *comm) : Task(comm), log(comm) {
    }

    TreeMakerTask::TreeMakerTask(const TreeMakerTask& orig) : Task(orig.comm), log(orig.comm) {
    }

    TreeMakerTask::~TreeMakerTask() {
    }

    TaskType TreeMakerTask::getTaskType() {
        return TaskType::PARENT_TREE;
    }

    void TreeMakerTask::open(Graph *newGraph) {
        log << "Opening task... ";
        double startTime = Wtime();
        graph = newGraph;
        taskOpeningTime = Wtime() - startTime;
        log << taskOpeningTime << " s\n";
    }

    void TreeMakerTask::close() {
        log << "Closing task... \n";
    }

    double TreeMakerTask::getTaskOpeningTime() {
        return taskOpeningTime;
    }

    void TreeMakerTask::setRoot(Vertex newRoot) {
        root = newRoot;
    }

}
