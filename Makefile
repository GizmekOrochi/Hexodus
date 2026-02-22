CXX = g++
CXXFLAGS = -Wall -Wextra -Wformat-security -std=c++20 -lSDL2
SRC = $(shell find src -name '*.cpp')
BIN = bin/Hexodus

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(BIN) -lSDL2

testMassAlgebra:
	$(CXX) $(CXXFLAGS) tests/testMassAlgebra.cpp -o bin/testMassAlgebra -lSDL2

run: all
	./$(BIN)

clean:
	rm -f $(BIN)
