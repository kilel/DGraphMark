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
CPPFLAGS = -std=c++11

ifneq ($(OPENMP), true)
	CPPFLAGS += $(OPENMP_FLAG)
endif

SRC_DIR = src/
SRC_BASE_DIR = $(SRC_DIR)base/
SRC_TASKS_DIR = $(SRC_DIR)tasks/

SRC_BASE = Edge.cpp Graph.cpp 
SOURCES = src/main.cpp $(addprefix $(SRC_BASE_DIR), $(SRC_BASE)) $(SRC_STUB)

BUILD = dgmark	    # list of builds

all: $(BUILD)
bin:		    # preparing bin catalogue
	mkdir bin
	
dgmark: bin	    # std build
	$(MPICPP) $(CPPFLAGS) $(SOURCES) -o $(addprefix bin/, $@)
	
clean:
	#rm -f bin/*
