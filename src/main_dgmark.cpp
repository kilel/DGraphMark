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

#include "task/search/bfs/BFSGraph500P2P.h"
#include "task/search/bfs/BFSGraph500RMA.h"
#include "task/search/bfs/BFSTaskP2P.h"
#include "task/search/bfs/BFSTaskP2PNoBlock.h"
#include "task/search/bfs/BFSTaskRMAFetch.h"

#include "controller/search/SearchController.h"

using namespace dgmark;

int main(int argc, char** argv)
{
	Init();
	Intracomm *comm = &COMM_WORLD;

	vector<Task*> *tasks = new vector<Task*>();

	#ifdef TASK_TYPE_dgmark_p2p
	tasks->push_back(new BFSTaskP2P(comm));
	#elif TASK_TYPE_dgmark_p2p_noblock
	tasks->push_back(new BFSTaskP2PNoBlock(comm));
	#elif TASK_TYPE_dgmark_rma
	tasks->push_back(new BFSTaskRMAFetch(comm));
	#elif TASK_TYPE_graph500_p2p
	tasks->push_back(new BFSGraph500P2P(comm));
	#elif TASK_TYPE_graph500_rma
	tasks->push_back(new BFSGraph500Optimized(comm));
	#else 
	tasks->push_back(new BFSTaskP2PNoBlock(comm));
	#endif

	SearchController *controller = new SearchController(comm, argc, argv);
	controller->run(tasks);
	controller->clean(tasks);

	Finalize();
	return 0;
}