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

#include "BFSdgmark.h"

namespace dgmark {

    class BFSTaskP2P : public BFSdgmark {
    public:
        BFSTaskP2P(Intracomm *comm);
        BFSTaskP2P(const BFSTaskP2P& orig);
        virtual ~BFSTaskP2P();

        virtual string getName();
        
        virtual void open(Graph *newGraph);
        virtual void close();

    protected:
        virtual bool performBFS();
        virtual bool processGlobalChild(Vertex currVertex, Vertex child);
        virtual bool probeBFSSynch();
        virtual void endActualStepAction();
        
    private:
        void performBFSSynch();
    };
}

#endif	/* BFSTASKP2P_H */

