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

#ifndef PARENTTREE_H
#define	PARENTTREE_H

#include <vector>
#include <mpi.h>

#include "../../base/Edge.h"
#include "../../base/Result.h"


namespace dgmark {
    
    using namespace std;
    using namespace MPI;

    class ParentTree : public Result{
    public:
        /**
         * Creates parent tree (result for tree-makers).
         * @param comm Communacator to work with.
         * @param parent Array of parents to verticies.
         * @param duration Duration of tree-making task in seconds (MPI::Wtime)
         */
        ParentTree(Intracomm *comm, vector<Vertex> *parent, double duration);
        ParentTree(const ParentTree& orig);
        virtual ~ParentTree();

        vector<Vertex>* getParent() {
            return parent;
        }
        
        virtual double getMark() {
            return mark;
        }
        
        virtual TaskType getTaskType() {
            return TaskType::PARENT_TREE;
        }
    private:
        vector<Vertex> *parent;
        double mark;
        
        /**
         * Calculates mark for thos result.
         * @param duration duration of task.
         * @return mark.
         */
        double calculateMark(double duration);
    };
}

#endif	/* PARENTTREE_H */

