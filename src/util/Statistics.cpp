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

#include "Statistics.h"
#include <cmath>
#include <algorithm>

namespace dgmark {

    Statistics::Statistics(vector<double>* data) {
        size_t dataSize = data->size();

        mean = 0;
        for (vector<double>::iterator it = data->begin(); it < data->end(); ++it) {
            mean += *it;
        }
        mean /= dataSize;

        stdDeviation = 0;
        for (vector<double>::iterator it = data->begin(); it < data->end(); ++it) {
            stdDeviation += pow(*it - mean, 2.);
        }
        stdDeviation = sqrt(stdDeviation / (dataSize - 1));

        relStdDeviation = stdDeviation / mean;

        vector<double> *sortedData = new vector<double>();
        sortedData->insert(sortedData->begin(), data->begin(), data->end());

        sort(sortedData->begin(), sortedData->end());


        size_t quartile = dataSize / 4 < 1 ? 1 : dataSize / 4;
        minimum = sortedData->front();
        firstQuartile = sortedData->at(dataSize / 4);
        median = sortedData->at(dataSize / 2);
        thirdQuartile = sortedData->at(dataSize - quartile);
        maximum = sortedData->back();

        delete sortedData;
    }

    Statistics::Statistics(const Statistics& orig) : mean(orig.mean),
    stdDeviation(orig.stdDeviation), relStdDeviation(orig.relStdDeviation),
    minimum(orig.minimum), firstQuartile(orig.firstQuartile), median(orig.median),
    thirdQuartile(orig.thirdQuartile), maximum(orig.maximum) {
    }

    Statistics::~Statistics() {
    }

}
