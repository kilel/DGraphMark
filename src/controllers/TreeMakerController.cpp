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

#include "TreeMakerController.h"

namespace dgmark {

    TreeMakerController::TreeMakerController(Intracomm *comm, GraphGenerator *generator, TreeMakerTask *task) :
    Controller(generator), comm(comm), task(task) {
    }

    TreeMakerController::TreeMakerController(const TreeMakerController& orig) :
    Controller(orig.generator), comm(orig.comm), task(orig.task) {
    }

    TreeMakerController::~TreeMakerController() {
    }

    void TreeMakerController::runBenchmark() {
        /*Graph* graph = generator->generate();
        
        ParentTreeValidator *validator = new ParentTreeValidator(comm);
        ParentTree *result = (ParentTree *) task->run();
        printf("%d", validator->validate(result));
        
        delete graph; */
        
        if(comm->Get_rank() == 0) {
            printf("\nRunning benchmark\n");
        }
    }

    void TreeMakerController::printStatistics() {
        if(comm->Get_rank() == 0) {
            printf("\nStatistics\n");
        }

    }
}

