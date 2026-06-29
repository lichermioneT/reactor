CXX := g++

CXXFLAGS := -std=c++11 -Wall -Wextra -g
LDFLAGS := -pthread

TARGET := server

SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=.o)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: all clean rebuild

all: $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

rebuild: clean all
