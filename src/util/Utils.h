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

#ifndef UTILS_H
#define	UTILS_H

namespace dgmark {

	class Utils {
	public:

		static void printGraph(Intracomm *comm, Graph *graph)
		{
			vector<Edge*> *edges = graph->edges;

			for (size_t i = 0; i < edges->size(); ++i) {
				Edge *edge = edges->at(i);
				printf("%d: %ld -> %ld\n", comm->Get_rank(), edge->from, edge->to);
			}
		}
	};

}
#endif	/* UTILS_H */

