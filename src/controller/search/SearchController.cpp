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

#include <assert.h>
#include <sstream>
#include <cstdlib>
#include "SearchController.h"
#include "../../benchmark/search/SearchBenchmark.h"

#include "../../generator/UniformGenerator.h"

namespace dgmark {

	GraphGenerator* SearchController::createGenerator()
	{
		return new UniformGenerator(comm);
	}

	SearchController::SearchController(Intracomm *comm, int argc, char **argv) :
	Controller(comm, argc, argv)
	{
		generator = createGenerator();
	}

	SearchController::SearchController(const SearchController& orig) :
	Controller(orig)
	{
		generator = createGenerator();
	}

	SearchController::~SearchController()
	{
		delete generator;
	}

	void SearchController::run(vector<Task*> *tasks)
	{
		Graph *graph = generator->generate(grade, density);

		vector<Benchmark*> *benchmarks = new vector<Benchmark*>(0);

		for (vector<Task*>::iterator taskIt = tasks->begin(); taskIt < tasks->end(); ++taskIt) {
			Task *task = *taskIt;
			assert(task->getTaskType() == SEARCH);
			Benchmark* benchmark = new SearchBenchmark(comm, (SearchTask*) task, graph, numStarts);
			benchmarks->push_back(benchmark);
		}

		Controller::run(benchmarks);
		Controller::clean(benchmarks);

		graph->clear();
		delete graph;
	}

	void SearchController::clean(vector<Task*> *tasks)
	{
		for (size_t taskId = 0; taskId < tasks->size(); ++taskId) {
			delete (tasks->at(taskId));
		}
		delete tasks;
	}

	string SearchController::getSpecificStatistics()
	{
		stringstream out;
		out.precision(CONTROLLER_PRECISION);
		out.setf(ios::fixed, ios::floatfield);

		out << "#\n";
		out << "#Duration of processes\n";
		out << "#\n";
		out << "time.graph.generation = " << generator->getGenerationTime() << "\n";
		out << "time.graph.distribution = " << generator->getDistributionTime() << "\n";

		return out.str();
	}

}

