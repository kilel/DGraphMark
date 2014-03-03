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

#include "ParentTreeValidator.h"
namespace dgmark {

    ParentTreeValidator::ParentTreeValidator(Intracomm *comm) : Validator(comm), log(comm) {
    }

    ParentTreeValidator::ParentTreeValidator(const ParentTreeValidator& orig) :
    Validator(orig.comm), log(orig.comm) {
    }

    ParentTreeValidator::~ParentTreeValidator() {
    }

    TaskType ParentTreeValidator::getTaskType() {
        return TaskType::PARENT_TREE;
    }

    double ParentTreeValidator::getValidationTime() {
        return validationTime;
    }

    bool ParentTreeValidator::validate(Result *taskResult) {
        log << "Validating result... ";
        double startTime = Wtime();

        if (taskResult->getTaskType() != getTaskType()) {
            log << "Error.\nInvalid taskType for result!\n";
            return false;
        }
        ParentTree *parentTreeResult = (ParentTree*) taskResult;
        bool isValid = true;

        //TODO do valudation
        //
        ParentTree* result = (ParentTree*) taskResult;
        result->setTraversedEdges(1);
        //

        validationTime = Wtime() - startTime;
        if (isValid) {
            log << "Sucess\n";
        } else {
            log << "Error\n";
        }
        log << "Validation time: " << validationTime << " s\n";

        return isValid;
    }
}
