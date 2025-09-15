# Makefile for mdns-repeater-rewrite


ZIP_NAME = mdns-repeater-rewrite-$(HGVERSION)

ZIP_FILES = mdns-repeater-rewrite	\
			README.txt		\
			LICENSE.txt

HGVERSION=$(shell git rev-parse HEAD )

CFLAGS=-Wall

ifdef DEBUG
CFLAGS+= -g
else
CFLAGS+= -Os
LDFLAGS+= -s
endif

CFLAGS+= -DHGVERSION="\"${HGVERSION}\""

.PHONY: all clean

all: mdns-repeater-rewrite

mdns-repeater-rewrite.o: _hgversion

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
_hgversion: dummy
	@echo $(HGVERSION) | cmp -s $@ - || echo $(HGVERSION) > $@

clean:
	-$(RM) *.o
	-$(RM) _hgversion
	-$(RM) mdns-repeater-rewrite
	-$(RM) mdns-repeater-rewrite-*.zip

