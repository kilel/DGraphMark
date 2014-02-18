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

#ifndef PARENTTREEVALIDATOR_H
#define	PARENTTREEVALIDATOR_H

#include "../../base/Validator.h"
#include "../../base/Log.h"
#include "ParentTree.h"

namespace dgmark {

    class ParentTreeValidator : public Validator {
    public:
        ParentTreeValidator(Intracomm *comm);
        ParentTreeValidator(const ParentTreeValidator& orig);
        virtual ~ParentTreeValidator();

        virtual TaskType getTaskType();
        virtual double getValidationTime();
        virtual bool validate(Result *taskResult);

    private:
        Log log;
        double validationTime;
    };
}
#endif	/* PARENTTREEVALIDATOR_H */

