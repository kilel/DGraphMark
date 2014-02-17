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

    ParentTree::ParentTree(Intracomm *comm, vector<Vertex> *parent, double duration) :
    Result(comm), parent(parent) {
        calculateMark(duration);
    }

    ParentTree::ParentTree(const ParentTree& orig) :
    Result(orig.comm), parent(orig.parent), mark(orig.mark) {
    }

    ParentTree::~ParentTree() {
        parent->clear();
        delete parent;
    }

    double ParentTree::calculateMark(double duration) {
        //TODO
    }
}

