CC := clang
SRC := main.c
TARGET := reig

$(TARGET): $(SRC)
		$(CC) -o $(TARGET) $(SRC)

run: $(TARGET)
		@./$(TARGET)

clean:
		-rm ./$(TARGET)

