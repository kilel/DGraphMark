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

#ifndef SIMPLEGENERATOR_H
#define	SIMPLEGENERATOR_H

#include "../base/GraphGenerator.h"
#include "../base/Log.h"

namespace dgmark {

    using namespace MPI;

    class SimpleGenerator : public GraphGenerator {
    public:
        /**
         * Creates simple graph generator, distributed in communicator nodes.
         * @param communicator MPI intracomm.
         * @param grade Grade of vertices. Total number is 2^grade
         * @param density Density of edges. Avg amount of edges per vertex.
         */
        SimpleGenerator(Intracomm *comm, int grade, int density);
        virtual ~SimpleGenerator();

        virtual Graph* generate();
        virtual double getGenerationTime();
        
        virtual int getGrade();
        virtual int getDensity();

    private:
        int grade;
        int density;
        double generationTime;
        Log log;
    };
}

#endif	/* SIMPLEGENERATOR_H */

