# This file is intended to help main Makefile to assemble
# each individual tool
#
# Imports:
# 1) ($NAME) - tool name var should be overriden in main Makefile
#    (example: make -fMakefile.sub all/clean NAME=txttool)
# 2) ($OBJS, $LIBS) - obect lists will be included from make-list.mk
#    file in target dir


# dirs
SRCDIR=./$(NAME)
BINDIR=./$(SRCDIR)/bin
OBJDIR=./$(SRCDIR)/obj
TEXDIR=./$(SRCDIR)/text
COMDIR=./common
COMOBJ=$(COMDIR)/obj
VPATH=$(COMDIR):$(SRCDIR)

# import obect lists ($OBJS, $LIBS)
include ./$(NAME)/make-list.mk

# tools
CC=g++
LD=g++
CFLAGS=-c -Wall -m32 -O2 -I$(COMDIR)
LDFLAGS=-m32 $(LIBS)

# bake GCC libs into Windows binary to be able to run it without installing GCC
ifeq ($(OS),Windows_NT)
LDFLAGS:=$(LDFLAGS) -static-libgcc -static-libstdc++
endif

$(COMOBJ)/%.o $(OBJDIR)/%.o: %.cpp
	$(CC) $(CFLAGS) $< -o $@

all: zlib-chk dirs $(OBJS)
	$(LD) $(OBJS) -o $(BINDIR)/$(NAME) $(LDFLAGS)
ifeq ($(OS),Windows_NT)
	cp -a $(TEXDIR)/. $(BINDIR)/
else
	cp $(TEXDIR)/*.txt $(BINDIR)/
endif

dirs:
	mkdir -p $(BINDIR)
	mkdir -p $(OBJDIR)
	mkdir -p $(COMOBJ)

clean:
	-rm $(OBJS)
	-rm -rf $(BINDIR)
	-rm -rf $(OBJDIR)

zlib-chk:
	test -s $(COMOBJ)/libz.a || \
	{ echo "error: zlib isn't found, make zlib first"; exit 1; }
