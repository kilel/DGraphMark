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

#ifndef DEPTHBUILDERBUFFERED_H
#define	DEPTHBUILDERBUFFERED_H

#include "../../../mpi/BufferedDataDistributor.h"
#include "DepthBuilder.h"

namespace dgmark {

	class DepthBuilderBuffered : public BufferedDataDistributor, public DepthBuilder {
	public:
		DepthBuilderBuffered(Intracomm *comm, Graph *graph);
		DepthBuilderBuffered(const DepthBuilderBuffered& orig);
		virtual ~DepthBuilderBuffered();

	protected:
		virtual void processRecvData(size_t countToRead);
		virtual void buildNextStep();
		virtual void prepare(Vertex root);
		
	private:
		static const size_t ELEMENT_SIZE = 2;
		static const size_t BUFFERED_ELEMENTS = 256;
		
		short *vertexState;
		static const short stateInitial = 0;
		static const short stateJustFilled = 1;
		static const short stateSent = 2;

		void distributeVertexDepth(Vertex localVertex);
		void updateDepth(Vertex currParent, Vertex parentDepth);
	};
}

#endif	/* DEPTHBUILDERBUFFERED_H */

