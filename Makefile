# Makefile for mdns-repeater-rewrite


ZIP_NAME = mdns-repeater-rewrite-$(GITVERSION)

ZIP_FILES = mdns-repeater-rewrite	\
			README.txt		\
			LICENSE.txt

GITVERSION=$(shell git rev-parse HEAD )

CFLAGS=-Wall

ifdef DEBUG
CFLAGS+= -g
else
CFLAGS+= -Os
LDFLAGS+= -s
endif

CFLAGS+= -DGITVERSION="\"${GITVERSION}\""

.PHONY: all clean

all: mdns-repeater-rewrite

mdns-repeater-rewrite.o: _gitversion

mdns-repeater-rewrite: mdns-repeater-rewrite.o

.PHONY: zip
zip: TMPDIR := $(shell mktemp -d)
zip: mdns-repeater-rewrite
	mkdir $(TMPDIR)/$(ZIP_NAME)
	cp $(ZIP_FILES) $(TMPDIR)/$(ZIP_NAME)
	-$(RM) $(CURDIR)/$(ZIP_NAME).zip
	cd $(TMPDIR) && zip -r $(CURDIR)/$(ZIP_NAME).zip $(ZIP_NAME)
	-$(RM) -rf $(TMPDIR)

# version checking rules
.PHONY: dummy
_gitversion: dummy
	@echo $(GITVERSION) | cmp -s $@ - || echo $(GITVERSION) > $@

clean:
	-$(RM) *.o
	-$(RM) mdns-repeater-rewrite
	-$(RM) mdns-repeater-rewrite-*.zip

