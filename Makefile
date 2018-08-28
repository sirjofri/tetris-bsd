# This Makefile is provided _as is_. It should work on most linux systems.

include config.mk

HIDEOUT= >/dev/null 2>/dev/null
FLAGS= -D_PATH_SCOREFILE=\"$(SCOREFILE)\"
LIBS= -lncursesw
RELRO= -Wl,-z,relro,-z,now
CFLAGS= -g -O2 $(RELRO) -D_GNU_SOURCE -D_DEFAULT_SOURCE $(LIBS) $(FLAGS)
CC= gcc

all: config tetris2

install: $(BINDIR)/tetris2 $(MANDIR)/man6/tetris2.6.gz $(SCOREFILE)
	@echo " → Installed package"

$(BINDIR)/tetris2: tetris2
	@cp $< $@

$(MANDIR)/man6/tetris2.6.gz: tetris.6.in
	@sed -e 's%@tetris_scorefile@%$(SCOREFILE)%g' $< | gzip > $@

$(SCOREFILE):
	@touch $(SCOREFILE)
	@chmod 666 $(SCOREFILE)

uninstall: remove
remove:
	@rm $(MANDIR)/man6/tetris2.6.gz $(BINDIR)/tetris2

clean:
	- @rm tetris.o scores.o screen.o shapes.o input.o tetris2 $(HIDEOUT)
	- @rm -Rf pkg src $(HIDEOUT)

package: pkgdirs $(PACKAGE)$(BINDIR)/tetris2 $(PACKAGE)$(MANDIR)/man6/tetris2.6.gz

pkgdirs:
	@mkdir -p $(PACKAGE)$(BINDIR)
	@mkdir -p $(PACKAGE)$(MANDIR)/man6

$(PACKAGE)$(BINDIR)/tetris2: tetris2
	@cp $< $@

$(PACKAGE)$(MANDIR)/man6/tetris2.6.gz: tetris.6.in
	@sed -e 's%@tetris_scorefile@%$(SCOREFILE)%g' $< | gzip > $@

.PHONY: config
config:

tetris2: tetris.o scores.o screen.o shapes.o input.o
	@$(CC) $(CFLAGS) -o $@ $^ $(HIDEOUT)
	@echo " → $@ compiled"

%.o: %.c %.h
	@$(CC) $(CFLAGS) -o $@ -c $< $(HIDEOUT)
	@echo " → $@ compiled"
