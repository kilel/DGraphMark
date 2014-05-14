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

#ifndef KRONECKERGENERATOR_H
#define	KRONECKERGENERATOR_H

#include "GraphGenerator.h"
#include "SimpleGenerator.h"


namespace dgmark {

	class KroneckerGenerator : SimpleGenerator {
	public:
		KroneckerGenerator(Intracomm *comm);
		virtual ~KroneckerGenerator();
	protected:

		/**
		 * Adds edges with kronecker algorithm 2x2. Equals to R-MAT.
		 * @param graph Graph to add in.
		 * @param localVertex local vertex to start from.
		 * @param numEdges num edges to add from vertex.
		 */
		virtual void addEdgeFromVertex(Graph *graph, Vertex localVertex, size_t numEdges);
	};
}

#endif	/* KRONECKERGENERATOR_H */

