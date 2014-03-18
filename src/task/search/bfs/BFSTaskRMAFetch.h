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

#ifndef BFSTASKRMAFETCH_H
#define	BFSTASKRMAFETCH_H

#include "../../../mpi/RMAWindow.h"
#include "BFSdgmark.h"

namespace dgmark {

    class BFSTaskRMAFetch : public BFSdgmark {
    public:
        BFSTaskRMAFetch(Intracomm *comm);
        BFSTaskRMAFetch(const BFSTaskRMAFetch& orig);
        virtual ~BFSTaskRMAFetch();

        virtual ParentTree* run();
        virtual string getName();
    private:
        static const int BFS_SYNCH_TAG = 45782;
        /**
         * Performes one BSF step for all nodes.
         * 
         * @param qWin queue RMA window
         * @param pWin parent RMA window
         * @return true, if more steps are required.
         */
        bool performBFS(RMAWindow<Vertex> *qWin, RMAWindow<Vertex> *pWin);

        /**
         * Performs actual BFS step. 
         * Traverses endges from all queued vertices, and processes childs.
         * 
         * @param qWin queue RMA window
         * @param pWin parent RMA window
         * @return true, if local or global queue is enlarged.
         */
        bool performBFSActualStep(RMAWindow<Vertex> *qWin, RMAWindow<Vertex> *pWin);

        /**
         * Performs RMA synchronization.
         * Purpose is to allow main process in queue to perform actual BFS.
         * @param qWin queue RMA window
         * @param pWin parent RMA window
         */
        void performBFSSynchRMA(RMAWindow<Vertex> *qWin, RMAWindow<Vertex> *pWin);

        /**
         * Processes global child. 
         * Adds it to other nodes queue and sets it's parent, if it was not set.
         * 
         * @param qWin queue RMA window
         * @param pWin parent RMA window
         * @param currVertex current vertex (parent of child) (global notation)
         * @param child Child of current vertex (global notation)
         * @return true, if global queue is enlarged.
         */
        bool processGlobalChild(RMAWindow<Vertex> *qWin, RMAWindow<Vertex> *pWin, Vertex currVertex, Vertex child);
    };
}

#endif	/* BFSTASKRMAFETCH_H */

