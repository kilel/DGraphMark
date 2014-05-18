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

#ifndef GRAPHDISTRIBUTOR_H
#define	GRAPHDISTRIBUTOR_H

#include "Graph.h"
#include "../mpi/BufferedDataDistributor.h"

namespace dgmark {

	class GraphDistributor : public BufferedDataDistributor {
	public:
		GraphDistributor(Intracomm* comm, Graph *graph);
		virtual ~GraphDistributor();

		/**
		 * Distributes graph. 
		 * Each edge (myRank|localNode -> hisRank|hisLocal) sends to hisRank as
		 * (hisRank|hisLocal -> myRank|localNode).
		 * 
		 * @param graph Graph to distribute.
		 */
		void distribute();

	protected:
		virtual void processRecvData(size_t countToRead);

	private:
		static const size_t ELEMENT_SIZE = 2;
		static const size_t BUFFERED_ELEMENTS = 256;

		const Graph * const graph;
		vector<Edge*> * const edges;

		/**
		 * Distributes edges to nodes.
		 */
		void distributeEdges();

		/**
		 * Transfers edge to it's destination.
		 * @param edge Edge
		 * @param memory allocated memory to send edge.
		 */
		void sendEdge(const Edge * const edge, int toRank);

	};
}

#endif	/* GRAPHDISTRIBUTOR_H */

