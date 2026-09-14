CC := clang

BUILD_DIR := build

SRCS := $(wildcard src/*.c)
OBJS := $(patsubst src/%.c, build/%.o, $(SRCS))

TARGET := $(BUILD_DIR)/reig

TEST_SRC := tests/test_parse.c
TEST_BIN := $(BUILD_DIR)/test_parse

CFLAGS := -Wall -Wextra -g -Iinclude
DEPFLAGS := -MMD -MP
LDLIBS :=

.PHONY: build run clean all test

all: $(TARGET)

test: $(TEST_BIN)
	@./$(TEST_BIN)

$(TEST_BIN): $(TEST_SRC) build/config.o
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $(TEST_SRC) build/config.o

profile: CFLAGS += -pg 
profile: LDFLAGS += -pg
profile: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDLIBS) $(LDFLAGS)

$(BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

build: $(TARGET)

run: $(TARGET)
	@./$(TARGET) ./scripts/in.fp

clean:
	-rm -rf ./$(BUILD_DIR)

-include $(OBJS:.o=.d)
