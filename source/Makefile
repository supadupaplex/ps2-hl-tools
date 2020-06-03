# This works only on Windows so far
# I used MSYS + Mingw32 4.8.2


# dirs
COMDIR=./common
BLDDIR=./build
COMOBJ=$(COMDIR)/obj
RELDIR=../release
LICFILE=../license.txt
RMEFILE=../README.md

# zlib stuff
ZLIB_VER=1.2.11
ZLIB_DIR=$(COMDIR)/zlib
ZLIB_WDIR=$(ZLIB_DIR)/zlib-$(ZLIB_VER)


all: epctool \
	mdltool \
	mustool \
	nodtool \
	paktool \
	phdtool \
	psitool \
	sprtool \
	txttool

clean: epctool-clean \
	mdltool-clean \
	mustool-clean \
	nodtool-clean \
	paktool-clean \
	phdtool-clean \
	psitool-clean \
	sprtool-clean \
	txttool-clean

release: clean-all all
	echo "build date: `date +%d-%m-%Y`, build time: `date +%H-%M-%S`" > $(BLDDIR)/build-info.txt
	cp $(LICFILE) $(BLDDIR)/
	cp $(RMEFILE) $(BLDDIR)/
	zip -r $(RELDIR)/release-latest.zip $(BLDDIR)/

# tools
%tool: zlib-build
	cd $@ && "$(MAKE)"
	mv $@/bin $(BLDDIR)/$@

%tool-clean:
	cd $(firstword $(subst -, ,$@)) && "$(MAKE)" clean
	rm -rf $(BLDDIR)/$(firstword $(subst -, ,$@))

dirs:
	mkdir -p $(COMOBJ)
	mkdir -p $(BLDDIR)

clean-all: clean zlib-clean
	-rm -rf $(COMOBJ)
	-rm -rf $(BLDDIR)

# ZLIB
zlib-build: dirs
ifeq ($(OS),Windows_NT)
	# windows
	cd $(ZLIB_DIR)	&& tar -xzf zlib-$(ZLIB_VER).tar.gz
	cd $(ZLIB_WDIR)	&& "$(MAKE)" -fwin32/Makefile.gcc
	cp $(ZLIB_WDIR)/libz.a	$(COMOBJ)/
	cp $(ZLIB_WDIR)/zlib.h	$(COMDIR)/
	cp $(ZLIB_WDIR)/zconf.h	$(COMDIR)/
else
	# other os
	echo "Not implemented"
	exit 1
endif

zlib-clean:
	-rm -rf $(ZLIB_WDIR)
	-rm $(COMDIR)/zlib.h
	-rm $(COMDIR)/zconf.h
	-rm $(COMOBJ)/libz.a
