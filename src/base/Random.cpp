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

#include "Random.h"
#include <ctime>
#include <stdlib.h>
#include <stdint.h>
#include <cstdio>


namespace dgmark {

    Random::Random() : typeBitSize(64) {
        seed = time(0);
        srand(rand() + seed);
        fillRandBitSize();
    }

    Random::Random(uint64_t newSeed) : typeBitSize(64) {
        seed = time(0) + newSeed;
        srand(time(0) + newSeed);
        srand(rand() + newSeed);
        fillRandBitSize();
    }

    Random::Random(const Random& orig) : seed(orig.seed), typeBitSize(orig.typeBitSize) {
        fillRandBitSize();
    }

    Random::~Random() {
    }

    void Random::fillRandBitSize() {
        uint64_t randMax = RAND_MAX;
        randBitSize = 0;
        while (randMax > 0) {
            randMax >>= 1;
            randBitSize += 1;
        }
    }

    uint64_t Random::next() {
        uint64_t randomNumber = 0;
        int typeBitSize = this->typeBitSize;

        while (typeBitSize / randBitSize > 0) {
            randomNumber = randomNumber << randBitSize | rand();
            typeBitSize -= randBitSize;
        }

        randomNumber = randomNumber << typeBitSize | (rand() & (1 << typeBitSize - 1));
        return randomNumber;
    }

    uint64_t Random::next(uint64_t min, uint64_t max) {
        if (min > max) {
            uint64_t temp = max;
            max = min;
            min = temp;
        }

        uint64_t randomNumber = next();
        randomNumber = min + randomNumber % (max - min);
        return randomNumber;
    }
}

