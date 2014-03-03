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

#ifndef TREEMAKERCONTROLLER_H
#define	TREEMAKERCONTROLLER_H

#include "../base/Controller.h"
#include "../base/Log.h"
#include "../tasks/tree-maker/ParentTreeValidator.h"
#include "../tasks/tree-maker/TreeMakerTask.h"

namespace dgmark {

    class TreeMakerController : public Controller {
    public:
        TreeMakerController(Intracomm *comm, GraphGenerator *generator, TreeMakerTask *task, int numStarts);
        TreeMakerController(const TreeMakerController& orig);
        virtual ~TreeMakerController();

        virtual void runBenchmark();
        virtual string getStatistics();
        virtual void printStatistics();


    private:
        const int CONTROLLER_PRECISION = 5;

        Log log;
        TreeMakerTask *task;
        Validator *validator;
        int numStarts;

        Vertex* generateStartRoots();

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

        string getStatistics(vector<double> *data, string name);

    };
}

#endif	/* TREEMAKERCONTROLLER_H */

