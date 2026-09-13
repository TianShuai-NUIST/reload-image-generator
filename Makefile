CC := clang
SRC := main.c
TARGET := reig
CFLAGS := -Wall -Wextra -g

$(TARGET): $(SRC)
		$(CC) -o $(TARGET) $(SRC) $(CFLAGS)

build: $(TARGET)

run: $(TARGET)
		@./$(TARGET)

clean:
		-rm ./$(TARGET)

