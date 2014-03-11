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

#ifndef SEARCHCONTROLLER_H
#define	SEARCHCONTROLLER_H

#include "Controller.h"
#include "../base/Log.h"
#include "../task/search/ParentTreeValidator.h"
#include "../task/search/SearchTask.h"

namespace dgmark {

    class SearchController : public Controller {
    public:
        SearchController(Intracomm *comm, GraphGenerator *generator, SearchTask *task, int numStarts);
        SearchController(const SearchController& orig);
        virtual ~SearchController();

        virtual void runBenchmark();
        virtual string getStatistics();
        virtual void printStatistics();


    private:
        static const int CONTROLLER_PRECISION = 5;

        Log log;
        SearchTask *task;
        Validator *validator;
        int numStarts;

        Vertex* generateStartRoots(size_t maxStartRoot);

        bool isLastRunValid;

        //statistics data
        double generationTime;
        double distributionTime;
        double taskOpeningTime;
        double rootsGenerationTime;

        vector<double> *taskRunningTimes;
        vector<double> *traversedEdges;
        vector<double> *validationTimes;
        vector<double> *marks;

        string getStatistics(vector<double> *data, string name, const ios::fmtflags floatfieldFlag = ios::scientific);

    };
}

#endif	/* SEARCHCONTROLLER_H */

