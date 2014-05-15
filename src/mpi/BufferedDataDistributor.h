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

#ifndef BUFFEREDDATADISTRIBUTOR_H
#define	BUFFEREDDATADISTRIBUTOR_H

#include "Communicable.h"


namespace dgmark {

	class BufferedDataDistributor : public Communicable {
	public:
		BufferedDataDistributor(Intracomm *comm,
			size_t elementSize, size_t bufferedElementsCount);
		virtual ~BufferedDataDistributor();

	protected:
		virtual void processRecvData(size_t countToRead) = 0;

		static const int DISTRIBUTION_TAG = 246;
		static const int END_TAG = 643;

		const size_t sendPackageSize;
		const size_t elementSize;

		Vertex **sendBuffer;
		Vertex *countToSend;
		Vertex *recvBuffer;

		Request *sendRequest;
		Request recvRequest;

		bool *isSendRequestActive;
		bool isRecvRequestActive;

		void sendData(int toRank);
		void probeReadData();

		void prepareBuffers();
		void flushBuffers();
		void updateRequestsActivity();
		void waitForOthersToEnd();
	};
}

#endif	/* BUFFEREDDATADISTRIBUTOR_H */

