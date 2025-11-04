CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -g

# SuiteSparse paths
SUITESPARSE_ROOT = ./SuiteSparse-5.10.1
UMFPACK_INC = $(SUITESPARSE_ROOT)/UMFPACK/Include
AMD_INC = $(SUITESPARSE_ROOT)/AMD/Include
SUITESPARSE_CONFIG = $(SUITESPARSE_ROOT)/SuiteSparse_config

UMFPACK_LIB = $(SUITESPARSE_ROOT)/UMFPACK/Lib
AMD_LIB = $(SUITESPARSE_ROOT)/AMD/Lib

# EZGL and GTK paths
EZ_INC = -I.     # Changed from -I./ezgl
GTK_CFLAGS = $(shell pkg-config --cflags gtk+-3.0)
GTK_LIBS = $(shell pkg-config --libs gtk+-3.0)

# Include paths
INCLUDES = -I$(UMFPACK_INC) -I$(AMD_INC) -I$(SUITESPARSE_CONFIG) $(EZ_INC) $(GTK_CFLAGS)

# Library paths
LDFLAGS = -L$(UMFPACK_LIB) -L$(AMD_LIB) -L$(SUITESPARSE_CONFIG)

# Libraries to link
LIBS = -lumfpack -lamd -lsuitesparseconfig -lm $(GTK_LIBS) -lX11

# Target executable
TARGET = placer

# EZGL source files
EZGL_SRCS = ezgl/graphics.cpp \
            ezgl/application.cpp \
            ezgl/canvas.cpp \
            ezgl/camera.cpp \
            ezgl/control.cpp \
            ezgl/callback.cpp

# Main source files
SRCS = main.cpp \
       input_parser.cpp \
       weight_matrix.cpp \
       umfpack_solver.cpp \
       hpwl_calculator.cpp \
       graphics.cpp \
       spreader.cpp

# All source files
ALL_SRCS = $(SRCS) $(EZGL_SRCS)

# Object files
OBJS = $(ALL_SRCS:.cpp=.o)

# Default target
all: build-suitesparse $(TARGET)

# Build SuiteSparse libraries (always)
build-suitesparse:
	@echo "========================================"
	@echo "Building SuiteSparse libraries..."
	@echo "========================================"
	@cd $(SUITESPARSE_ROOT) && $(MAKE) clean
	@cd $(SUITESPARSE_ROOT)/SuiteSparse_config && $(MAKE)
	@cd $(SUITESPARSE_ROOT)/AMD && $(MAKE) library
	@cd $(SUITESPARSE_ROOT)/UMFPACK && $(MAKE) library
	@echo "========================================"
	@echo "SuiteSparse build complete!"
	@echo "========================================"

# Link
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS) $(LIBS)

# Compile
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Dependencies
main.o: main.cpp data_structures.h input_parser.h weight_matrix.h umfpack_solver.h hpwl_calculator.h graphics.h spreader.h
spreader.o: spreader.cpp spreader.h data_structures.h hpwl_calculator.h
input_parser.o: input_parser.cpp input_parser.h data_structures.h
weight_matrix.o: weight_matrix.cpp weight_matrix.h data_structures.h
umfpack_solver.o: umfpack_solver.cpp umfpack_solver.h weight_matrix.h data_structures.h
hpwl_calculator.o: hpwl_calculator.cpp hpwl_calculator.h data_structures.h
graphics.o: graphics.cpp graphics.h data_structures.h

# Clean
clean:
	rm -f $(OBJS) $(TARGET)

# Clean everything including SuiteSparse
distclean: clean
	@echo "Cleaning SuiteSparse..."
	@cd $(SUITESPARSE_ROOT) && $(MAKE) clean
	@echo "All clean!"

# Phony targets
.PHONY: all clean distclean build-suitesparse
