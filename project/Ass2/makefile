CC = g++
CPPFLAGS = -g -Wall -O2 -std=c++17 -Wno-sign-compare

SRC = main.cpp TSPInstance.cpp TSPHeuristic.cpp
OBJ = $(SRC:.cpp=.o)

TARGET = project

%.o: %.cpp
	$(CC) $(CPPFLAGS) -c $< -o $@

$(TARGET): $(OBJ)
	$(CC) $(CPPFLAGS) $(OBJ) -o $(TARGET)

clean:
	rm -rf $(OBJ) $(TARGET)

.PHONY: clean