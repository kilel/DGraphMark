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

		/**
		 * Creates benchmarks for tasks and runs them.
		 * @param tasks Tasks.
		 */
		virtual void run(vector<Task*> *tasks) = 0;

		/**
		 * Cleans tasks array.
		 * @param tasks Tasks.
		 */
		virtual void clean(vector<Task*> *tasks) = 0;

		/**
		 * Runs benchmarks.
		 * @param benchmarks Bencharks.
		 */
		void run(vector<Benchmark*> *benchmarks);

		/**
		 * Cleans benchmarks array.
		 * @param benchmarks Bencharks.
		 */
		void clean(vector<Benchmark*> *benchmarks);

	protected:
		int grade;
		int density;
		int numStarts;

		Log log;
		static const int CONTROLLER_PRECISION = 5;

		/**
		 * Returns task-specific statistics.
		 * @return Statistics string.
		 */
		virtual string getSpecificStatistics() = 0;
	private:
		string getInitialStatistics();
		void parseArguments(int argc, char** argv);
		void printResult(string stat);
	};
}

#endif	/* CONTROLLER_H */

