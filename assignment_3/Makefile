CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread -O2

# GTK3 flags (for EZGL graphics)
# Add -I. so "ezgl/xxx.hpp" includes work
GTK_CFLAGS = -I. $(shell pkg-config --cflags gtk+-3.0)
GTK_LIBS = $(shell pkg-config --libs gtk+-3.0) -lX11

TARGET = partitioner

# Main source files
MAIN_OBJS = main.o parser.o partitioner.o graphics.o

# EZGL source files
EZGL_OBJS = ezgl/application.o ezgl/callback.o ezgl/camera.o \
            ezgl/canvas.o ezgl/control.o ezgl/graphics.o

OBJS = $(MAIN_OBJS) $(EZGL_OBJS)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(GTK_LIBS)

# Main source files
main.o: main.cpp parser.h partitioner.h circuit.h graphics.h
	$(CXX) $(CXXFLAGS) $(GTK_CFLAGS) -c main.cpp

parser.o: parser.cpp parser.h circuit.h
	$(CXX) $(CXXFLAGS) $(GTK_CFLAGS) -c parser.cpp

partitioner.o: partitioner.cpp partitioner.h circuit.h graphics.h
	$(CXX) $(CXXFLAGS) $(GTK_CFLAGS) -c partitioner.cpp

graphics.o: graphics.cpp graphics.h circuit.h
	$(CXX) $(CXXFLAGS) $(GTK_CFLAGS) -c graphics.cpp

# EZGL source files
ezgl/%.o: ezgl/%.cpp
	$(CXX) $(CXXFLAGS) $(GTK_CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(MAIN_OBJS) $(EZGL_OBJS)

.PHONY: all clean