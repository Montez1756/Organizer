SRC := $(wildcard *.cpp)
OBJ := $(SRC:.cpp=.o)
TARGET := organizer.exe
FLAGS := -I./raygui/src -I./raylib/include -L./raylib/lib -lraylib -lwinmm -lgdi32 -lshell32 -luser32 -mwindows
$(TARGET): $(OBJ)
	$(CXX) -o $@ $^ $(FLAGS)
	rm -rf *.o

%.o: %.cpp
	$(CXX) -v -c $< -o $@ $(FLAGS)

clean:
	rm -f $(OBJ) $(TARGET)
