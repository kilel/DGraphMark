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

#ifndef DEPTHBUILDER_H
#define	DEPTHBUILDER_H

#include "../ParentTree.h"

namespace dgmark {

	class DepthBuilder {
	public:

		DepthBuilder(Graph *graph) :
				graph(graph)
		{
			depth = new Vertex[graph->numLocalVertex];
		}

		virtual ~DepthBuilder()
		{
			delete[] depth;
		}

		/**
		 * Builds depth. Do not clean array, it is reusable.
		 * Deletes with deletion of builder.
		 * @param parentTree Source of parent tree.
		 * @return Depths array of verticies. 0 if failed to build array.
		 */
		Vertex* buildDepth(ParentTree *parentTree)
		{
			prepare(parentTree->getRoot());
			parent = parentTree->getParent();

			while (buildState == buildStateNextStepRequired) {
				//printf("%d: step\n\n", rank);
				buildNextStep();
			}

			if (buildState == buildStateError) {
				return 0;
			}

			return depth;
		}

	protected:
		static const int SYNCH_END_TAG = 24067;
		static const int LOCAL_SEND_TAG = 467;
		static const int DEPTH_SEND_TAG = 335;

		Vertex *depth;
		Graph *graph;
		Vertex *parent;

		short buildState;
		static const short buildStateError = 0;
		static const short buildStateNextStepRequired = 1;
		static const short buildStateSuccess = 2;

		virtual void buildNextStep() = 0;

		virtual void prepare(Vertex root)
		{
			buildState = buildStateNextStepRequired;

			for (Vertex localVertex = 0; localVertex < graph->numLocalVertex; ++localVertex) {
				depth[localVertex] = graph->numGlobalVertex;
			}
		}
	private:

	};
}

#endif	/* DEPTHBUILDER_H */

