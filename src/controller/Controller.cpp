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


#include <sstream>
#include <fstream>
#include <cstdlib>
#include "Controller.h"

namespace dgmark {

	Controller::Controller(Intracomm *comm, int argc, char **argv) :
	Communicable(comm),
	log(comm)
	{
		parseArguments(argc, argv);
	}

	Controller::Controller(const Controller& orig) :
	Communicable(orig.comm),
	log(orig.comm),
	grade(orig.grade),
	density(orig.density),
	numStarts(orig.numStarts)
	{
	}

	Controller::~Controller()
	{
	}

	void Controller::run(vector<Benchmark*> *benchmarks)
	{
		string statistics = getInitialStatistics() + getSpecificStatistics();

		for (size_t bmark = 0; bmark < benchmarks->size(); ++bmark) {
			Benchmark *benchmark = benchmarks->at(bmark);
			benchmark->run();
			statistics += benchmark->getStatistics();
		}

		printResult(statistics);
	}

	void Controller::clean(vector<Benchmark*> *benchmarks)
	{
		for (size_t bmIndex = 0; bmIndex < benchmarks->size(); ++bmIndex) {
			delete (benchmarks->at(bmIndex));
		}
		delete benchmarks;
	}

	string Controller::getInitialStatistics()
	{
		stringstream out;
		out.precision(CONTROLLER_PRECISION);
		out.setf(ios::fixed, ios::floatfield);

		out << "#\n";
		out << "#Statistics\n";
		out << "#\n";
		out << "#Initial data\n";
		out << "initial.grade = " << grade << "\n";
		out << "initial.edgeDensity = " << density << "\n";
		out << "initial.mpiNodes = " << comm->Get_size() << "\n";
		out << "initial.numStarts = " << numStarts << "\n";

		return out.str();
	}

	void Controller::printResult(string stat)
	{
		if (rank != 0) {
			return;
		}

		log << stat;

		int mkdirResult = system("mkdir -p dgmarkStatistics");
		if (mkdirResult) { //if can't create file
			log << "\nCan't create statistics file. "
				<< "Mkdir error code " << mkdirResult << "\n";
			return;
		}

		time_t datetime = time(0);
		tm *date = localtime(&datetime);

		stringstream fileName;
		fileName << "dgmarkStatistics/dgmark_stat_"
			<< "g" << grade << "_d" << density << "_"
			<< (date->tm_year + 1900) << "-"
			<< (date->tm_mon + 1) << "-"
			<< date->tm_mday << "_"
			<< date->tm_hour << "-"
			<< date->tm_min << "-"
			<< date->tm_sec
			<< ".properties";

		ofstream fileOut;
		fileOut.open(fileName.str().c_str());
		fileOut << stat;
		fileOut.close();
	}

	void Controller::parseArguments(int argc, char** argv)
	{
		if (rank == 0) {
			grade = 8;
			density = 16;
			numStarts = 32;

			if (argc >= 2) {
				grade = atoi(argv[1]);
			}
			if (argc >= 3) {
				density = atoi(argv[2]);
			}
			if (argc >= 4) {
				numStarts = atoi(argv[3]);
			}
		}

		comm->Bcast(&grade, 1, INT, 0);
		comm->Bcast(&density, 1, INT, 0);
		comm->Bcast(&numStarts, 1, INT, 0);
	}
}

