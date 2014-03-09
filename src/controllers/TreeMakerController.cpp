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
#include <cmath>
#include <algorithm>
#include <fstream>
#include <ctime>
#include "TreeMakerController.h"
#include "../base/SortedGraph.h"

namespace dgmark {

    TreeMakerController::TreeMakerController(Intracomm *comm, GraphGenerator *generator, TreeMakerTask *task, int numStarts) :
    Controller(comm, generator), task(task), numStarts(numStarts), log(comm) {
        validator = new ParentTreeValidator(comm);

        taskRunningTimes = new vector<double>();
        traversedEdges = new vector<double>();
        validationTimes = new vector<double>();
        marks = new vector<double>();
    }

    TreeMakerController::TreeMakerController(const TreeMakerController& orig) :
    Controller(orig.comm, orig.generator), task(orig.task), numStarts(orig.numStarts), log(comm) {
        validator = new ParentTreeValidator(comm);

        taskRunningTimes = new vector<double>();
        traversedEdges = new vector<double>();
        validationTimes = new vector<double>();
        marks = new vector<double>();
    }

    TreeMakerController::~TreeMakerController() {
        delete validator;

        delete taskRunningTimes;
        delete traversedEdges;
        delete validationTimes;
        delete marks;
    }

    Vertex* TreeMakerController::generateStartRoots(size_t maxStartRoot) {
        log << "Generating roots... ";
        double startTime = Wtime();
        Vertex* startRoots = new Vertex[numStarts];
        if (rank == 0) {
            for (int i = 0; i < numStarts; ++i) {
                startRoots[i] = rand() % maxStartRoot;
            }
        }

        comm->Bcast(startRoots, numStarts, VERTEX_TYPE, 0);
        rootsGenerationTime = Wtime() - startTime;
        log << rootsGenerationTime << " s\n\n";
        return startRoots;
    }

    void TreeMakerController::runBenchmark() {
        log << "Running benchmark\n";
        isLastRunValid = true;

        Graph* graph = generator->generate();
        task->open(graph);
        Vertex* startRoots = generateStartRoots(graph->numGlobalVertex);

        for (int i = 0; i < numStarts; ++i) {
            task->setRoot(startRoots[i]);

            log << "Running tree-making task (" << (i + 1) << "/" << numStarts << ")\n";
            ParentTree *result = task->run();
            bool isValid = validator->validate(result);

            if (isValid) {
                log << "Task mark: " << result->getMark() << "\n\n";
            }

            taskRunningTimes->push_back(result->getTaskRunTime());
            traversedEdges->push_back(result->getTraversedEdges());
            marks->push_back(result->getMark());
            validationTimes->push_back(validator->getValidationTime());

            delete result;

            if (!isValid) {
                isLastRunValid = false;
                break;
            }
        }

        //filling statistics parameters
        generationTime = generator->getGenerationTime();
        distributionTime = generator->getDistributionTime();
        taskOpeningTime = task->getTaskOpeningTime();

        comm->Barrier();

        task->close();
        graph->clear();

        delete graph;

        comm->Barrier();
    }

    string TreeMakerController::getStatistics() {
        stringstream out;
        out.precision(CONTROLLER_PRECISION);
        out.setf(std::ios::fixed, std::ios::floatfield);

        if (isLastRunValid) {
            out << "\n#Statistics\n";
            out << "#\n";
            out << "#Initial data\n";
            out << "initial.grade = " << generator->getGrade() << "\n";
            out << "initial.edgeDensity = " << generator->getDensity() << "\n";
            out << "initial.mpiNodes = " << comm->Get_size() << "\n";
            out << "initial.numStarts = " << numStarts << "\n";
            out << "#\n";
            out << "#Duration of processes\n";
            out << "#\n";
            out << "time.graph.generation = " << generationTime << "\n";
            out << "time.graph.distribution = " << distributionTime << "\n";
            out << "time.generation.roots = " << rootsGenerationTime << "\n";
            out << "time.taskOpening = " << taskOpeningTime << "\n";
            out << "#\n";
            out << getStatistics(taskRunningTimes, "time.taskRun");
            out << "#\n";
            out << getStatistics(validationTimes, "time.validation");
            out << "#\n";
            out << getStatistics(traversedEdges, "traversedEdges");
            out << "#\n";
            out << getStatistics(marks, "mark");

        } else {
            out << "\n#There were errors while running benchmark, no statistics available.\n";
        }

        return out.str();
    }

    string TreeMakerController::getStatistics(vector<double> *data, string statName) {
        double mean = 0;
        for (auto it = data->begin(); it < data->end(); ++it) {
            mean += *it;
        }
        mean /= data->size();

        double stdDeviation = 0;
        for (auto it = data->begin(); it < data->end(); ++it) {
            stdDeviation += pow(*it - mean, 2.);
        }
        stdDeviation = sqrt(stdDeviation / (data->size() - 1));

        double relStdDeviation = stdDeviation / mean;

        vector<double> *sortedData = new vector<double>();
        sortedData->insert(sortedData->begin(), data->begin(), data->end());
        sort(sortedData->begin(), sortedData->end());

        double minimum = sortedData->front();
        double firstQuartile = sortedData->at(sortedData->size() / 4);
        double median = sortedData->at(sortedData->size() / 2);
        double thirdQuartile = sortedData->at(sortedData->size() - sortedData->size() / 4);
        double maximum = sortedData->back();

        delete sortedData;

        stringstream out;
        out.precision(CONTROLLER_PRECISION);
        out.setf(std::ios::fixed, std::ios::floatfield);

        out << statName << ".mean = " << mean << "\n";
        out << statName << ".stdDeviation = " << stdDeviation << "\n";
        out << statName << ".relativeStdDeviation = " << relStdDeviation << "\n";
        out << statName << ".min = " << minimum << "\n";
        out << statName << ".firstQuartile = " << firstQuartile << "\n";
        out << statName << ".median = " << median << "\n";
        out << statName << ".thirdQuertile = " << thirdQuartile << "\n";
        out << statName << ".max = " << maximum << "\n";

        return out.str();
    }

    void TreeMakerController::printStatistics() {
        string statistics = getStatistics();
        log << statistics;

        time_t datetime = time(0);
        tm *date = localtime(&datetime);

        int mkdirResult = system("mkdir -p dgmarkStatistics");

        stringstream fileName;
        fileName << "dgmarkStatistics/dgmark_stat_" 
                << (date->tm_year + 1900) << "-"
                << (date->tm_mon + 1) << "-"
                << date->tm_mday << "_"
                << date->tm_hour << "-"
                << date->tm_min << "-"
                << date->tm_sec
                << ".properties";

        ofstream fileOut;
        fileOut.open(fileName.str());
        fileOut << statistics;
        fileOut.close();
    }

}

