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

#include "../Controller.h"
#include "../../task/search/SearchTask.h"
#include "../../generator/GraphGenerator.h"

namespace dgmark {

	class SearchController : public Controller {
	public:
		SearchController(Intracomm *comm, int argc, char **argv);
		SearchController(const SearchController& orig);
		virtual ~SearchController();

		virtual void run(vector<Task*> *tasks);
		virtual void clean(vector<Task*> *tasks);

	protected:
		virtual string getAdditionalStatistics();
	private:
		GraphGenerator *generator;


	};
}

#endif	/* SEARCHCONTROLLER_H */

