CXX=g++
SRC_DIR=src
OBJ_DIR=obj
BIN_DIR=bin
HDR_DIR=include
TEST_DIR=tests
OUTPUT=$(BIN_DIR)/sandbox

CXXFLAGS=-O3 -std=c++23 -Wall -Wextra -I$(HDR_DIR) -MMD -MP
LDLIBS=-lseccomp

SRC_FILES=$(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES=$(SRC_FILES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEP_FILES=$(OBJ_FILES:.o=.d)

TEST_CXXFLAGS=-O2 -std=c++23 -Wall -Wextra
TEST_CPP=$(wildcard $(TEST_DIR)/src/*.cpp)
TEST_SH=$(wildcard $(TEST_DIR)/src/*.sh)
TEST_BINS=$(TEST_CPP:$(TEST_DIR)/src/%.cpp=$(TEST_DIR)/bin/%) $(TEST_SH:$(TEST_DIR)/src/%=$(TEST_DIR)/bin/%)

$(OUTPUT) : $(OBJ_FILES) | $(BIN_DIR)
	$(CXX) $(OBJ_FILES) -o $@ $(LDLIBS)

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(TEST_DIR)/bin/% : $(TEST_DIR)/src/%.cpp | $(TEST_DIR)/bin
	$(CXX) $(TEST_CXXFLAGS) $< -o $@

$(TEST_DIR)/bin/%.sh : $(TEST_DIR)/src/%.sh | $(TEST_DIR)/bin
	install -m 755 $< $@

$(TEST_DIR)/bin:
	mkdir -p $(TEST_DIR)/bin

test: $(OUTPUT) $(TEST_BINS)
	./$(TEST_DIR)/run.sh

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(TEST_DIR)/bin

.PHONY: clean test

-include $(DEP_FILES)
