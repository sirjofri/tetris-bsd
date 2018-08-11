/*	$NetBSD: tetris.c,v 1.17 2004/01/27 20:30:30 jsm Exp $	*/

/*-
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek and Darren F. Provine.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)tetris.c	8.1 (Berkeley) 5/31/93
 */

#include <sys/cdefs.h>
#ifndef lint
//__COPYRIGHT("@(#) Copyright (c) 1992, 1993\n\
	The Regents of the University of California.  All rights reserved.\n");
#endif /* not lint */

/*
 * Tetris (or however it is spelled).
 */

#include <sys/time.h>

#include <err.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "input.h"
#include "scores.h"
#include "screen.h"
#include "tetris.h"

cell	board[B_SIZE];		/* 1 => occupied, 0 => empty */

int	Rows, Cols;		/* current screen size */

const struct shape *curshape;
const struct shape *nextshape;

const struct shape *holdshape;	/* shape for hold function */
const struct shape *holdbuf;	/* buf shape for hold function */
int hold;			/* 0 => empty */
int switched;			/* 1 => the shape has been switched */

#define bagsize 7
const struct shape *bag[bagsize];
int bagptr = bagsize;
int usebag;

long	fallrate;		/* less than 1 million; smaller => faster */

int	score;			/* the obvious thing */
gid_t	gid, egid;

char	key_msg[100];
int	showpreview;

int	fastelide;		/* 1 => elide all full rows at once */
int	useghost;		/* 1 => draw ghost tile */
int	usehold;		/* 1 => enable hold support */

static	void	elide(void);
static	void	setup_board(void);
	int	main(int, char **);
	void	onintr(int) __attribute__((__noreturn__));
	void	usage(void) __attribute__((__noreturn__));

/*
 * Set up the initial board.  The bottom display row is completely set,
 * along with another (hidden) row underneath that.  Also, the left and
 * right edges are set.
 */
static void
setup_board()
{
	int i;
	cell *p;

	p = board;
	for (i = B_SIZE; i; i--)
		*p++ = i <= (2 * B_COLS) || (i % B_COLS) < 2;
}

/*
 * Elide any full active rows.
 */
static void
elide()
{
	int i, j, base;
	cell *p;

	for (i = A_FIRST; i < A_LAST; i++) {
		base = i * B_COLS + 1;
		p = &board[base];
		for (j = B_COLS - 2; *p++ != 0;) {
			if (--j <= 0) {
				/* this row is to be elided */
				memset(&board[base], 0, B_COLS - 2);
				if(!fastelide)
				{
					scr_update();
					tsleep();
				}
				while (--base != 0)
					board[base + B_COLS] = board[base];
				if(!fastelide)
				{
					scr_update();
					tsleep();
				}
				break;
			}
		}
	}
}

int
main(argc, argv)
	int argc;
	char *argv[];
{
	int pos, ghostpos, c;
	int ghost_placed = 0;
	const char *keys;
	int level = 2;
	char key_write[6][10];
	int ch, i, j;
	int fd;

	gid = getgid();
	egid = getegid();
	setegid(gid);

	fd = open("/dev/null", O_RDONLY);
	if (fd < 3)
		exit(1);
	close(fd);

	keys = "jkl pqd";
	fastelide = 0;
	usebag = 1;
	useghost = 0;
	usehold = 1;
	hold = 0;
	switched = 0;

	while ((ch = getopt(argc, argv, "k:l:bfghps")) != -1)
		switch(ch) {
		case 'k':
			if (strlen(keys = optarg) != 6)
				usage();
			break;
		case 'l':
			level = atoi(optarg);
			if (level < MINLEVEL || level > MAXLEVEL) {
				errx(1, "level must be from %d to %d",
				     MINLEVEL, MAXLEVEL);
			}
			break;
		case 'p':
			showpreview = 1;
			break;
		case 's':
			showscores(0);
			exit(0);
			break;
		case 'f':
			fastelide = 1;
			break;
		case 'b':
			usebag = 0;
			break;
		case 'g':
			useghost = 1;
			break;
		case 'h':
			usehold = 0;
			break;
		case '?':
		default:
			usage();
		}

	argc -= optind;
	argv += optind;

	if (argc)
		usage();

	fallrate = 1000000 / level;

	for (i = 0; i <= 6; i++) {
		for (j = i+1; j <= 6; j++) {
			if (keys[i] == keys[j]) {
				errx(1, "duplicate command keys specified.");
			}
		}
		if (keys[i] == ' ')
			strcpy(key_write[i], "<space>");
		else {
			key_write[i][0] = keys[i];
			key_write[i][1] = '\0';
		}
	}

	sprintf(key_msg,
"%s - left   %s - rotate   %s - right  %s - hold   %s - drop   %s - pause   %s - quit",
		key_write[0], key_write[1], key_write[2], key_write[6], key_write[3],
		key_write[4], key_write[5]);

	(void)signal(SIGINT, onintr);
	scr_init();
	setup_board();

	srandom(getpid());
	scr_set();

	pos = A_FIRST*B_COLS + (B_COLS/2)-1;
	nextshape = randshape();
	curshape = randshape();

	/* calculate initial ghost position */
	ghostpos = pos; // not in (if) to prevent maybe uninitialized warning
	if (useghost) {
		while (fits_in(curshape, ghostpos + B_COLS))
			ghostpos += B_COLS;
	}

	scr_msg(key_msg, 1);

	for (;;) {
		/*
		 * Do not place a ghost block directly after dropping the block!
		 *
		 * Place the ghost before placing the actual tile to achieve a nice
		 * overlap.
		 */
		if (useghost && !ghost_placed)
			place(curshape, ghostpos, 2);
		ghost_placed = 0;

		place(curshape, pos, 1);

		scr_update();
		place(curshape, pos, 0);
		if (useghost)
			place(curshape, ghostpos, 0);

		c = tgetchar();
		if (c < 0) {
			/*
			 * Timeout.  Move down if possible.
			 */
			if (fits_in(curshape, pos + B_COLS)) {
				pos += B_COLS;

				/* calculate ghost position */
				if (useghost) {
					ghostpos = pos;
					while (fits_in(curshape, ghostpos + B_COLS))
						ghostpos += B_COLS;
				}

				continue;
			}

			/*
			 * Put up the current shape `permanently',
			 * bump score, and elide any full rows.
			 */
			place(curshape, pos, 1);
			score++;
			elide();
			switched = 0;

			/*
			 * Choose a new shape.  If it does not fit,
			 * the game is over.
			 */
			curshape = nextshape;
			nextshape = randshape();
			pos = A_FIRST*B_COLS + (B_COLS/2)-1;

			if (useghost) {
				/* calculate ghost position */
				ghostpos = pos;
				while (fits_in(curshape, ghostpos + B_COLS))
					ghostpos += B_COLS;
			}

			if (!fits_in(curshape, pos))
				break;
			continue;
		}

		/*
		 * Handle command keys.
		 */
		if (c == keys[5]) {
			/* quit */
			break;
		}
		if (c == keys[4]) {
			static char msg[] =
			    "paused - press RETURN to continue";

			if (useghost)
				place(curshape, ghostpos, 2);
			place(curshape, pos, 1);
			do {
				scr_update();
				scr_msg(key_msg, 0);
				scr_msg(msg, 1);
				(void) fflush(stdout);
			} while (rwait((struct timeval *)NULL) == -1);
			scr_msg(msg, 0);
			scr_msg(key_msg, 1);
			place(curshape, pos, 0);
			continue;
		}
		if (c == keys[0]) {
			/* move left */
			if (fits_in(curshape, pos - 1))
				pos--;

			/* calculate ghost position */
			if (useghost) {
				ghostpos = pos;
				while (fits_in(curshape, ghostpos + B_COLS))
					ghostpos += B_COLS;
			}

			continue;
		}
		if (c == keys[1]) {
			/* turn */
			const struct shape *new = &shapes[curshape->rot];

			if (!fits_in(new, pos)) { /* move tile if necessary */
				if (fits_in(new, pos+1)) {
					pos++;
				} else
				if (fits_in(new, pos-1)) {
					pos--;
				} else
				if (fits_in(new, pos-2)) { /* long tile at right border */
					pos -= 2;
				}
			}
			curshape = new;

			/* calculate ghost position */
			if (useghost) {
				ghostpos = pos;
				while (fits_in(curshape, ghostpos + B_COLS))
					ghostpos += B_COLS;
			}

			continue;
		}
		if (usehold && !switched && c == keys[6]) {
			/* hold key */
			pos = A_FIRST*B_COLS + (B_COLS/2)-1;

			if (useghost) { /* hide ghost tile */
				place(curshape, ghostpos, 0);
			}

			/* if there is something in the buffer */
			if (hold) {
				holdbuf = curshape;
				curshape = holdshape;
				holdshape = holdbuf;
			} else { /* aargh, I don't like else */
				holdshape = curshape;
				curshape = nextshape;
				hold = 1; /* now we have a shape in the buffer*/
			}
			/* we switched the shape, so we must wait for the next new shape */
			switched = 1;
			if (useghost) {
				ghostpos = pos;
				/* calculate new ghost position */
				while (fits_in(curshape, ghostpos + B_COLS))
					ghostpos += B_COLS;
				place(curshape, ghostpos, 2);
			}
			continue;
		}
		if (c == keys[2]) {
			/* move right */
			if (fits_in(curshape, pos + 1))
				pos++;

			/* calculate ghost position */
			if (useghost) {
				ghostpos = pos;
				while (fits_in(curshape, ghostpos + B_COLS))
					ghostpos += B_COLS;
			}

			continue;
		}
		if (c == keys[3]) {
			/* move to bottom */

			if (useghost)
				ghost_placed = 1; /* Do not place ghost block after this! */

			while (fits_in(curshape, pos + B_COLS)) {
				pos += B_COLS;
				score++;
			}
			switched = 0;
			continue;
		}
		if (c == '\f') {
			scr_clear();
			scr_msg(key_msg, 1);
		}
	}

	scr_clear();
	scr_end();

	(void)printf("Your score:  %d point%s  x  level %d  =  %d\n",
	    score, score == 1 ? "" : "s", level, score * level);
	savescore(level);

	printf("\nHit RETURN to see high scores, ^C to skip.\n");

	while ((i = getchar()) != '\n')
		if (i == EOF)
			break;

	showscores(level);

	exit(0);
}

void
genbag(void)
{
	for(int i=0; i<7; i++)
	{
		const struct shape *id = 0;
		genbag_loop: while(id == 0)
		{
			id = &shapes[random() % 7];
			for(int j=0; j<i; j++)
			{
				if(bag[j] == id)
				{
					id = 0;
					goto genbag_loop;
				}
			}
		}
		bag[i] = id;
	}
}

const struct shape
*randshape(void)
{
	if(!usebag)
		return &shapes[random() % 7];

	if(bagptr >= bagsize)
	{
		bagptr = 0;
		genbag();
	}
	return bag[bagptr++];
}

void
onintr(signo)
	int signo __attribute__((__unused__));
{
	scr_clear();
	scr_end();
	exit(0);
}

void
usage()
{
	(void)fprintf(stderr, "usage: tetris-bsd [-bfghps] [-k keys] [-l level]\n");
	exit(1);
}
