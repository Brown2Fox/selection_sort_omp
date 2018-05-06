
WORK_DIR=.
OUT_DIR=.build


GEN_NAME=gendata
GEN_SRC=generate_data.cpp

SORT_NAME=ssort
SORT_SRC=select_sort_omp.cpp

CXX=g++
CXX_FLAGS=-std=c++11 -fopenmp -O2
PARALLEL=-DPARALLEL
INNER_PARALLEL=-DINNER_PARALLEL


all: gen sorts
sorts: np npi p pi

gen: $(GEN_SRC)
	$(CXX) $(GEN_SRC) $(CXX_FLAGS) -o $(OUT_DIR)/$(GEN_NAME)

np: $(SORT_SRC)
	$(CXX) $(SORT_SRC) $(CXX_FLAGS) -o $(OUT_DIR)/$(SORT_NAME)_np

npi:
	$(CXX) $(SORT_SRC) $(CXX_FLAGS) $(INNER_PARALLEL) -o $(OUT_DIR)/$(SORT_NAME)_npi

p: $(SORT_SRC)
	$(CXX) $(SORT_SRC) $(CXX_FLAGS) $(PARALLEL) -o $(OUT_DIR)/$(SORT_NAME)_p

pi:
	$(CXX) $(SORT_SRC) $(CXX_FLAGS) $(PARALLEL) $(INNER_PARALLEL) -o $(OUT_DIR)/$(SORT_NAME)_pi

