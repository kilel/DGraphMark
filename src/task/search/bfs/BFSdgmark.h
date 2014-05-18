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

#ifndef BFSDGMARK_H
#define	BFSDGMARK_H

#include "../SearchTask.h"

namespace dgmark {

	class BFSdgmark : public SearchTask {
	public:
		BFSdgmark(Intracomm *comm);
		BFSdgmark(const BFSdgmark& orig);
		virtual ~BFSdgmark();

		virtual ParentTree* run();

	protected:
		static const int BFS_SYNCH_TAG = 541;
		static const int BFS_DATA_TAG = 7353;

		int stepCount;

		/**
		 * queue is a queue of vertex (local).
		 * Traversed vertex adds to the end of the queue.
		 * When BFS performs, it looks on the first vertex (at queue[0] index)
		 * queue[0] is a length
		 * Elements are located in 1..queue[0] indecies.
		 */
		Vertex *queue;

		/**
		 * Queue, prepared for next step.
		 */
		Vertex *nextQueue;

		/**
		 * parent is an array, which associates vertex with it parent (global) in tree.
		 * parent[root] is always must be root.
		 * parent[visited] >= 0 and \<= numGlobalVertex
		 * parent[initially] == numGlobalVertex
		 * Note: contains local vertex only.
		 */
		Vertex *parent;

		/**
		 * Performs BFS step.
		 */
		virtual void performBFS() = 0;

		/**
		 * Performs actual BFS step.
		 */
		virtual void performBFSActualStep();

		/**
		 * Processes local child. 
		 * Adds it to local queue and sets it's parent, if it was not set.
		 * 
		 * @param parentVertexGlobal Current vertex (parent of child) (global notation)
		 * @param childVertexLocal Child of current vertex (local notation)
		 */
		virtual void processLocalChild(Vertex parentVertexGlobal, Vertex childVertexLocal);

		/**
		 * Processes global child. 
		 * Adds it to local queue and sets it's parent, if it was not set.
		 * 
		 * @param currVertex current vertex (parent of child) (global notation)
		 * @param child Child of current vertex (global notation)
		 */
		virtual void processGlobalChild(Vertex currVertex, Vertex child) = 0;

		/**
		 * Swaps queues.
		 * Swaps next and current queues.
		 * 
		 * @param queue queue of local vertices.
		 */
		virtual void swapQueues();

		/**
		 * Resets queues and parent to defult values.
		 */
		void resetQueueParent();

		/**
		 * Function to calculate queue size.
		 * 
		 * @return queue size.
		 */
		virtual Vertex getQueueSize();

		/**
		 * Calculaded need of next BFS step.
		 * @return true, if next step needed.
		 */
		virtual bool isNextStepNeeded();
	};
}

#endif	/* BFSDGMARK_H */

