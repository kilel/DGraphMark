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
#include "../../../mpi/BufferedDataDistributor.h"

namespace dgmark {

	class BFSTaskP2PNoBlock : public BFSTaskP2P, public BufferedDataDistributor {
	public:
		BFSTaskP2PNoBlock(Intracomm *comm);
		virtual ~BFSTaskP2PNoBlock();

		virtual string getName();

	protected:
		virtual void performBFS();
		virtual void processGlobalChild(Vertex currVertex, Vertex child);

		virtual void processRecvData(size_t countToRead);
	private:
		static const size_t ELEMENT_SIZE = 2;
		static const size_t BUFFERED_ELEMENTS = 256;

	};
}

#endif	/* BFSTASKP2PNOBLOCK_H */

