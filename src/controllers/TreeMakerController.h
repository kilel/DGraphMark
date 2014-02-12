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

#ifndef TREEMAKERCONTROLLER_H
#define	TREEMAKERCONTROLLER_H

#include <mpi.h>
#include "../base/Controller.h"
#include "../tasks/tree-maker/TreeMakerTask.h"
#include "../tasks/tree-maker/ParentTreeValidator.h"


namespace dgmark {
    
    using namespace MPI;

    class TreeMakerController : public Controller {
    public:
        TreeMakerController(Intracomm *comm, GraphGenerator *generator, TreeMakerTask *task);
        TreeMakerController(const TreeMakerController& orig);
        virtual ~TreeMakerController();

        virtual void runBenchmark();
        virtual void printStatistics();
    private:
        Intracomm *comm;
        TreeMakerTask *task;
    };
}

#endif	/* TREEMAKERCONTROLLER_H */

