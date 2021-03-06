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

#ifndef RANDOMGENERATOR_H
#define	RANDOMGENERATOR_H

#include "GraphGenerator.h"
#include "../graph/GraphDistributor.h"
#include "../util/Random.h"
#include "../util/Log.h"

namespace dgmark {

	class RandomGenerator : public GraphGenerator {
	public:
		RandomGenerator(Intracomm *comm);
		virtual ~RandomGenerator();

		Graph* generate(int grade, int density);
		double getGenerationTime();
		double getDistributionTime();

	protected:
		Random *random;
		Log log;

		/**
		 * For implementing generators.
		 * Here generated graph accorfing to algorithm of generator.
		 * @param graph Graph to generate edges in.
		 */
		virtual void generateInternal(Graph *graph) = 0;

	private:
		double generationTime;
		double distributionTime;

		void doGenerate(Graph* graph);
		void doDistribute(Graph* graph);
	};
}

#endif	/* RANDOMGENERATOR_H */

