SRC := $(wildcard *.c)
OBJ := $(SRC:.c=.o)
TARGET := organizer.exe

$(TARGET): $(OBJ)
	$(CC) -o $@ $^
	rm -rf *.o

%.o: %.c
	$(CC) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
