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

#ifndef GRAPHTASK_H
#define	GRAPHTASK_H

#include "Graph.h"
#include "Result.h"

namespace dgmark {

    /**
     * Represents the task.
     */
    class Task : public Classifieble, public Communicable {
    public:

        Task(Intracomm *comm) : Communicable(comm) {
        }

        virtual ~Task() {
        }

        /**
         * Opens task for graph.
         * @param graph Graph to run task on.
         */
        virtual void open(Graph *graph) = 0;

        /**
         * Run task on graph. Be sure to initialize all parameters before run.
         * @return 
         */
        virtual Result* run() = 0;

        /**
         * Close task for graph.
         */
        virtual void close() = 0;
    };

}

#endif	/* GRAPHTASK_H */

