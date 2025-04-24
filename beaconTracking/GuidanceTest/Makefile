# Makefile for GuidanceTest

CC       := gcc
AR       := ar

SRC_DIR  := Src
INC_DIR  := Inc
TEST_DIR := tests

CFLAGS   := -std=c99 -Wall -I$(INC_DIR)
LDFLAGS  := -L. -lguidance -lm

SRC_SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS     := $(patsubst $(SRC_DIR)/%.c,%.o,$(SRC_SRCS))

LIB       := libguidance.a
TEST_SRC  := $(TEST_DIR)/guidance_test.c
TEST_OBJ  := guidance_test.o
TEST_BIN  := guidance_test.exe

.PHONY: all run clean

all: $(LIB) $(TEST_BIN)

# 1) Build library objects
%.o: $(SRC_DIR)/%.c $(INC_DIR)/%.h
	$(CC) $(CFLAGS) -c $< -o $@

# 2) Archive into static library
$(LIB): $(OBJS)
	$(AR) rcs $@ $^

# 3) Compile test object
$(TEST_OBJ): $(TEST_SRC) $(INC_DIR)/guidance.h
	$(CC) $(CFLAGS) -c $< -o $@

# 4) Link test executable
$(TEST_BIN): $(LIB) $(TEST_OBJ)
	$(CC) $(TEST_OBJ) $(LDFLAGS) -o $@

# Convenience: build + run
run: all
	./$(TEST_BIN)

# Clean up
clean:
	rm -f *.o $(LIB) $(TEST_OBJ) $(TEST_BIN)
