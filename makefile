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


OPENMP = false	    # true of false to use or not of OpenMP in compilation
OPENMP_FLAG = -fopenmp
MPICPP = mpic++
CPPFLAGS =  -Ofast #-std=c++11

ifneq ($(OPENMP), true)
	CPPFLAGS += $(OPENMP_FLAG)
endif

SRC_DIR = src/
BIN_DIR = bin/
OBJ_DIR = $(BIN_DIR)obj/
BASE_DIR = base/
TASKS_DIR = tasks/tree-maker/
GENERATOR_DIR = generators/
CONTROLLER_DIR = controllers/

DIRECTORIES = $(OBJ_DIR)$(BASE_DIR) $(OBJ_DIR)$(TASKS_DIR) \
	      $(OBJ_DIR)$(GENERATOR_DIR) $(OBJ_DIR)$(CONTROLLER_DIR)

BASE = Graph SortedGraph RMAWindow Statistics
GENERATORS = SimpleGenerator
TASKS = BFSGraph500 BFSGraph500Optimized BFSTask ParentTree ParentTreeValidator TreeMakerTask
CONTROLLERS = TreeMakerController
FILES_LIST = $(addprefix $(BASE_DIR), $(BASE)) \
	    $(addprefix $(GENERATOR_DIR), $(GENERATORS)) \
	    $(addprefix $(TASKS_DIR), $(TASKS)) \
	    $(addprefix $(CONTROLLER_DIR), $(CONTROLLERS)) 

SOURCES = $(addprefix $(SRC_DIR), $(addsuffix .cpp, $(FILES_LIST)))
OBJECTS = $(addprefix $(OBJ_DIR), $(addsuffix .o, $(FILES_LIST)))

BUILD = dgmark dgmark_graph500 dgmark_graph500_opt	    # list of builds

all: $(BUILD)

$(DIRECTORIES):			    # prepairing directories.
	mkdir -p $@
	
dgmark: $(DIRECTORIES) $(OBJECTS) $(OBJ_DIR)main_dgmark.o # basic build
	$(MPICPP) $(CPPFLAGS) $(OBJECTS) $(OBJ_DIR)main_$@.o -o $(addprefix $(BIN_DIR), $@)
	
dgmark_graph500: $(DIRECTORIES) $(OBJECTS) $(OBJ_DIR)main_dgmark_graph500.o # original graph500 build
	$(MPICPP) $(CPPFLAGS) $(OBJECTS) $(OBJ_DIR)main_$@.o -o $(addprefix $(BIN_DIR), $@)
	
dgmark_graph500_opt: $(DIRECTORIES) $(OBJECTS) $(OBJ_DIR)main_dgmark_graph500_opt.o # optimized graph500 build
	$(MPICPP) $(CPPFLAGS) $(OBJECTS) $(OBJ_DIR)main_$@.o -o $(addprefix $(BIN_DIR), $@)
	
$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp	    # building of resource
	$(MPICPP) $(CPPFLAGS) -c $< -o $@

clean:
	rm -rf $(BIN_DIR)*
