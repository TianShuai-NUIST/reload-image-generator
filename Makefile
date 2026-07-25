CC := clang
SRC := main.c
TARGET := reig

$(TARGET): $(SRC)
		$(CC) -o $(TARGET) $(SRC)

build: $(TARGET)

run: $(TARGET)
		@./$(TARGET)

clean:
		-rm ./$(TARGET)

