CC = g++
CPPFLAGS = -g -Wall -O2 -std=c++17 -Wno-sign-compare
LDFLAGS =

CPX_BASE    = /opt/ibm/ILOG/CPLEX_Studio2211
CPX_INCDIR  = $(CPX_BASE)/cplex/include
CPX_LIBDIR  = $(CPX_BASE)/cplex/lib/x86-64_linux/static_pic
CPX_LDFLAGS = -lcplex -lm -pthread -ldl

SRC = main.cpp TSPInstance.cpp TSPModel.cpp
OBJ = $(SRC:.cpp=.o)

TARGET = project

%.o: %.cpp
	$(CC) $(CPPFLAGS) -I$(CPX_INCDIR) -c $< -o $@

$(TARGET): $(OBJ)
	$(CC) $(CPPFLAGS) $(OBJ) -o $(TARGET) -L$(CPX_LIBDIR) $(CPX_LDFLAGS)

clean:
	rm -rf $(OBJ) $(TARGET)

.PHONY: clean