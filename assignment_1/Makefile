# Usage: make {all | clean} CONF={debug|release} VERBOSE={0|1}
# Default is the "all" target, debug build, and verbose output

VERBOSE ?= 1
ifeq ($(VERBOSE),1)
	ECHO := 
else
	ECHO := @
endif

#Default build type
CONF ?= debug

# the compiler
CXX = g++

# target EXE
TARGET_DIR = .
TARGET = router

# the version of GTK being used
GTK_VERSION_NUM = 3.0

# the base directory of EZGL
EZGL_DIR = ezgl

# get the source files - router.cpp and all EZGL files (but not basic_application.cpp)
SRCS = router.cpp $(wildcard $(EZGL_DIR)/*.cpp)
HDRS = $(wildcard ./*.h $(EZGL_DIR)/*.hpp)

# the GTK include directories. Runs "pkg-config --cflags gtk+-3.0" to get the include directories.
GTK_INCLUDE_DIRS := $(shell pkg-config --cflags gtk+-$(GTK_VERSION_NUM) x11)

# the GTK libraries. Runs "pkg-config --libs gtk+-3.0" to get the libraries to include for GTK.
GTK_LIBS := $(shell pkg-config --libs gtk+-$(GTK_VERSION_NUM) x11)

# set the include directories
INC_DIRS = . $(EZGL_DIR) $(EZGL_DIR)/..

# compiler flags
CXX_FLAGS = -g -Wall -std=c++14

ifeq (release, $(CONF))
	CXX_FLAGS += -O3
else ifeq (debug, $(CONF))
# Don't change anything
else
    $(error Invalid value for CONF: '$(CONF)', must be 'release' or 'debug'. Try 'make help' for usage)
endif

# 'make all' target
all: $(TARGET_DIR)/$(TARGET)

# create the exe
$(TARGET_DIR)/$(TARGET) : Makefile $(SRCS)
	$(ECHO) $(CXX) $(CXX_FLAGS) $(foreach D,$(INC_DIRS),-I$D) $(GTK_INCLUDE_DIRS) $(SRCS) $(GTK_LIBS) -o $(TARGET_DIR)/$(TARGET)

# clean the EXE 
clean:
	$(ECHO) rm -f $(TARGET_DIR)/$(TARGET)

# Test targets for convenience
test1:
	@echo "Testing with cct1.txt (distributed architecture)..."
	$(ECHO) ./$(TARGET) -f cct1.txt -a distributed

test1_gui:
	@echo "Testing with cct1.txt (distributed architecture + GUI)..."
	$(ECHO) ./$(TARGET) -f cct1.txt -a distributed -i

test_tb:
	@echo "Testing with cct1.txt (top/bottom architecture)..."
	$(ECHO) ./$(TARGET) -f cct1.txt -a topbottom

test_tb_gui:
	@echo "Testing with cct1.txt (top/bottom architecture + GUI)..."
	$(ECHO) ./$(TARGET) -f cct1.txt -a topbottom -i

help:
	@echo "Makefile for ECE1387 Assignment 1 - FPGA Router"
	@echo ""
	@echo "Usage: "
	@echo '    > make -j4'
	@echo "        Call the default make target (all)."
	@echo "        This builds the router executable: '$(TARGET)'."
	@echo "        Use -j4 option to do parallel builds."
	@echo "    > make clean"
	@echo "        Removes the executable."
	@echo "    > make test1"
	@echo "        Run router on cct1.txt without GUI."
	@echo "    > make test1_gui"
	@echo "        Run router on cct1.txt with GUI."
	@echo "    > make test_tb"
	@echo "        Run router on cct1.txt with top/bottom architecture."
	@echo "    > make help"
	@echo "        Prints this help message."
	@echo ""
	@echo "Configuration Variables: "
	@echo "    CONF={release | debug}"
	@echo "        Controls whether the build performs compiler optimizations"
	@echo "        to improve performance. Currently set to '$(CONF)'."
	@echo ""
	@echo "        With CONF=release compiler optimization is enabled."
	@echo ""
	@echo "        With CONF=debug compiler optimization is disabled to improve"
	@echo "        interactive debugging."


.PHONY: all clean test1 test1_gui test_tb test_tb_gui help