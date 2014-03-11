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

#ifndef SEARCHTASK_H
#define	SEARCHTASK_H

#include "ParentTree.h"
#include "../Task.h"
#include "../../graph/SortedGraph.h"
#include "../../util/Log.h"

namespace dgmark {

    class SearchTask : public Task {
    public:
        SearchTask(Intracomm *comm);
        SearchTask(const SearchTask& orig);
        virtual ~SearchTask();

        virtual TaskType getTaskType();

        virtual void open(Graph *newGraph);
        virtual double getTaskOpeningTime();
        virtual ParentTree *run() = 0;
        virtual void close();

        void setRoot(Vertex newRoot);

    protected:
        SortedGraph *graph;
        Vertex root;
        Log log;
    private:
        double taskOpeningTime;
    protected:
        Vertex numLocalVertex;
    };

}

#endif	/* SEARCHTASK_H */

