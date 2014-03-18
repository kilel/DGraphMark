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

    protected:
        static const int BFS_SYNCH_TAG = 541;

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

#endif	/* BFSDGMARK_H */

