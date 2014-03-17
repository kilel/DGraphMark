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

#ifndef BFSTASKP2P_H
#define	BFSTASKP2P_H

#include "../SearchTask.h"

namespace dgmark {

    class BFSTaskP2P : public SearchTask {
    public:
        BFSTaskP2P(Intracomm *comm);
        BFSTaskP2P(const BFSTaskP2P& orig);
        virtual ~BFSTaskP2P();

        virtual ParentTree* run();
        virtual string getName();
    private:
        static const int BFS_SYNCH_TAG = 541;
        /**
         * Performes one BSF step for all nodes.
         * 
         * @param queue queue array
         * @param parent parent array
         * @return true, if more steps are required.
         */
        bool performBFS(Vertex *queue, Vertex *parent);

        /**
         * Performs actual BFS step. 
         * Traverses endges from all queued vertices, and processes childs.
         * 
         * @param queue queue array
         * @param parent parent array
         * @return true, if local or global queue is enlarged.
         */
        bool performBFSActualStep(Vertex *queue, Vertex *parent);

        /**
         * Performs RMA synchronization.
         * Purpose is to allow main process in queue to perform actual BFS.
         * 
         * @param queue queue array
         * @param parent parent array
         */
        void performBFSSynchRMA(Vertex *queue, Vertex *parent);

        /**
         * Processes local child. 
         * Adds it to local queue and sets it's parent, if it was not set.
         * 
         * @param queue queue array
         * @param parent parent array
         * @param currVertex current vertex (parent of child) (global notation)
         * @param child Child of current vertex (global notation)
         * @return true, if local queue is enlarged.
         */
        bool processLocalChild(Vertex *queue, Vertex *parent, Vertex currVertex, Vertex child);

        /**
         * Processes global child. 
         * Adds it to other nodes queue and sets it's parent, if it was not set.
         * 
         * @param queue queue array
         * @param parent parent array
         * @param currVertex current vertex (parent of child) (global notation)
         * @param child Child of current vertex (global notation)
         * @return true, if global queue is enlarged.
         */
        bool processGlobalChild(Vertex *queue, Vertex *parent, Vertex currVertex, Vertex child);

        /**
         * Aligns queue.
         * Just moves elements to the start of array.
         * 
         * @param queue queue of local vertices.
         */
        void alignQueue(Vertex *queue);

        /**
         * Function to calculate queue size.
         * 
         * @return queue size.
         */
        Vertex getQueueSize();
    };
}

#endif	/* BFSTASKP2P_H */

