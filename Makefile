CC = gcc
CFLAGS = -Wall -Wextra -Ilib -Isrc -Iexm

SRC_DIR = src
LIB_DIR = lib
EXM_DIR = exm
BUILD_DIR = build

LIBRARY = $(BUILD_DIR)/libiso15765.a
LIB_DEP = $(BUILD_DIR)/libiqueue.a
EXAMPLE = $(BUILD_DIR)/example

SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
LIB_FILES = $(wildcard $(LIB_DIR)/*.c)
EXM_FILES = $(wildcard $(EXM_DIR)/*.c)

SRC_OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/src_%.o, $(SRC_FILES))
LIB_OBJS = $(patsubst $(LIB_DIR)/%.c, $(BUILD_DIR)/lib_%.o, $(LIB_FILES))
EXM_OBJS = $(patsubst $(EXM_DIR)/%.c, $(BUILD_DIR)/exm_%.o, $(EXM_FILES))

# Rules
all: $(LIBRARY) $(EXAMPLE)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile dependency library
$(LIB_DEP): $(BUILD_DIR) $(LIB_OBJS)
	ar rcs $@ $(LIB_OBJS)

$(BUILD_DIR)/lib_%.o: $(LIB_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile main library
$(LIBRARY): $(LIB_DEP) $(SRC_OBJS)
	ar rcs $@ $(SRC_OBJS)

$(BUILD_DIR)/src_%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compile example
$(EXAMPLE): $(LIBRARY) $(LIB_DEP) $(EXM_OBJS)
	$(CC) $(CFLAGS) $(EXM_OBJS) $(LIBRARY) $(LIB_DEP) -o $@

$(BUILD_DIR)/exm_%.o: $(EXM_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

rebuild: clean all

.PHONY: all clean
