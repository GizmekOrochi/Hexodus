CXX = g++
CXXFLAGS = -Wall -Wextra -Wformat-security -std=c++20
SRC = $(shell find src -name '*.cpp')
BIN = bin/Hexodus

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(BIN)

run: all
	./$(BIN)

clean:
	rm -f $(BIN)
