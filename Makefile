CXX=g++
SRC_DIR=src
OBJ_DIR=obj
BIN_DIR=bin
HDR_DIR=include
OUTPUT=$(BIN_DIR)/sandbox

CXXFLAGS=-O3 -std=c++23 -Wall -Wextra -I$(HDR_DIR) -MMD -MP
LDLIBS=-lseccomp

SRC_FILES=$(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES=$(SRC_FILES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEP_FILES=$(OBJ_FILES:.o=.d)

$(OUTPUT) : $(OBJ_FILES) | $(BIN_DIR)
	$(CXX) $(OBJ_FILES) -o $@ $(LDLIBS)

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

run: $(OUTPUT)
	./$(OUTPUT) $(ARGS)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: clean run

-include $(DEP_FILES)
