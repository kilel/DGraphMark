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

#ifndef BFSGRAPH500P2P_H
#define	BFSGRAPH500P2P_H

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "../SearchTask.h"

namespace dgmark {

	class BFSGraph500P2P : public SearchTask {
	public:
		BFSGraph500P2P(Intracomm *comm);
		BFSGraph500P2P(const BFSGraph500P2P& orig);
		virtual ~BFSGraph500P2P();

		virtual ParentTree* run();
		virtual string getName();

		virtual void open(Graph *newGraph);
		virtual void close();
	private:
		void run_bfs(Vertex root, int64_t* parent);

		int64_t* g_oldq;
		int64_t* g_newq;
		unsigned long* g_visited;
		static const int coalescing_size = 256;
		int64_t* g_outgoing;
		size_t* g_outgoing_counts /* 2x actual count */;
		MPI_Request* g_outgoing_reqs;
		int* g_outgoing_reqs_active;
		int64_t* g_recvbuf;
	};
}

#endif	/* BFSGRAPH500P2P_H */

