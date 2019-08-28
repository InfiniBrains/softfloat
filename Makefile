# Makefile for softFloat
#
# (C) ETH Zurich, all rights reserved
# Stefan Mach, Integrated Systems Laboratory

L386PATH = build/Linux-386-GCC

.PHONY: all build clean

all: linux386

linux386: BUILDPATH = $(L386PATH)
linux386: build

build:
	$(MAKE) -C $(BUILDPATH)

clean:
	@for a in $(shell ls build); do \
	if [ -d build/$$a ]; then \
	    echo "processing folder $$a"; \
	    $(MAKE) -C build/$$a clean; \
	fi; \
	done;
	@echo "Clean done!"
