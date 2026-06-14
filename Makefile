CXX = g++
CXXFLAGS = -std=c++17 -O3 -Wall -Wextra
SRCDIR = src
OBJDIR = obj
BINDIR = .

SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))
TARGET = $(BINDIR)/chess-engine

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@ -lpthread

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR) $(TARGET)

test: $(TARGET)
	@echo "=== UCI test ==="
	@echo "uci" | ./$(TARGET)
	@echo ""
	@echo "=== Position test ==="
	@echo -e "position startpos\nd\nperft 3\nmoves\nquit" | ./$(TARGET)

.DEFAULT_GOAL := all