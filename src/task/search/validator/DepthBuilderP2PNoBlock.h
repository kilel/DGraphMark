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

#ifndef DEPTHBUILDERP2PNOBLOCK_H
#define	DEPTHBUILDERP2PNOBLOCK_H

#include "../../../mpi/Communicable.h"
#include "DepthBuilder.h"

namespace dgmark {

	class DepthBuilderP2PNoBlock : public Communicable, public DepthBuilder {
	public:
		DepthBuilderP2PNoBlock(Intracomm *comm, Graph *graph);
		virtual ~DepthBuilderP2PNoBlock();

	protected:
		virtual void buildNextStep();
		virtual void prepare(Vertex root);

	private:
		Vertex getDepth(Vertex currVertex);
		void synchAction();

		void waitForOthersToEnd();
	};
}

#endif	/* DEPTHBUILDERP2PNOBLOCK_H */

