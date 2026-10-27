.DEFAULT_GOAL := all
CXX ?= g++
CPPFLAGS ?=
CXXFLAGS ?=
LDFLAGS ?=
LDLIBS ?=

COMMON_FLAGS := -std=c++20 -Wall -Wextra -Wformat-security
RELEASE_FLAGS ?= -O3 -DNDEBUG
DEBUG_FLAGS ?= -O0 -g3
TEST_FLAGS ?= -O2 -g
SANITIZE_FLAGS ?= -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer

SRC := $(sort $(shell find src -name '*.cpp'))
CORE_SRC := $(filter-out src/main.cpp,$(SRC))
TEST_SRC := $(sort $(shell find tests -name '*.cpp'))
ALGEBRA_SRC := tests/main.cpp tests/testMassAlgebra.cpp src/core/TUI-Rendering/Geometry/object/Matrix.cpp src/core/TUI-Rendering/Geometry/object/Vector.cpp

.PHONY: all run debug test test-algebra testMassAlgebra test-sanitize clean help
all: bin/Hexodus
run: bin/Hexodus
	./bin/Hexodus
debug: bin/Hexodus-debug
test: bin/tests
	./bin/tests
test-algebra: bin/testMassAlgebra
	./bin/testMassAlgebra
# Compatibility with the previous target, now using the common test runner.
testMassAlgebra: test-algebra
test-sanitize: bin/tests-sanitize
	./bin/tests-sanitize

# Separate object trees prevent release/debug/sanitizer flags from mixing.
# .d files track included headers, so incremental rebuilds are reliable.
define BUILD_PROFILE
$(1)_OBJECTS := $$(patsubst %.cpp,build/$(1)/%.o,$(3))
$(2): $$($(1)_OBJECTS) Makefile
	@mkdir -p $$(@D)
	$$(CXX) $(4) $$(CXXFLAGS) $$(LDFLAGS) $$($(1)_OBJECTS) $$(LDLIBS) -o $$@
build/$(1)/%.o: %.cpp Makefile
	@mkdir -p $$(@D)
	$$(CXX) $$(CPPFLAGS) $$(COMMON_FLAGS) $(4) $$(CXXFLAGS) -MMD -MP -c $$< -o $$@
-include $$($(1)_OBJECTS:.o=.d)
endef

$(eval $(call BUILD_PROFILE,release,bin/Hexodus,$(SRC),$(RELEASE_FLAGS)))
$(eval $(call BUILD_PROFILE,debug,bin/Hexodus-debug,$(SRC),$(DEBUG_FLAGS)))
$(eval $(call BUILD_PROFILE,test,bin/tests,$(CORE_SRC) $(TEST_SRC),$(TEST_FLAGS)))
$(eval $(call BUILD_PROFILE,algebra,bin/testMassAlgebra,$(ALGEBRA_SRC),$(TEST_FLAGS)))
$(eval $(call BUILD_PROFILE,sanitize,bin/tests-sanitize,$(CORE_SRC) $(TEST_SRC),$(SANITIZE_FLAGS)))

clean:
	rm -rf build
	rm -f bin/Hexodus bin/Hexodus-debug bin/tests bin/testMassAlgebra bin/tests-sanitize
help:
	@echo 'make [-jN] Build release (bin/Hexodus)'
	@echo 'make run Build and run the application'
	@echo 'make debug Build debug (bin/Hexodus-debug)'
	@echo 'make test Build and run all test suites'
	@echo 'make test-algebra Run only the algebra suites'
	@echo 'make test-sanitize Run all suites with ASan + UBSan'
	@echo 'make clean Remove build outputs'
	@echo 'After changing command-line compiler flags, run make clean.'
