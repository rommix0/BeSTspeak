#=======================================================
#
# Makefile for g++/gcc
#
# meant to be used with build environments such as
# MINGW and WinLibs.
#
#=======================================================

TARGET_EXEC := bst_spk

SRCS := bst_spk.cpp argparse.c

OBJS   := $(SRCS:%=%.o)

CPP      = g++
CXXFLAGS = -std=c++0x -fpermissive

$(TARGET_EXEC): $(OBJS)
	$(CPP) $(OBJS) -o $@

%.c.o: %.c
	$(CPP) $(CXXFLAGS) -c $< -o $@

%.cpp.o: %.cpp
	$(CPP) $(CXXFLAGS) -c $< -o $@

clean:
	rm *.o
