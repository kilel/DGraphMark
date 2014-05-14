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

namespace dgmark {

	class GraphDistributor : Communicable {
	public:
		GraphDistributor(Intracomm* comm);
		virtual ~GraphDistributor();

		/**
		 * Distributes graph. 
		 * Each edge (myRank|localNode -> hisRank|hisLocal) sends to hisRank as
		 * (hisRank|hisLocal -> myRank|localNode).
		 * 
		 * @param graph Graph to distribute.
		 */
		void distribute(Graph *graph);
	private:
		static const int DISTRIBUTION_TAG = 246;
		static const int END_TAG = 643;
		static const size_t sendPackageSize = 512;
		Vertex **sendBuffer;
		Vertex *countToSend;
		Vertex *recvBuffer;
		
		Request *sendRequest;
		Request recvRequest;
		
		bool *isSendRequestActive;
		bool isRecvRequestActive;

		/**
		 * Transfers edge to it's destination.
		 * @param edge Edge
		 * @param memory allocated memory to send edge.
		 */
		void sendEdge(Edge *edge, int toRank, vector<Edge*> *edges);
		void sendEdges(int toRank, vector<Edge*> *edges);

		/**
		 * Tries to read edge from communicator.
		 * @param memory allocated memory to read edge.
		 * @return true, if edge is readed, false if there are nothig to read.
		 */
		bool probeReadEdge(vector<Edge*> *edges);
		
		void prepareBuffers();
		void flushBuffers(vector<Edge*> *edges);
		void updateRequestsActivity();
		void waitForOthersToEnd(vector<Edge*> *edges);

	};
}

#endif	/* GRAPHDISTRIBUTOR_H */

