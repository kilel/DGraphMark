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

#include <sstream>
#include <fstream>
#include "../util/Statistics.h"
#include "Benchmark.h"

namespace dgmark {

    Benchmark::Benchmark(Intracomm *comm, Task *task, Validator *validator, Graph *graph, int numStarts) :
    Communicable(comm), task(task), validator(validator), graph(graph), numStarts(numStarts), log(comm) {
        taskRunningTimes = new vector<double>();
        validationTimes = new vector<double>();
        marks = new vector<double>();
    }

    Benchmark::Benchmark(const Benchmark& orig) : Communicable(orig.comm), task(orig.task),
    validator(orig.validator), graph(orig.graph), numStarts(orig.numStarts), log(orig.comm),
    taskRunningTimes(orig.taskRunningTimes), validationTimes(orig.validationTimes), marks(orig.marks) {
    }

    Benchmark::~Benchmark() {
        delete taskRunningTimes;
        delete validationTimes;
        delete marks;
    }

    void Benchmark::run() {
        log << "Running " << task->getName() << " benchmark\n";
        isSuccessfullyFinished = true;
        task->open(graph);
        log << "\n";
        for (int startIndex = 0; startIndex < numStarts; ++startIndex) {
            bool isValid = runSingleTask(startIndex);
            if (!isValid) {
                isSuccessfullyFinished = false;
                break;
            }
        }

        comm->Barrier(); // wait, while all processes are finished task.
        task->close();
        comm->Barrier(); // wait for closing task for all processes.
    }

    double Benchmark::getTaskOpeningTime() {
        return task->getTaskOpeningTime();
    }

    string Benchmark::getStatistics() {
        stringstream out;
        out.precision(statisticsPrecision);
        out.setf(ios::fixed, ios::floatfield);
        string name = task->getName();

        out << "#\n";
        out << "#" << name << " benchmark\n";
        out << "#\n";

        if (isSuccessfullyFinished) {
            out << name << ".time.taskOpening = " << task->getTaskOpeningTime() << "\n";
            out << "#\n";
            out << getStatistics(taskRunningTimes, name + (".time.singleRun"), ios::fixed);
            out << "#\n";
            out << getStatistics(validationTimes, name + (".time.validation"), ios::fixed);
            out << "#\n";
            out << getStatistics(marks, name + (".mark"), ios::fixed);
        } else {
            out << "\n#There were errors while running benchmark, no statistics available.\n";
        }

        return out.str();
    }

    string Benchmark::getStatistics(vector<double> *data, string statName,
            const ios::fmtflags floatfieldFlag) {
        Statistics* stat = new Statistics(data);

        stringstream out;
        out.precision(statisticsPrecision);
        out.setf(floatfieldFlag, ios::floatfield);

        out << statName << ".mean = " << stat->getMean() << "\n";
        out << statName << ".stdDeviation = " << stat->getStdDeviation() << "\n";
        out << statName << ".relativeStdDeviation = " << stat->getRelStdDeviation() << "\n";
        out << statName << ".min = " << stat->getMinimum() << "\n";
        out << statName << ".firstQuartile = " << stat->getFirstQuartile() << "\n";
        out << statName << ".median = " << stat->getMedian() << "\n";
        out << statName << ".thirdQuertile = " << stat->getThirdQuartile() << "\n";
        out << statName << ".max = " << stat->getMaximum() << "\n";

        delete stat;
        return out.str();
    }

}

