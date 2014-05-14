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

#ifndef CONTROLLER_H
#define	CONTROLLER_H

#include <string>
#include <vector>
#include "../benchmark/Benchmark.h"

namespace dgmark {

	class Controller : public Communicable {
	public:

		Controller(Intracomm *comm, int argc, char **argv);
		Controller(const Controller& orig);
		virtual ~Controller();

		virtual void run(vector<Task*> *tasks) = 0;
		virtual void clean(vector<Task*> *tasks) = 0;
		void run(vector<Benchmark*> *benchmarks);
		void clean(vector<Benchmark*> *benchmarks);

	protected:
		int grade;
		int density;
		int numStarts;

		Log log;
		static const int CONTROLLER_PRECISION = 5;

		virtual string getAdditionalStatistics() = 0;
	private:
		string getInitialStatistics();
		void parseArguments(int argc, char** argv);
		void printResult(string stat);
	};
}

#endif	/* CONTROLLER_H */

