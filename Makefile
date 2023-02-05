SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
RELATIONAL_DIR=data/relational_schemas
JSON_DIR=data/json_schemas

SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
EXECUTABLE = $(BIN_DIR)/main
INPUT_FILE = $(RELATIONAL_DIR)/relational_schema2.txt
OUTPUT_FILE = $(JSON_DIR)/json_schema1.json

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	g++ $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	g++ -c $< -o $@

clean:
	rm -f $(OBJ_DIR)/*.o $(EXECUTABLE)

run: all
	./$(EXECUTABLE) $(INPUT_FILE) $(OUTPUT_FILE)

.PHONY: all clean
