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

#ifndef BFSGRAPH500_OPTIMIZED_H
#define	BFSGRAPH500_OPTIMIZED_H

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "../SearchTask.h"

namespace dgmark {

    class BFSGraph500Optimized : public SearchTask {
    public:
        BFSGraph500Optimized(Intracomm *comm);
        BFSGraph500Optimized(const BFSGraph500Optimized& orig);
        virtual ~BFSGraph500Optimized();

        virtual ParentTree* run();
        virtual string getName();
    private:
        void run_bfs(Vertex root, int64_t* parent);

    };
}

#endif	/* BFSGRAPH500_OPTIMIZED_H */

