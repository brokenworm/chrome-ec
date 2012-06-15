/* Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* Port 80 module for Chrome EC */

#include "board.h"
#include "console.h"
#include "port80.h"
#include "util.h"

#define CPRINTF(format, args...) cprintf(CC_PORT80, format, ## args)

#define HISTORY_LEN 16

static uint8_t history[HISTORY_LEN];
static int writes;  /* Number of port 80 writes so far */
static int scroll;


void port_80_write(int data)
{
	/*
	 * Note that this currently prints from inside the LPC interrupt
	 * itself.  Probably not worth the system overhead to buffer the data
	 * and print it from a task, because we're printing a small amount of
	 * data and cprintf() doesn't block.
	 */
	CPRINTF("%c[%T Port 80: 0x%02x]", scroll ? '\n' : '\r', data);

	history[writes % ARRAY_SIZE(history)] = data;
	writes++;
}

/*****************************************************************************/
/* Console commands */

static int command_port80(int argc, char **argv)
{
	int head, tail;
	int i;

	/*
	 * 'port80 scroll' toggles whether port 80 output begins with a newline
	 * (scrolling) or CR (non-scrolling).
	 */
	if (argc > 1 && !strcasecmp(argv[1], "scroll")) {
		scroll = !scroll;
		ccprintf("scroll %sabled\n", scroll ? "en" : "dis");
		return EC_SUCCESS;
	}

	/*
	 * Print the port 80 writes so far, clipped to the length of our
	 * history buffer.
	 *
	 * Technically, if a port 80 write comes in while we're printing this,
	 * we could print an incorrect history.  Probably not worth the
	 * complexity to work around that.
	 */
	head = writes;
	if (head > ARRAY_SIZE(history))
		tail = head - ARRAY_SIZE(history);
	else
		tail = 0;

	for (i = tail; i < head; i++)
		ccprintf(" %02x", history[i % ARRAY_SIZE(history)]);
	ccputs(" <--new\n");
	return EC_SUCCESS;
}
DECLARE_CONSOLE_COMMAND(port80, command_port80,
			"[scroll]",
			"Print port80 writes or toggle port80 scrolling",
			NULL);
