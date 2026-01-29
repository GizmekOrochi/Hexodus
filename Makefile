CXX = g++
CXXFLAGS = -Wall -Wextra -Wformat-security -std=c++20
SRC = $(shell find src -name '*.cpp')
BIN = bin/Hexodus

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(BIN)

testMassAlgebra:
	$(CXX) $(CXXFLAGS) tests/testMassAlgebra.cpp -o bin/testMassAlgebra

run: all
	./$(BIN)

clean:
	rm -f $(BIN)
