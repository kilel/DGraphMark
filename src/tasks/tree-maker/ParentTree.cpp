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

#include <vector>

#include "ParentTree.h"
namespace dgmark {

    ParentTree::ParentTree(Intracomm *comm, Vertex *parent, size_t parentSize, double taskRunTime) :
    Result(comm), parent(parent), parentSize(parentSize), taskRunTime(taskRunTime) {
    }

    ParentTree::ParentTree(const ParentTree& orig) :
    Result(orig.comm), parent(orig.parent), parentSize(orig.parentSize), taskRunTime(orig.taskRunTime) {
    }

    ParentTree::~ParentTree() {
        //delete parent;
        Free_mem(parent);
    }

    Vertex* ParentTree::getParent() {
        return parent;
    }

    double ParentTree::getMark() {
        return traversedEdges / taskRunTime;
    }

    double ParentTree::getTaskRunTime() {
        return taskRunTime;
    }

    TaskType ParentTree::getTaskType() {
        return TaskType::PARENT_TREE;
    }

    void ParentTree::setTraversedEdges(size_t newTraversedEdges) {
        traversedEdges = newTraversedEdges;
    }

    double ParentTree::getTraversedEdges() {
        return traversedEdges;
    }
}

