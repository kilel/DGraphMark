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
        static const int BFS_SYNCH_2_TAG = 13646;
        static const int BFS_END_SYNCH_TAG = 24657;
        static const int BFS_DATA_TAG = 7353;
        static const int BFS_DATA_2_TAG = 924;
        static const int BFS_DATA_3_TAG = 23664;
        static const int BFS_DATA_4_TAG = 7421;

        /**
         * queue is a queue of vertex (local).
         * Traversed vertex adds to the end of the queue.
         * When BFS performs, it looks on the first vertex (at queue[0] index)
         * queue[0] is a start index.
         * queue[1] is an index after the end.
         */
        Vertex *queue;

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
         * @return true, if more steps are needed.
         */
        virtual bool performBFS() = 0;
        
        /**
         * Performs actual BFS step. Probes synchronization by calling probeBFSSynch.
         * @return true, if some queuewas enlarged.
         */
        virtual bool performBFSActualStep();
        
        /**
         * Action, called in end of actual step. Used mostly to end synchronization.
         */
        virtual void endActualStepAction() = 0;
        
        /**
         * Probes BFS synchronization.
         * @return true, if some queuewas enlarged.
         */
        virtual bool probeBFSSynch() = 0;

        /**
         * Processes local child. 
         * Adds it to local queue and sets it's parent, if it was not set.
         * 
         * @param currVertex current vertex (parent of child) (global notation)
         * @param child Child of current vertex (global notation)
         * @return true, if some queue is enlarged.
         */
        bool processLocalChild(Vertex currVertex, Vertex child);

        /**
         * Processes global child. 
         * Adds it to local queue and sets it's parent, if it was not set.
         * 
         * @param currVertex current vertex (parent of child) (global notation)
         * @param child Child of current vertex (global notation)
         * @return true, if some queue is enlarged.
         */
        virtual bool processGlobalChild(Vertex currVertex, Vertex child) = 0;

        /**
         * Aligns queue.
         * Just moves elements to the start of array.
         * 
         * @param queue queue of local vertices.
         */
        void alignQueue();
        
        void resetQueueParent();

        /**
         * Function to calculate queue size.
         * 
         * @return queue size.
         */
        virtual Vertex getQueueSize();
    };
}

#endif	/* BFSDGMARK_H */

