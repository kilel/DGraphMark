DGraphMark
---------------

Distributed graph benchmark. Inspired by Graph500.  

Main goal is to create benchmark, which:  

1. generates random graph (different algorithms);
2. distributes it between nodes with MPI;
3. measures performance of some operations (at least BFS).

What is done now:

1. creation and distribution of graph
  1. generation of graph. Verticies of edges are generated with rand() function;
  2. distributing of graph with MPI point-to-point communication;
  3. creating Compressed Sparse Row Graph from initial list of edges;
2. tree-making task (consists of creating a tree in graph, starting in root vertex)
  1. creating root verticies with rand() function;
  2. BFS task:
     1. DGraphMark BFS. Implementation of BFS algorithm, used MPI one-sided communication;
     2. Graph500 BFS. Uses bfs_run function from mpi/bfs_onesided.c (v 2.1.4);
     3. optimized Graph500 BFS. Same, as 2.1.2, bun refactored and optimized;
  3. BFS result is distributed array of global parents for each local vertex
  4. validation of built tree:
     1. ranges validation : make sure, that all vertices were traversed;
     2. parents validation: make sure, that no vertex is parent of itselt (except for root);
     3. depth build/validation: builds depth for vertices and makes sure, that all vertices has valid depth;
3. generation of statistics:
  1. statistics consists of initial data, duration of some processes and some math statistics about:
     1. bfs duration;
     2. validation duration;
     3. traversed edges count;
     4. mark value;
  2. math statistics parameters:
     1. mean (arithmetic);
     2. standard deviation;
     3. relative standard deviation (to mean);
     4. minimum;
     5. first quartile;
     6. median;
     7. trird quartile;
     8. maximum;
  3. all statistics prints on screen and writes to file "./dgmarkStatistics/dgmark_stat_YYYY-MM-DDThh-mm-ss.properties" in machine-readable format.


What to be done:

1. write random numbers generator to replace rand();
2. write Kronecker graph generator and compare with simple one;
3. test work on several machines with different architectures and realizations of MPI to find problems.
