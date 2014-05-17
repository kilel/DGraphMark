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

#include "../../task/search/ParentTreeValidator.h"

namespace dgmark {

	SearchBenchmark::SearchBenchmark(Intracomm *comm, SearchTask *task, Graph *graph, int numStarts) :
	Benchmark(comm, task, new ParentTreeValidator(comm, graph), graph, numStarts),
	startRoots(generateStartRoots(graph->numGlobalVertex))
	{
	}

	SearchBenchmark::SearchBenchmark(const SearchBenchmark& orig) :
	Benchmark(orig.comm, orig.task, orig.validator, orig.graph, numStarts),
	startRoots(orig.startRoots)
	{
	}

	SearchBenchmark::~SearchBenchmark()
	{
		delete validator;
		delete startRoots;
	}

	Vertex* SearchBenchmark::generateStartRoots(size_t maxStartRoot)
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
		bool isValid = validator->validate(result);

		if (isValid) {
			log << "Task mark: " << result->getMark() << " TEPS" << "\n\n";
		}

		taskRunningTimes->push_back(result->getTaskRunTime());
		marks->push_back(result->getMark());
		validationTimes->push_back(validator->getValidationTime());

		delete result;
		return isValid;
	}



}

