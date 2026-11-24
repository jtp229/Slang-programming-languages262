# You should not need to edit this file

# Executables to build.  We assume each .exe is built from just one .cc file
TARGETS = slang

# Default to 64 bits, but allow overriding on command line
BITS     ?= 64

# Give name to output folder, and ensure it is created before any compilation
ODIR          := ./obj$(BITS)
output_folder := $(shell mkdir -p $(ODIR))

# Names of all .exe files, .o files, and .d files
EXEFILES = $(patsubst %, $(ODIR)/%.exe, $(TARGETS))
OFILES   = $(patsubst %, $(ODIR)/%.o, $(TARGETS))
DFILES   = $(patsubst %, $(ODIR)/%.d, $(TARGETS))

# Basic tool configuration for gcc/g++
CXX       = g++
LD        = g++
CXXFLAGS += -MMD -O3 -m$(BITS) -ggdb -std=c++2a -g -Wall -Werror 
LDFLAGS  += -m$(BITS) -ltinyxml2

# Build 'all' by default, and don't clobber .o files after each build
.DEFAULT_GOAL = all
.PRECIOUS: $(OFILES)
.PHONY: all clean

# Goal is to build all executables
all: $(EXEFILES)

# Rules for building object files
$(ODIR)/%.o: %.cc
	@echo "[CXX] $< --> $@"
	@$(CXX) $< -o $@ -c $(CXXFLAGS)

# Rules for building executables
$(ODIR)/%.exe: $(ODIR)/%.o
	@echo "[LD] $^ --> $@"
	@$(LD) $^ -o $@ $(LDFLAGS)

# clean by clobbering the build folder
clean:
	@echo Cleaning up...
	@rm -rf $(ODIR)

# Include any dependencies we generated previously
-include $(DFILES)
