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

#include "RandomGenerator.h"

namespace dgmark {

	class KroneckerGenerator : public RandomGenerator {
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
		virtual void generateInternal(Graph *graph);

	private:
		Edge* generateEdge(Graph *graph);

		void moveBinary(Vertex &left, Vertex &right, int dest);
		Vertex revert(Vertex source, int grade);
		void getKroneckerDest(int &fromDest, int &toDest);
	};

	/**
	 * This map represents probability to connect vertices with.
	 * Definition: vertice is strong, if it is in first half of table.
	 * Definition: vertice is weak, if it is in second half of table.
	 * Algorithm to connect is binary search. 
	 * Each time we have to decide, if our vertice is weak or strong, 
	 * and with which kind to connect.
	 */
	static const double kronecker2x2Probability[2][2] = {
		{0.57, 0.19},
		{0.19, 0.05}
	};

	static const double KPStrongToStrong = kronecker2x2Probability[0][0];
	static const double KPStrongToWeak = KPStrongToStrong + kronecker2x2Probability[0][1];
	static const double KPWeakToStrong = KPStrongToWeak + kronecker2x2Probability[1][0];
	static const double KPWeakToWeak = KPStrongToWeak + kronecker2x2Probability[1][1];
}

#endif	/* KRONECKERGENERATOR_H */

