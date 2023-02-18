SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
RELATIONAL_DIR=data/relational_schemas
JSON_DIR=data/json_schemas

SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
EXECUTABLE = $(BIN_DIR)/main


INPUT_FILE = $(RELATIONAL_DIR)/relational_schema$(FILE_SUFFIX).txt
OUTPUT_FILE = $(JSON_DIR)/json_schema$(FILE_SUFFIX).json
 
all: create_folders $(EXECUTABLE)

create_folders: 
	mkdir $(SRC_DIR) | echo "Folder already exists"
	mkdir $(OBJ_DIR)  | echo "Folder already exists"
	mkdir $(BIN_DIR)  | echo "Folder already exists"
	mkdir $(RELATIONAL_DIR)  | echo "Folder already exists"
	mkdir $(JSON_DIR) | echo "Folder already exists"

$(EXECUTABLE): $(OBJECTS)
	g++ $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	g++ -std=c++11 -c $< -o $@ -Wno-c++11-extensions

clean:
	rm -f $(OBJ_DIR)/*.o $(EXECUTABLE)

run: all 

	echo "\n\n\nRUNNING PROGRAM :)"
	./$(EXECUTABLE) $(INPUT_FILE) $(OUTPUT_FILE)

debug: CXXFLAGS += -g
debug: all
	gdb $(EXECUTABLE) $(INPUT_FILE) $(OUTPUT_FILE)
.PHONY: all clean
