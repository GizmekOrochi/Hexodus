CXX := g++

CXXFLAGS := \
	-Wall \
	-Wextra \
	-Wformat-security \
	-std=c++20 \
	-O3 \
	-march=native \
	-flto \
	-DNDEBUG

LDFLAGS := -flto

SRC := $(shell find src -name '*.cpp')

BIN := bin/Hexodus

.PHONY: all clean run debug testMassAlgebra

all: $(BIN)

$(BIN): $(SRC)
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $(SRC) $(LDFLAGS) -o $(BIN)

debug:
	@mkdir -p bin
	$(CXX) \
		-Wall \
		-Wextra \
		-Wformat-security \
		-std=c++20 \
		-O0 \
		-g3 \
		$(SRC) \
		-o $(BIN)

testMassAlgebra:
	@mkdir -p bin
	$(CXX) \
		$(CXXFLAGS) \
		tests/testMassAlgebra.cpp \
		-o bin/testMassAlgebra

run: all
	./$(BIN)

clean:
	rm -f $(BIN)