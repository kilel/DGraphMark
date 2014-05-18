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

#ifndef GRAPH_H
#define	GRAPH_H

#include <vector>
#include "Edge.h"
#include "../mpi/Communicable.h"

namespace dgmark {
	using namespace std;

	/**
	 * Describes an oriented graph.
	 */
	class Graph : public Communicable {
	public:
		vector<Edge*> * const edges;
		const int grade;
		const int density;
		const Vertex numLocalVertex;
		const Vertex numGlobalVertex;

		/**
		 * Creates graph.
		 * @param comm communicator.
		 * @param grade grade of graph (log[2] numGlobalVertices)
		 * @param density density of graph (mean num of edges per vertice)
		 */
		Graph(Intracomm *comm, int grade, int density);

		/**
		 * Copying graph. 
		 * Note, that graph copies reference to original edgelist.
		 * 
		 * @param orig Original graph.
		 */
		Graph(const Graph& orig);

		/**
		 * Copying graph. 
		 * Note, that graph copies reference to original edgelist.
		 * 
		 * @param orig Original graph.
		 */
		Graph(const Graph *orig);

		virtual ~Graph();

		/**
		 * Clears edgelist.
		 */
		virtual void clear();

		/**
		 * Converts global vertex to local one.
		 * 
		 * @param globalVertex Global vertex
		 * @return local vertex
		 */
		inline Vertex vertexToLocal(Vertex globalVertex) const
		{
			return(vertexRank(globalVertex) << diffGrade) ^ globalVertex;
		}

		/**
		 * Converts local vertex to global one. (Assumes, that vertex from current process).
		 * 
		 * @param localVertex local vertex from current process
		 * @return global vertex.
		 */
		inline Vertex vertexToGlobal(Vertex localVertex) const
		{
			return vertexToGlobal(rank, localVertex);
		}

		/**
		 * Converts local vertex to global one.
		 * 
		 * @param rank rank of process.
		 * @param localVertex local vertex of the process.
		 * @return global vertex.
		 */
		inline Vertex vertexToGlobal(int rank, Vertex localVertex) const
		{
			return(rank << diffGrade) | localVertex;
		}

		/**
		 * Returns rank of the global vertex.
		 * @param globalVertex global vertex.
		 * @return rank of the global vertex.
		 */
		inline Vertex vertexRank(Vertex globalVertex) const
		{
			return globalVertex >> diffGrade;
		}

	private:
		int diffGrade;

		void initialize();
	};

	inline static int getLog2(int value)
	{
		int log2value = 0;
		while (value != 1) {
			value >>= 1;
			++log2value;
		}
		return log2value;
	}

}

#endif	/* GRAPH_H */
