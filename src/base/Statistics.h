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

#ifndef STATISTICS_H
#define	STATISTICS_H

#include <vector>

namespace dgmark {
    using namespace std;

    class Statistics {
    public:
        Statistics(vector<double>* data);
        Statistics(const Statistics& orig);
        virtual ~Statistics();

    private:
        double mean;
        double stdDeviation;
        double relStdDeviation;

        double minimum;
        double firstQuartile;
        double median;
        double thirdQuartile;
        double maximum;

    public: //getters

        double getMean() {
            return mean;
        }

        double getStdDeviation() {
            return stdDeviation;
        }

        double getRelStdDeviation() {
            return relStdDeviation;
        }

        double getMinimum() {
            return minimum;
        }

        double getFirstQuartile() {
            return firstQuartile;
        }

        double getMedian() {
            return median;
        }

        double getThirdQuartile() {
            return thirdQuartile;
        }

        double getMaximum() {
            return maximum;
        }
    };
}

#endif	/* STATISTICS_H */

