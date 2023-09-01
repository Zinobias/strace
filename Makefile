CC := clang++

ifeq ($(PROD), true)
	CFLAGS := -Werror -Wall -Wextra -std=c++20 -o2
else
	CFLAGS := -Werror -Wall -Wextra -std=c++20 -fsanitize=address -fsanitize=thread -D PROD
endif

SRC_DIR := ./src
OBJ_DIR := ./build
BIN_DIR := ./bin

SRCS := $(wildcard $(SRC_DIR)/*.cpp)
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
EXEC := $(BIN_DIR)/strace


all: $(EXEC)

$(EXEC): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	$(RM) -r $(OBJ_DIR)

fclean: clean
	$(RM) -r $(BIN_DIR)

re: fclean all

run: $(EXEC)
	$(EXEC)

rrun: re run

.PHONY: all clean fclean re rrun run
