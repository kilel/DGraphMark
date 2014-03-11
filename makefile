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
OPENMP = true
OPENMP_FLAG = -fopenmp
MPICPP = mpic++
CPPFLAGS = -Ofast #-std=c++11

ifeq ($(OPENMP), true)
	CPPFLAGS += $(OPENMP_FLAG)
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
		$(BENCHMARK_DIR) $(CONTROLLER_DIR) $(GENERATOR_DIR)  \
		$(GRAPH_DIR) $(MPI_DIR) $(TASK_DIR) $(BFS_DIR) \
		$(VALIDATOR_DIR) $(UTIL_DIR) )

#Definitions of sources to compile
BENCHMARK = 
CONTROLLER = SearchController
GENERATOR = SimpleGenerator
GRAPH = Graph SortedGraph
MPI = RMAWindow
TASK = ParentTree ParentTreeValidator SearchTask
BFS = BFSGraph500 BFSGraph500Optimized BFSTask
VALIDATOR = ParentTreeValidatorRMAFetch
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
	    $(addprefix $(UTIL_DIR), $(UTIL)) 
#full sources and objects paths
SOURCES = $(addprefix $(SRC_DIR), $(addsuffix .cpp, $(FILES_LIST)))
OBJECTS = $(addprefix $(OBJ_DIR), $(addsuffix .o, $(FILES_LIST)))

#build targets
BUILD = dgmark dgmark_graph500 dgmark_graph500_opt

#build rules
all: $(BUILD)

# prepairing directories.
$(OBJ_DIR_PATHS):					    
	mkdir -p $@

# basic build
dgmark: $(OBJ_DIR_PATHS) $(OBJECTS) $(OBJ_DIR)main_dgmark.o
	$(MPICPP) $(CPPFLAGS) $(OBJECTS) $(OBJ_DIR)main_$@.o -o $(addprefix $(BIN_DIR), $@)

# extended builds
dgmark_%: $(OBJ_DIR_PATHS) $(OBJECTS) $(OBJ_DIR)main_dgmark_%.o
	$(MPICPP) $(CPPFLAGS) $(OBJECTS) $(OBJ_DIR)main_$@.o -o $(addprefix $(BIN_DIR), $@)

#building of sources
$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp
	$(MPICPP) $(CPPFLAGS) -c $< -o $@

#cleaning binaries
clean:
	rm -rf $(BIN_DIR)*
