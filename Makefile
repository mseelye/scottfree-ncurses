#
# ScottFree-ncurses
# Reworking of "ScottFree 1.14b" with ncurses
#
# Scott Free, A Scott Adams game driver in C.  
# Release 1.14b (PC), (c) 1993,1994,1995 Swansea University Computer Society.  
# Port to Commodore 64 as ScottFree64 Release 0.9, (c) 2020 Mark Seelye  
# Distributed under the GNU software license  
# (C) 2020 - Mark Seelye - mseelye@yahoo.com - 2020-06-20

VERSION = v1.0.0

# Determine what OS this is running on and adjust
OSUNAME := $(shell uname)
ifeq "$(OSUNAME)" "Darwin"
	ECHO := echo
else ifeq "$(OSUNAME)" "Linux"
	ECHO := echo -e
	CC65_HOME := /usr
else # Windows
	ECHO := echo -e
endif

DIST_SED:=s/{VERSION}/$(VERSION)/g;

CC=gcc
CFLAGS = -Wall
LIBS=-lncurses
MKDIR_P = mkdir -p
OUT_DIR = out
SRC_DIR = src

all: directories scottfree-ncurses

scottfree-ncurses: $(SRC_DIR)/scottfree-ncurses.c $(SRC_DIR)/scottfree.h
	@$(ECHO) "*** Building $<"
	@cat $< | sed -e "$(DIST_SED)" > $(SRC_DIR)/tmp-$(<F)
	$(CC) -o out/$@ $(SRC_DIR)/tmp-$(<F) $(CFLAGS) $(LIBS)
	@$(ECHO) "*** Building complete\n"
	@rm $(SRC_DIR)/tmp-$(<F)

.PHONY: clean
clean:
	rm -rf ${OUT_DIR}

.PHONY: directories
directories: ${OUT_DIR}

${OUT_DIR}:
	${MKDIR_P} ${OUT_DIR}