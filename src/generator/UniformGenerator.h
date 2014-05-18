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

#ifndef UNIFORMGENERATOR_H
#define	UNIFORMGENERATOR_H

#include "RandomGenerator.h"

namespace dgmark {

	class UniformGenerator : public RandomGenerator {
	public:
		UniformGenerator(Intracomm *comm);
		virtual ~UniformGenerator();

	protected:

		void generateInternal(Graph *graph);

		/**
		 * Adds edges evenly with equal probabilities.
		 * @param graph Graph to add in.
		 * @param localVertex local vertex to start from.
		 * @param numEdges num edges to add from vertex.
		 */
		virtual void addEdgeFromVertex(Graph *graph, Vertex localVertex, size_t numEdges);
	};
}

#endif	/* UNIFORMGENERATOR_H */

