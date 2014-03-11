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

#ifndef RANDOM_H
#define	RANDOM_H
#include <inttypes.h>

namespace dgmark {

    class Random {
    public:
        Random();
        Random(uint64_t seed);
        Random(const Random& orig);
        virtual ~Random();

        uint64_t next();
        uint64_t next(uint64_t min, uint64_t max);
    private:
        uint64_t seed;

        int typeBitSize;
        int randBitSize;
        void fillRandBitSize();
    };
}

#endif	/* RANDOM_H */
