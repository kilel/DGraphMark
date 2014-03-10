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

#ifndef CONTROLLER_H
#define	CONTROLLER_H

#include <string>
#include "../base/Communicable.h"
#include "../generators/GraphGenerator.h"
#include "../tasks/Task.h"

namespace dgmark {

    class Controller : public Communicable {
    public:

        Controller(Intracomm *comm, GraphGenerator *generator) : Communicable(comm), generator(generator) {
        }

        Controller(const Controller& orig) : Communicable(orig.comm), generator(orig.generator) {
        }

        virtual ~Controller() {
        }

        virtual void runBenchmark() = 0;
        virtual string getStatistics() = 0;
        virtual void printStatistics() = 0;

    protected:
        GraphGenerator *generator;
    };
}

#endif	/* CONTROLLER_H */

