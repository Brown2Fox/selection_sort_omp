
WORK_DIR=.
OUT_DIR=.build


GEN_NAME=gendata
GEN_SRC=generate_data.cpp

SORT_NAME=ssort
SORT_SRC=select_sort_omp.cpp

CXX=g++
CXX_FLAGS=-std=c++11 -fopenmp -O2

all: gen sort

gen: $(GEN_SRC)
	$(CXX) $(GEN_SRC) $(CXX_FLAGS) -o $(OUT_DIR)/$(GEN_NAME)


sort: $(SORT_SRC)
	$(CXX) $(SORT_SRC) $(CXX_FLAGS) -o $(OUT_DIR)/$(SORT_NAME)