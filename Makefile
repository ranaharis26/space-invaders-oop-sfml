CXX      = g++
CXXFLAGS = -std=c++17 -Wall -O2
LDFLAGS  = -lsfml-graphics -lsfml-window -lsfml-audio -lsfml-system

SRCDIR = .
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)
TARGET  = SpaceInvaders

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

$(SRCDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(SRCDIR)/*.o $(TARGET)

run: all
	./$(TARGET)

.PHONY: all clean run
