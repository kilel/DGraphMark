#   Copyright 2014 Kislitsyn Ilya
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.


# true of false to use or not of OpenMP in compilation
OPENMP = false

#true to enable building of graph500 bfs task runners
BUILD_GRAPH500_BFS = true

#true to enable graph distribution
GRAPH_DISTRIBUTION = false

#Type of used generator. Use KRONECKER or UNIFORM.
#Illegal type will produce failure on start.
GRAPH_GENERATOR_TYPE = UNIFORM

#Type of depth builder used in validator. Use BUFFERED of P2PNOBLOCK.
#BUFFERED is faster on small amount of local vertex ( ~2^10 is limit).
#P2PNOBLOCK is stable builder, for other uses.
VALIDATOR_DEPTH_BUILDER_TYPE = BUFFERED

#compile flags
OPENMP_FLAG = -fopenmp
MPICPP = mpic++
CPPFLAGS = -Ofast -std=c++98 #-std=c++11
CPPFLAGS += -DGENERATOR_TYPE_$(GRAPH_GENERATOR_TYPE)
CPPFLAGS += -DDEPTH_BUILDER_TYPE_$(VALIDATOR_DEPTH_BUILDER_TYPE)

ifeq ($(OPENMP), true)
	CPPFLAGS += $(OPENMP_FLAG)
endif

ifeq ($(GRAPH_DISTRIBUTION), true)
	CPPFLAGS += -DGRAPH_DISTRIBUTION
endif

#directories definition
SRC_DIR = src/
BIN_DIR = bin/
OBJ_DIR = $(BIN_DIR)obj/
BENCHMARK_DIR = benchmark/
CONTROLLER_DIR = controller/
GENERATOR_DIR = generator/
GRAPH_DIR = graph/
MPI_DIR = mpi/
TASK_DIR = task/search/
BFS_DIR = $(TASK_DIR)bfs/
VALIDATOR_DIR = $(TASK_DIR)validator/
UTIL_DIR = util/

#definition of path to object directories
OBJ_DIR_PATHS = $(addprefix $(OBJ_DIR), \
		$(BENCHMARK_DIR) $(BENCHMARK_DIR)search/ \
		$(CONTROLLER_DIR) $(CONTROLLER_DIR)search/ \
		$(GENERATOR_DIR) $(GRAPH_DIR) $(MPI_DIR) \
		$(TASK_DIR) $(BFS_DIR) $(VALIDATOR_DIR) $(UTIL_DIR) )

#Definitions of sources to compile
BENCHMARK = Benchmark search/SearchBenchmark
CONTROLLER = Controller search/SearchController
GENERATOR = RandomGenerator UniformGenerator KroneckerGenerator
GRAPH = Graph CSRGraph GraphDistributor
MPI = Communicable RMAWindow BufferedDataDistributor
TASK = ParentTree ParentTreeValidator SearchTask
BFS = BFSdgmark BFSGraph500P2P BFSGraph500RMA BFSTaskRMAFetch BFSTaskP2P BFSTaskP2PNoBlock
VALIDATOR = DepthBuilder DepthBuilderBuffered DepthBuilderP2PNoBlock
UTIL = Statistics Random

#separated, because if use all, it is too long, error occurred in build.
FILES_LIST = $(addprefix $(BENCHMARK_DIR), $(BENCHMARK)) \
	    $(addprefix $(CONTROLLER_DIR), $(CONTROLLER)) \
	    $(addprefix $(GENERATOR_DIR), $(GENERATOR)) \
	    $(addprefix $(GRAPH_DIR), $(GRAPH)) \
	    $(addprefix $(MPI_DIR), $(MPI))
FILES_LIST += $(addprefix $(TASK_DIR), $(TASK)) \
	    $(addprefix $(BFS_DIR), $(BFS)) \
	    $(addprefix $(VALIDATOR_DIR), $(VALIDATOR)) \
	    $(addprefix $(UTIL_DIR), $(UTIL)) \
	    main_dgmark

#full sources and objects paths
SOURCES = $(addprefix $(SRC_DIR), $(addsuffix .cpp, $(FILES_LIST)))
OBJECTS = $(addprefix $(OBJ_DIR), $(addsuffix .o, $(FILES_LIST)))

#build targets
BUILD = dgmark dgmark_p2p dgmark_p2p_noblock dgmark_rma

ifeq ($(BUILD_GRAPH500_BFS), true)
	BUILD += graph500_p2p graph500_rma
endif

#build rules
.PHONY : all
all: $(BUILD)

# prepairing directories.
$(OBJ_DIR_PATHS):					    
	mkdir -p $@

# extended builds
dgmark dgmark_% graph500% : $(OBJ_DIR_PATHS) $(OBJECTS)
	rm -f $(OBJ_DIR)main_dgmark.o;
	$(MPICPP) $(CPPFLAGS) -DTASK_TYPE_$@ -c $(SRC_DIR)main_dgmark.cpp -o $(OBJ_DIR)main_dgmark.o
	$(MPICPP) $(CPPFLAGS) $(OBJECTS) -o $(BIN_DIR)$@;

#building of sources
$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp
	$(MPICPP) $(CPPFLAGS) -c $< -o $@

#cleaning binaries
.PHONY : clean
clean:
	rm -rf $(BIN_DIR)*
