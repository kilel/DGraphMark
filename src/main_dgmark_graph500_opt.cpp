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

#include <cstdlib>
#include <time.h>
#include "generators/SimpleGenerator.h"
#include "controllers/TreeMakerController.h"
#include "tasks/tree-maker/bfs/BFSGraph500Optimized.h"

using namespace std;
using namespace MPI;
using namespace dgmark;

Utils* Utils::instance;

/**
 * @param argc
 * @param argv
 * @return sdf
 */
int main(int argc, char** argv) {
    Init();

    Intracomm *comm = &COMM_WORLD;
    Random* random = new Random(comm->Get_rank());

    int grade;
    int density;
    int startTimes;

    Utils::initialize(comm);
    Utils::parseArguments(&grade, &density, &startTimes, argc, argv);

    SimpleGenerator *gen = new SimpleGenerator(comm, grade, density, random);
    TreeMakerTask *task = new BFSGraph500Optimized(comm);
    Controller *controller = new TreeMakerController(comm, gen, task, startTimes);

    controller->runBenchmark();
    controller->printStatistics();

    Finalize();
    return 0;
}