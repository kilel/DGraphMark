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

#ifndef RMAWINDOW_H
#define	RMAWINDOW_H

#include "Communicable.h"

namespace dgmark {

	template<class T>
	class RMAWindow : public Communicable {
	public:
		RMAWindow(Intracomm *comm, size_t size, Datatype dataType);
		RMAWindow(const RMAWindow& orig);
		virtual ~RMAWindow();

		T* getData();
		size_t getDataSize();

		/**
		 * Cleans data, stored in window.
		 */
		void clean();

		/**
		 * Opens fence synchronization
		 * 
		 * @param assertType: MODE_NOPUT if no data putted. Can be 0, if no assertation used.
		 */
		void fenceOpen(int assertType);

		/**
		 * Opens fence synchronization
		 * 
		 * @param assertType: MODE_NOSTORE if no data was stored (get method). Can be 0, if no assertation used.
		 */
		void fenceClose(int assertType);

		/**
		 * Retrieves data wrom window. Be sure, thar targerank != this node rank.
		 * 
		 * @param dataToGet pointer to allocated place to store data.
		 * @param dataLength length of data.
		 * @param targetRank rank of node, where data lies.
		 * @param shift shift in target's data.
		 */
		void get(T* dataToGet, size_t dataLength, int targetRank, size_t shift);

		/**
		 * Puts data in target's storage. Be sure, thar targerank != this node rank.
		 * 
		 * @param dataToPut pointer to allocated place of data to put.
		 * @param dataLength length of data.
		 * @param targetRank rank of node, where data lies.
		 * @param shift shift in target's data.
		 */
		void put(T* dataToPut, size_t dataLength, int targetRank, size_t shift);

		/**
		 * Accumulates data in target's storege. Be sure, thar targerank != this node rank.
		 * @param dataToAcc pointer to allocated place of data to accumulate.
		 * @param dataLength length of data.
		 * @param targetRank rank of node, where data lies.
		 * @param shift shift in target's data.
		 * @param operation operation to accumulate with
		 */
		void accumulate(T* dataToAcc, size_t dataLength, int targetRank, size_t shift, const Op &operation);

	private:
		T *data; //don't freed here
		const size_t dataSize;
		const Datatype dataType;
		Win *win; //freed in ~RMAWindow
	};
}

#endif	/* RMAWINDOW_H */

