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
#include "validator/DepthBuilder.h"

namespace dgmark {

	class ParentTreeValidator : public Validator {
	public:
		ParentTreeValidator(Intracomm *comm, Graph *graph);
		ParentTreeValidator(const ParentTreeValidator& orig);
		virtual ~ParentTreeValidator();

		virtual TaskType getTaskType();
		virtual double getValidationTime();
		virtual bool validate(Result *taskResult);

	protected:
		Graph* graph;
		Log log;

		virtual bool validateDepth(ParentTree *parentTree);
		bool doValidateDepth(ParentTree *parentTree, Vertex *depths);
	private:
		double validationTime;
		DepthBuilder *builder;

		const Vertex illegalDepth;
		const Vertex illegalParent;

		bool doValidate(ParentTree *parentTree);
		bool validateRanges(ParentTree *parentTree);
		bool validateParents(ParentTree *parentTree);

		static DepthBuilder* createBuilder(Intracomm *comm, Graph *graph);
	};
}
#endif	/* PARENTTREEVALIDATOR_H */

