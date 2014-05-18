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

#include "SearchTask.h"

namespace dgmark {

	SearchTask::SearchTask(Intracomm *comm) :
	Task(comm),
	log(comm)
	{
	}

	SearchTask::SearchTask(const SearchTask& orig) :
	Task(orig.comm),
	log(orig.comm)
	{
	}

	SearchTask::~SearchTask()
	{
	}

	TaskType SearchTask::getTaskType()
	{
		return SEARCH;
	}

	void SearchTask::open(Graph *newGraph)
	{
		log << "Opening task... ";
		comm->Barrier();
		double startTime = Wtime();

		graph = new CSRGraph(newGraph);

		comm->Barrier();
		taskOpeningTime = Wtime() - startTime;
		log << taskOpeningTime << " s\n";

		numLocalVertex = graph->numLocalVertex;
	}

	void SearchTask::close()
	{
		comm->Barrier();
		log << "Closing task... \n";
		delete graph;
		comm->Barrier();
	}

	double SearchTask::getTaskOpeningTime()
	{
		return taskOpeningTime;
	}

	void SearchTask::setRoot(Vertex newRoot)
	{
		root = newRoot;
	}

}

