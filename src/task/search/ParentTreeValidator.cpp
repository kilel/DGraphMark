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
#include "validator/DepthBuilderBuffered.h"
#include "validator/DepthBuilderP2PNoBlock.h"

namespace dgmark {

	DepthBuilder* ParentTreeValidator::createBuilder(Intracomm *comm,
							Graph *graph)
	{
		DepthBuilder *builder;
		//builder = new DepthBuilderBuffered(comm, graph);
		builder = new DepthBuilderP2PNoBlock(comm, graph);
		return builder;
	}

	ParentTreeValidator::ParentTreeValidator(Intracomm *comm,
						Graph *graph) :
	Validator(comm),
	log(comm),
	graph(graph),
	builder(createBuilder(comm, graph)),
	illegalDepth(graph->numGlobalVertex),
	illegalParent(graph->numGlobalVertex)
	{

	}

	ParentTreeValidator::ParentTreeValidator(const ParentTreeValidator& orig) :
	Validator(orig.comm),
	log(orig.comm),
	graph(orig.graph),
	illegalDepth(orig.illegalDepth),
	illegalParent(orig.illegalParent)
	{
	}

	ParentTreeValidator::~ParentTreeValidator()
	{
		delete builder;
	}

	TaskType ParentTreeValidator::getTaskType()
	{
		return SEARCH;
	}

	double ParentTreeValidator::getValidationTime()
	{
		return validationTime;
	}

	bool ParentTreeValidator::validate(Result *taskResult)
	{
		log << "Validating result... ";
		double startTime = Wtime();

		if (taskResult->getTaskType() != getTaskType()) {
			log << "Error.\nInvalid taskType for result!\n";
			return false;
		}

		ParentTree *parentTree = (ParentTree*) taskResult;
		bool isValid = doValidate(parentTree);

		validationTime = Wtime() - startTime;
		if (isValid) {
			log << "Sucess\n";
		}
		log << "Validation time: " << validationTime << " s\n";

		return isValid;
	}

	bool ParentTreeValidator::doValidate(ParentTree *parentTree)
	{
		if (!validateRanges(parentTree)) {
			return false;
		}

		if (!validateParents(parentTree)) {
			return false;
		}

		if (!validateDepth(parentTree)) {
			return false;
		}

		return true;
	}

	bool ParentTreeValidator::validateRanges(ParentTree *parentTree)
	{
		bool isValid = true;
		const Vertex *parent = parentTree->getParent();

		for (Vertex localVertex = 0; localVertex < graph->numLocalVertex; ++localVertex) {
			isValid &= 0 <= parent[localVertex]
				&& parent[localVertex] <= illegalParent;
		}

		comm->Allreduce(IN_PLACE, &isValid, 1, BOOL, LAND);

		if (!isValid) {
			log << "\nError validating: illegal value ranges of parent tree\n";
		}

		return isValid;
	}

	bool ParentTreeValidator::validateParents(ParentTree *parentTree)
	{
		bool isValid = true;
		const Vertex *parent = parentTree->getParent();
		const Vertex root = parentTree->getRoot();
		const Vertex rootLocal = graph->vertexToLocal(root);

		if (graph->vertexRank(root) == rank) {
			if (root != parent[rootLocal]) {
				isValid = false;
				log << "\nError validating: root parent is not root\n";
			}
		}

		if (isValid) {
			for (Vertex localVertex = 0; localVertex < graph->numLocalVertex; ++localVertex) {
				isValid &= parent[localVertex] != graph->vertexToGlobal(localVertex)
					|| localVertex == rootLocal;
			}
		}

		comm->Allreduce(IN_PLACE, &isValid, 1, BOOL, LAND);

		if (!isValid) {
			log << "\nError validating: some vertices are self parents, or root is not a parent of itself\n";
		}

		return isValid;
	}

	bool ParentTreeValidator::validateDepth(ParentTree *parentTree)
	{
		//no need to clean, array cached in depth builder.
		Vertex *depth = builder->buildDepth(parentTree);

		if (depth == 0) {
			log << "\nError: cycle detected in result\n";
			return false;
		}

		bool isValid = true;

		const Vertex *parent = parentTree->getParent();

		//Depth must be built, when vertex was visited, and not, if it was not;
		//Depth is a value from [0, illegalDepth].
		//Valid value is in [0, illegalDepth)

		for (size_t localVertex = 0; localVertex < graph->numLocalVertex; ++localVertex) {
			const Vertex currDepth = depth[localVertex];
			isValid &= !(parent[localVertex] < illegalParent
				^ currDepth < illegalDepth);
			isValid &= 0 <= currDepth && currDepth <= illegalDepth;
		}

		comm->Allreduce(IN_PLACE, &isValid, 1, BOOL, LAND);

		if (!isValid) {
			log << "\nError: depths builded not for all visited verticies (or for some of unvisited)\n";
		}

		return isValid;
	}
}
