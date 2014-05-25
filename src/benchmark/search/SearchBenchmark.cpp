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
#include "../../util/Random.h"
#include "SearchBenchmark.h"
#include <cstdlib>
#include <ctime>

#include "../../task/search/ParentTreeValidator.h"

namespace dgmark {

	SearchBenchmark::SearchBenchmark(Intracomm *comm,
					SearchTask *task,
					Graph *graph,
					int numStarts) :
	Benchmark(comm,
		task,
		new ParentTreeValidator(comm, graph),
		graph,
		numStarts),
	startRoots(generateStartRoots(graph->numGlobalVertex))
	{
		traversedEdges = new vector<double>();
	}

	SearchBenchmark::SearchBenchmark(const SearchBenchmark& orig) :
	Benchmark(orig.comm,
		orig.task,
		new ParentTreeValidator(orig.comm, orig.graph),
		orig.graph,
		orig.numStarts),
	startRoots(generateStartRoots(graph->numGlobalVertex))
	{
		traversedEdges = new vector<double>();
	}

	SearchBenchmark::~SearchBenchmark()
	{
		delete validator;
		delete startRoots;
		delete traversedEdges;
	}

	Vertex SearchBenchmark::generateOutstandingRoot()
	{
		Vertex newRoot;
		if (rank == 0) {
			Random *random = Random::getInstance(comm);
			newRoot = random->next(0, graph->numGlobalVertex);
		}
	}

	Vertex* SearchBenchmark::generateStartRoots(Vertex maxStartRoot)
	{
		log << "\nGenerating roots... ";
		double startTime = Wtime();
		Vertex* startRoots = new Vertex[numStarts];
		Random *random = Random::getInstance(comm);

		if (rank == 0) {
			for (int i = 0; i < numStarts; ++i) {
				startRoots[i] = random->next(0, maxStartRoot);
			}
		}

		comm->Bcast(startRoots, numStarts, VERTEX_TYPE, 0);
		double rootsGenerationTime = Wtime() - startTime;
		log << rootsGenerationTime << " s\n";
		return startRoots;
	}

	bool SearchBenchmark::runSingleTask(int startIndex)
	{
		assert(task->getTaskType() == SEARCH);

		SearchTask *task = (SearchTask*) this->task;
		task->setRoot(startRoots[startIndex]);

		log << "Running search task (" << (startIndex + 1) << "/" << numStarts << ")\n";
		ParentTree *result = task->run();

		int restartCount = 0;
		const int maxRestarts = 5;
		while (result->getTraversedEdges() < 1 && ++restartCount <= maxRestarts) {
			log << "Error running BFS: no edges going from root.\n";
			log << "Restart " << restartCount << "\n";
			delete result;
			task->setRoot(generateOutstandingRoot());
			result = task->run();
		}

		if (result->getTraversedEdges() < 1) {
			log << "Error running BFS: graph is too sparse\n";
		}

		bool isValid = validator->validate(result);

		if (isValid) {
			log << "Traversed edges: " << (size_t) result->getTraversedEdges() << "\n";
			log << "Task mark: " << result->getMark() << " TEPS" << "\n\n";
		}

		traversedEdges->push_back(result->getTraversedEdges());
		taskRunningTimes->push_back(result->getTaskRunTime());
		validationTimes->push_back(validator->getValidationTime());
		marks->push_back(result->getMark());

		delete result;
		return isValid;
	}

	void SearchBenchmark::printBenchmarkStatistics(stringstream &out)
	{
		out << "#\n";
		out << getStatistics(traversedEdges, ".traversedEdges", ios::fixed, 0);

		Benchmark::printBenchmarkStatistics(out);
	}

}

