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

#include "Task.h"
#include "Validator.h"

class StubResult : public Result {
public:

	virtual ~StubResult()
	{
	}

	virtual TaskType getTaskType()
	{
		return STUB;
	}

	virtual double getMark()
	{
		return 0;
	}
};

class StubValidator : public Validator {
public:

	virtual ~StubValidator()
	{
	}

	TaskType getTaskType()
	{
		return STUB;
	}

	bool validate(Result *result)
	{
		return true;
	}
};

class StubTask : public Task {
public:

	virtual ~StubTask()
	{
	}

	TaskType getTaskType()
	{
		return STUB;
	}

	StubResult* run()
	{
		return new StubResult();
	}
};
