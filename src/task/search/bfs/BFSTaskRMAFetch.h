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

        virtual string getName();

        virtual void open(Graph *newGraph);
        virtual void close();

    protected:
        RMAWindow<Vertex> *qWin;
        RMAWindow<Vertex> *pWin;

        virtual bool performBFS();
        virtual bool processGlobalChild(Vertex currVertex, Vertex child);
        virtual bool probeBFSSynch();
        virtual void endActualStepAction();

    private:
        /**
         * Performs RMA synchronization.
         * Purpose is to allow main process in queue to perform actual BFS.
         * @param qWin queue RMA window
         * @param pWin parent RMA window
         */
        void performBFSSynchRMA();
    };
}

#endif	/* BFSTASKRMAFETCH_H */

