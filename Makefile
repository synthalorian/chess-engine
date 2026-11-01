CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra
SRCDIR = src
OBJDIR = obj
BINDIR = .

SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))
TARGET = $(BINDIR)/chess-engine

.PHONY: all clean test test-slow

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@ -lpthread

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/main.o: src/perft_stats.h

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR) $(TARGET) $(TEST_TARGET) $(STATS_TEST_TARGET)

TEST_TARGET = tests/perft_test
STATS_TEST_TARGET = tests/perft_stats_test

$(TEST_TARGET): tests/perft_test.cpp src/board.cpp src/movegen.cpp src/board.h src/move.h src/bitboard.h
	$(CXX) $(CXXFLAGS) -Isrc tests/perft_test.cpp src/board.cpp src/movegen.cpp -o $@

$(STATS_TEST_TARGET): tests/perft_stats_test.cpp src/perft_stats.h
	$(CXX) $(CXXFLAGS) -Isrc $< -o $@

test: $(TEST_TARGET) $(STATS_TEST_TARGET)
	./$(TEST_TARGET)
	./$(STATS_TEST_TARGET)

test-slow: $(TEST_TARGET)
	./$(TEST_TARGET) --slow

.DEFAULT_GOAL := all
