# dirs
BINDIR=./bin
OBJDIR=./obj
TEXDIR=./text
COMDIR=../common
COMOBJ=$(COMDIR)/obj
VPATH=$(COMDIR)

# tools
CC=g++
LD=g++
CFLAGS=-c -Wall -m32 -O1 -I$(COMDIR)
LDFLAGS=-m32


NAME=epctool
OBJS=$(COMOBJ)/fops.o $(OBJDIR)/EPCTool.o

$(COMOBJ)/%.o $(OBJDIR)/%.o: %.cpp
	$(CC) $(CFLAGS) $< -o $@

all: dirs $(OBJS)
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
