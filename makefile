SRC_DIR = src
TEST_DIR = tests
OBJ_DIR = obj
TEST_OBJ_DIR = testObj
BIN_DIR = .

EXE = $(BIN_DIR)/able
TEST = $(BIN_DIR)/test
SRC = $(wildcard $(SRC_DIR)/*.cpp)
SRC_TEST = $(wildcard $(TEST_DIR)/*.cpp)
OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
OBJ_WITHOUT_MAIN = $(filter-out $(OBJ_DIR)/main.o, $(OBJ))
OBJ_TEST = $(SRC_TEST:$(TEST_DIR)/%.cpp=$(TEST_OBJ_DIR)/%.o)

CPPFLAGS = -Iinclude -MMD -MP
CFLAGS = -Wall
LDFLAGS =  
LDLIBS = -lgtest -lgmock -lpthread

.PHONY = all clean

all: $(EXE) $(TEST)

$(EXE): $(OBJ) | $(BIN_DIR)
	g++ $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	g++ $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR) $(TEST_OBJ_DIR):
	mkdir -p $@

$(TEST): $(OBJ_WITHOUT_MAIN) $(OBJ_TEST) | $(BIN_DIR)
	g++ $(LDFLAGS) $^ $(LDLIBS) -o $@
	$(BIN_DIR)/$@

$(TEST_OBJ_DIR)/%.o: $(TEST_DIR)/%.cpp | $(TEST_OBJ_DIR)
	g++ $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	@$(RM) -rv $(OBJ_DIR) $(TEST_OBJ_DIR) $(EXE) $(TEST)

-include $(OBJ:.o=.d)