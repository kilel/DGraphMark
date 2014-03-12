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

#ifndef BENCHMARK_H
#define	BENCHMARK_H

#include <string>
#include "../mpi/Communicable.h"
#include "../task/Task.h"
#include "../task/Validator.h"
#include "../util/Log.h"

namespace dgmark {

    class Benchmark : public Communicable {
    public:
        Benchmark(Intracomm *comm, Task *task, Validator *validator, Graph *graph, int numStarts);
        Benchmark(const Benchmark& orig);
        virtual ~Benchmark();

        void run();
        double getTaskOpeningTime();
        virtual string getStatistics();

    protected:
        static const int statisticsPrecision = 5;
        
        Log log;
        Task *task;
        Validator *validator;
        Graph *graph;

        int numStarts;
        bool isSuccessfullyFinished;

        vector<double> *taskRunningTimes;
        vector<double> *validationTimes;
        vector<double> *marks;

        virtual bool runSingleTask(int startIndex) = 0;
        string getStatistics(vector<double> *data, string name, const ios::fmtflags floatfieldFlag = ios::scientific);
    private:

    };
}

#endif	/* BENCHMARK_H */

