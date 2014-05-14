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

#ifndef BFSTASKP2PNOBLOCK_H
#define	BFSTASKP2PNOBLOCK_H

#include "BFSTaskP2P.h"

namespace dgmark {

	class BFSTaskP2PNoBlock : public BFSTaskP2P {
	public:
		BFSTaskP2PNoBlock(Intracomm *comm);
		BFSTaskP2PNoBlock(const BFSTaskP2PNoBlock& orig);
		virtual ~BFSTaskP2PNoBlock();

		virtual string getName();

		//redefune open/close to support creation of buffers.
		virtual void open(Graph *newGraph);
		virtual void close();

	protected:
		virtual bool performBFS();
		virtual bool processLocalChild(Vertex currVertex, Vertex child);
		virtual bool processGlobalChild(Vertex currVertex, Vertex child);
		virtual bool probeBFSSynch();
	private:
		/**
		 * Quantity of elements to send with single package.
		 */
		static const size_t itemsToSendCount = 512;

		/**
		 * Cached requests for all nodes.
		 */
		Request* requests;

		/**
		 * Cached recieve data request.
		 */
		Request recvReq;

		/**
		 * Recv request activity state.
		 * True means, that data is recieved in buffer completely.
		 */
		bool isRecvActive;

		/**
		 * Cached request activity states for nodes. 
		 * True, if request to node is still running.
		 */
		bool* isRequestActive;

		/**
		 * Buffers of data for all nodes. 
		 * Each node buffer contains itemsToSendCount elements.
		 */
		Vertex** sendDataBuffer;

		/**
		 * Count of elements, which are ready to be sent.
		 */
		size_t* sendDataCount;

		/**
		 * Recieve buffer.
		 */
		Vertex* recvBuffer;

		/**
		 * Sends request to specified rank.
		 * @param toRank Reciever node rank.
		 */
		void sendData(int toRank);

		/**
		 * Sets buffers to default values.
		 */
		void prepareBufers();

		/**
		 * Sends all remaining data and assures, that it was recieved by other process.
		 * @return true, if queue was enlarged.
		 */
		bool flushBuffers();

		/**
		 * Tests all active requests to see, if them are completed.
		 */
		void refreshActivityState();

		/**
		 * Waits before all other nodes to enter this section.
		 * @return true, if queue was enlarged.
		 */
		bool waitForOthersToEnd();
	};
}

#endif	/* BFSTASKP2PNOBLOCK_H */

