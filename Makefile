# This Makefile is provided _as is_. It should work on most linux systems.

include config.mk

HIDEOUT= >/dev/null 2>/dev/null
FLAGS= -D_PATH_SCOREFILE=\"$(SCOREFILE)\"
LIBS= -lncursesw
CFLAGS= -g -O2 -D_GNU_SOURCE -D_DEFAULT_SOURCE $(LIBS) $(FLAGS)
CC= gcc

all: config tetris

install: $(BINDIR)/tetris $(MANDIR)/man6/tetris.6.gz
	@echo " → Installed package"

$(BINDIR)/tetris: tetris
	@cp $< $@

$(MANDIR)/man6/tetris.6.gz: tetris.6.in
	@sed -e 's/@tetris_scorefile@/$(SCOREFILE)/g' $< | gzip > $@

uninstall: remove
remove:
	@rm $(MANDIR)/man6/tetris.6.gz $(BINDIR)/tetris

clean:
	@rm tetris.o scores.o screen.o shapes.o input.o tetris

.PHONY: config
config:

tetris: tetris.o scores.o screen.o shapes.o input.o
	@$(CC) $(CFLAGS) -o $@ $^ $(HIDEOUT)
	@echo " → $@ compiled"

%.o: %.c %.h
	@$(CC) $(CFLAGS) -o $@ -c $< $(HIDEOUT)
	@echo " → $@ compiled"
