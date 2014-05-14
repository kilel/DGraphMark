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

#ifndef LOG_H
#define	LOG_H

#include <cstdio>
#include <iostream>
#include <string>
#include "../mpi/Communicable.h"
#include "../graph/Edge.h"

namespace dgmark {
	using namespace std;

	class Log : public Communicable {
	public:

		Log() : Communicable(0)
		{
		}

		Log(Intracomm *comm) : Communicable(comm)
		{
		}

		Log(const Log& orig) : Communicable(orig.comm)
		{
		}

		virtual ~Log()
		{
		}

		Log& operator<<(string data)
		{
			if (rank == 0)
				printf("%s", data.c_str()); //cout << data;
			return *this;
		}

		Log& operator<<(char* data)
		{
			if (rank == 0)
				printf("%s", data); //cout << data;
			return *this;
		}

		Log& operator<<(const char data[])
		{
			if (rank == 0)
				printf("%s", &data[0]); //cout << data;
			return *this;
		}

		Log& operator<<(Vertex data)
		{
			if (rank == 0)
				printf("%ld", data); //cout << data;
			return *this;
		}

		Log& operator<<(int data)
		{
			if (rank == 0)
				printf("%d", data); //cout << data;
			return *this;
		}

		Log& operator<<(double data)
		{
			if (rank == 0)
				printf("%.5lf", data); //cout << data;
			return *this;
		}

	private:
	};
}

#endif	/* LOG_H */

