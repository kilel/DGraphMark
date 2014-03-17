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

#include "../../util/Log.h"
#include "../Validator.h"
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

    protected:
        static const int VALIDATOR_SYNCH_TAG = 28952;
        static const int VALIDATOR_SYNCH_END_TAG = 5794;
        static const int VALIDATOR_LOCAL_SEND_TAG = 35738;
        static const int VALIDATOR_DEPTH_SEND_TAG = 35832;
        virtual bool validateDepth(ParentTree *parentTree) = 0;

    private:
        Log log;
        double validationTime;

        bool doValidate(ParentTree *parentTree);
        bool validateRanges(ParentTree *parentTree);
        bool validateParents(ParentTree *parentTree);

    };
}
#endif	/* PARENTTREEVALIDATOR_H */

