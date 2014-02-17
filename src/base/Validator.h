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

#ifndef VALIDATOR_H
#define	VALIDATOR_H

#include "Result.h"

namespace dgmark {

    /**
     * Validates result of the task.
     */
    class Validator : public Classifieble, public Communicable {
    public:

        Validator(Intracomm *comm) : Communicable(comm) {
        }

        virtual ~Validator() {
        }

        virtual bool validate(Result *result) = 0;
    };
}

#endif	/* VALIDATOR_H */

