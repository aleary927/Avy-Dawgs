# Makefile for GuidanceTest
# Place in:
# C:/Users/sikor/Documents/CS5780/Avy_Dawgs/Avy-Dawgs/beaconTracking/GuidanceTest/Makefile

CC      := gcc
AR      := ar
CFLAGS  := -std=c99 -Wall -IInc
LDFLAGS := -L. -lguidance -lm

SRC_DIR   := Src
INC_DIR   := Inc
TEST_SRC  := tests/guidance_test.c
TEST_BIN  := guidance_test.exe

# all .c under Src/, and corresponding .o in the root
SRC_C     := $(wildcard $(SRC_DIR)/*.c)
OBJS      := $(patsubst $(SRC_DIR)/%.c,%.o,$(SRC_C))

LIB       := libguidance.a

.PHONY: all run clean

all: $(LIB) $(TEST_BIN)

# 1) Build library objects
#    e.g. guidance.o from Src/guidance.c, circ_buf.o from Src/circ_buf.c
%.o: $(SRC_DIR)/%.c $(INC_DIR)/%.h
	$(CC) $(CFLAGS) -c $< -o $@

# 2) Archive into static library
$(LIB): $(OBJS)
	$(AR) rcs $@ $^

# 3) Compile & link test
$(TEST_BIN): $(TEST_SRC) $(LIB)
	$(CC) $(CFLAGS) -c $< -o test.o
	$(CC) test.o $(LDFLAGS) -o $@

# Convenience: build + run
run: all
	./$(TEST_BIN)

# Clean up
clean:
	rm -f *.o $(LIB) $(TEST_BIN)
