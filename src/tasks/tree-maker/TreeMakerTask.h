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

#ifndef TREEMAKERTASK_H
#define	TREEMAKERTASK_H

#include <mpi.h>
#include "../../base/Task.h"
#include "../../base/Graph.h"
#include "../../base/Log.h"
#include "ParentTree.h"

namespace dgmark {

    class TreeMakerTask : public Task {
    public:
        TreeMakerTask(Intracomm *comm);
        TreeMakerTask(const TreeMakerTask& orig);
        virtual ~TreeMakerTask();

        virtual TaskType getTaskType();

        virtual void open(Graph *newGraph);
        virtual double getTaskOpeningTime();
        virtual ParentTree *run() = 0;
        virtual void close();

        void setRoot(Vertex newRoot);

    protected:
        Graph *graph;
        Vertex root;
        Log log;
    private:
        double taskOpeningTime;
    };

}

#endif	/* TREEMAKERTASK_H */
