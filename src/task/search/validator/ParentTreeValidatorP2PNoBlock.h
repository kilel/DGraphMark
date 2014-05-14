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

#ifndef PARENTTREEVALIDATORP2PNOBLOCK_H
#define	PARENTTREEVALIDATORP2PNOBLOCK_H

#include "../ParentTreeValidator.h"

namespace dgmark {

    class ParentTreeValidatorP2PNoBlock : public ParentTreeValidator {
    public:
        ParentTreeValidatorP2PNoBlock(Intracomm *comm);
        ParentTreeValidatorP2PNoBlock(const ParentTreeValidatorP2PNoBlock& orig);
        virtual ~ParentTreeValidatorP2PNoBlock();

    protected:
        virtual bool validateDepth(ParentTree *parentTree);

    private:
        Vertex* buildDepth(ParentTree *parentTree);
        Vertex getDepth(Graph *graph, Vertex* depths, Vertex currVertex);
        void synchAction(Vertex* depths);
        
        void waitForOthersToEnd(Vertex *depths);
    };
}
#endif	/* PARENTTREEVALIDATORP2PNOBLOCK_H */

