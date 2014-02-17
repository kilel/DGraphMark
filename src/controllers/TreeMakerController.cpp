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
#include <cstdlib>

namespace dgmark {

    TreeMakerController::TreeMakerController(Intracomm *comm, GraphGenerator *generator, TreeMakerTask *task, int numStarts) :
    Controller(comm, generator), task(task), numStarts(numStarts) {
        validator = new ParentTreeValidator(comm);
        log = new Log(comm);
    }

    TreeMakerController::TreeMakerController(const TreeMakerController& orig) :
    Controller(orig.comm, orig.generator), task(orig.task), numStarts(orig.numStarts) {
        validator = new ParentTreeValidator(comm);
        log = new Log(comm);
    }

    TreeMakerController::~TreeMakerController() {
        delete validator;
        delete log;
    }

    Vertex* TreeMakerController::generateStartRoots() {
        Vertex* startRoots = new Vertex[numStarts];
        if (comm->Get_rank() == 0) {
            for (int i = 0; i < numStarts; ++i) {
                startRoots[i] = rand() % comm->Get_size();
            }
        }

        comm->Bcast(startRoots, numStarts, VERTEX_TYPE, 0);
        return startRoots;
    }

    void TreeMakerController::runBenchmark() {
        int rank = comm->Get_rank();
        *log << "Running benchmark\n";


        Graph* graph = generator->generate();
        *log << "Graph generated\n";

        task->open(graph);
        *log << "Task openned\n";

        Vertex* startRoots = generateStartRoots();
        *log << "Roots generated\n\n";

        for (int i = 0; i < numStarts; ++i) {
            task->setRoot(startRoots[i]);

            *log << "Running BFS from " << startRoots[i] << "...\n";

            ParentTree *result = task->run();
            *log << "BFS mark: " << result->getMark() << "\n";

            *log << "Validating...";
            bool isValid = validator->validate(result);

            if (isValid) {
                *log << "Success\n\n";
            } else {
                *log << "\nError validating";
                delete result;
                delete graph;
                return;
            }

            delete result;

        }
        
        *log << "Task work is done\n";
        
        task->close();
        delete graph;
        
        comm->Barrier();
    }

    void TreeMakerController::printStatistics() {
        //logString("Statistics:");
    }

}

