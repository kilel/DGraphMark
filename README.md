DGraphMark
---------------

Distributed graph benchmark. Inspired by Graph500.

DGraphMark helps to test parallel computer (multicore, cluster - anything, 
that supports MPI) in terms of solving data-intensive tasks.

For now DGraphMark solves only BFS (breadth-first search) task, but it can be 
extensed to solve any task, which can be solved on distibuted graph. 

BFS task was choosen to implement first, because it is a kernel algorithm of 
Graph500 -- famous data-intensive benchmark.

What's difference? Why to use DGraphMark?

1. Different project model. Much more extensible and transparent.
2. Different implementation of algorithms. You are not bound to use only one.
3. Performance risen significantly. Validation of BFS result is almost as quick as BFS running.
4. Lots of build parameters to provide full control.
5. It is possible to run several benchmarks or tasks in one run to create complex benchmark builds.
6. Results and statistics rates of run are saved to file in .properties format. 
It can be read well by both machines and humans!
7. Self-documented well-formatted code.

Tested in:
1. Ubuntu-like Linux, GCC 4.6+, MPICH 3.*+


What is done now:

1. creation and distribution of graph
  1. generation of oriented graph with random (4.1) algorithms: uniform and Kronecker 2x2 (R-MAT);
  2. optional deorientation edges with MPI point-to-point communication;
  3. creating Compressed Sparse Row Graph from initial list of edges;
2. search task (consists of creating a tree in graph, starting from root vertex)
  1. creating root verticies with random (4.1) function;
  2. available BFS tasks:
     1. dgmark_p2p -- MPI P2P with lock;
     2. dgmark_rma -- MPI RMA with fetch locking;
     3. dgmark_p2p_nolock -- MPI P2P without locking. Buffered senders;
     4. Graph500 RMA. Uses bfs_run function from mpi/bfs_onesided.c (v2.1.4). Refactored and optimized a bit;
     5. Graph500 P2P. Uses bfs_run function from mpi/bfs_simple.c (v2.1.4);
  3. BFS result is distributed array of global parents for each local vertex;
  4. validation of built tree:
     1. ranges validation : make sure, parents are in legal values (global vertex or UNREACHED);
     2. parents validation: make sure, that no vertex is parent of itselt (except for root);
     3. depth validation: make sure, that all visited vertices has valid depth;
  5. depth building algorithms (also they check for loops in parent array):
     1. buffered -- buffered sending of visited local vertices depth (if was not sended before), while there is anything to send.
     2. p2p_noblock -- try to retrieve depth of parent for all visited parents, while it is possible.
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
  3. all statistics prints on screen and writes to file "./dgmarkStatistics/dgmark_stat_YYYY-MM-DDThh-mm-ss.properties" in machine-readable format;
4. random numbers generation
  1. implemented simple method of generation. Results of rand() (from cstdib) are concatinated to provide 64 bit random value;
  2. seed is generated separately for each node (based on it's rank);
  3. this method is used, because it provides compability with compilers without c++11 functions. (<random>)

What to be done:

1. test work on several machines with different architectures and realizations of MPI to find problems.
