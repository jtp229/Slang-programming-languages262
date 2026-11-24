# You should not need to edit this file

# Executables to build.  We assume each .exe is built from all of the .o files
TARGETS = slang

# Default to 64 bits, but allow overriding on command line
BITS     ?= 64

# Give name to output folder, and ensure it is created before any compilation
ODIR          := ./obj$(BITS)
output_folder := $(shell mkdir -p $(ODIR))

# Names of all .exe files, .o files, .d files, and .c files
CFILES   = env.c evaluator.c xml.c gc.c lib_math.c lib_lists.c lib_string.c \
           lib_vector.c slang.c
EXEFILES = $(patsubst %, $(ODIR)/%.exe, $(TARGETS))
OFILES   = $(patsubst %.c, $(ODIR)/%.o, $(CFILES))
DFILES   = $(patsubst %.c, $(ODIR)/%.d, $(CFILES))

# Basic tool configuration for gcc
CC        = gcc
LD        = gcc
CFLAGS   += -MMD -O0 -m$(BITS) -ggdb -std=c11 -g -Wall -Werror -D_GNU_SOURCE
LDFLAGS  += -m$(BITS) -lm

# Build 'all' by default, and don't clobber .o files after each build
.DEFAULT_GOAL = all
.PRECIOUS: $(OFILES)
.PHONY: all clean

# Goal is to build all executables
all: $(EXEFILES)

# Rules for building object files
$(ODIR)/%.o: %.c
	@echo "[CC] $< --> $@"
	@$(CC) $< -o $@ -c $(CFLAGS)

# Rules for building executables
$(EXEFILES): $(OFILES)
	@echo "[LD] $^ --> $@"
	@$(LD) $^ -o $@ $(LDFLAGS)

# clean by clobbering the build folder
clean:
	@echo Cleaning up...
	@rm -rf $(ODIR)

# Include any dependencies we generated previously
-include $(DFILES)
