CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread

TARGET = partitioner
OBJS = main.o parser.o partitioner.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

main.o: main.cpp parser.h partitioner.h circuit.h
	$(CXX) $(CXXFLAGS) -c main.cpp

parser.o: parser.cpp parser.h circuit.h
	$(CXX) $(CXXFLAGS) -c parser.cpp

partitioner.o: partitioner.cpp partitioner.h circuit.h
	$(CXX) $(CXXFLAGS) -c partitioner.cpp

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all clean