/*
 * This work is part of the White Rabbit project
 *
 * Copyright (C) 2014 UGR (www.ugr.es)
 * Author: Miguel Jimenez Lopez <klyone@ugr.es>
 *
 * Released according to the GNU GPL, version 2 or any later version.
 */

#include <string.h>
#include <wrc.h>
#include "shell.h"

static int cmd_write(const char *args[])
{
int addr;
int val;
int * addr_w;

	if (args[0] && args[1]) {
		fromhex(args[0],&addr);
		fromhex(args[1],&val);
		
		if (!args[2]) {
			addr_w = (int *) BASE_WRPC_RAM + addr;
		}
		else {
			addr_w = (int *) 0x0 + addr;
		}
		
		*addr_w = val;
	}
	else {
		return -1;
	}

	return 0;
}

DEFINE_WRC_COMMAND(write) = {
	.name = "write",
	.exec = cmd_write,
};
