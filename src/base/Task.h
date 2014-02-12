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

#ifndef GRAPHTASK_H
#define	GRAPHTASK_H

enum TaskType {
    STUB, PARENT_TREE
};

/**
 * Shows, that object can be classified to some task type.
 */
class Classifieble {
public:

    virtual ~Classifieble() {
    }

    /**
     * @return Task type, in which  this object is classified.
     */
    virtual TaskType getTaskType() = 0;
};

/**
 * Represents result of the task.
 */
class Result : public Classifieble {
public:

    virtual ~Result() {
    }
    
    /**
     * @return performance mark of the result.
     */
    virtual double getMark() = 0;
};

/**
 * Validates result of the task.
 */
class Validator : public Classifieble {
public:

    virtual ~Validator() {
    }

    bool validate(Result *result);
};

/**
 * Represents the task. Parameters should be passed through constructor.
 */
class Task : public Classifieble {
public:

    virtual ~Task() {
    }

    Result* run();
};

#endif	/* GRAPHTASK_H */

