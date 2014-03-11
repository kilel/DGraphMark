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

#ifndef GRAPHGENERATOR_H
#define	GRAPHGENERATOR_H

#include "../base/Communicable.h"
#include "../base/Graph.h"
#include "../base/Random.h"

namespace dgmark {

    /**
     * Generates Graphs.<br />
     * Parameters for generation passes with constructor.
     */
    class GraphGenerator : public Communicable {
    public:

        GraphGenerator(Intracomm *comm) : Communicable(comm) {
        }

        virtual ~GraphGenerator() {
        }

        /**
         * Generates graph with specified parameters.
         * @return Graph.
         */
        virtual Graph* generate() = 0;
        virtual double getGenerationTime() = 0;
        virtual double getDistributionTime() = 0;
        
        virtual int getGrade() = 0;
        virtual int getDensity() = 0;
        
        virtual Random* getRandom() = 0;
    };

}

#endif	/* GRAPHGENERATOR_H */

