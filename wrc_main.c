/*
 * This work is part of the White Rabbit project
 *
 * Copyright (C) 2011,2012 CERN (www.cern.ch)
 * Author: Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
 * Author: Grzegorz Daniluk <grzegorz.daniluk@cern.ch>
 *
 * Released according to the GNU GPL, version 2 or any later version.
 */
#include <stdio.h>
#include <inttypes.h>

#include <stdarg.h>

#include <wrc.h>
#include "uart.h"
#include "shell.h"

void _irq_entry() {}

static void obp_initialize()
{

	sdb_find_devices();
	uart_init_sw();
	uart_init_hw();

	mprintf("OBP: starting up...\n");

}

static void print_found_devices() {
	mprintf("MINIC: 0x%x\n",(unsigned int)BASE_MINIC);
	mprintf("EP: 0x%x\n",(unsigned int)BASE_EP);
	mprintf("SOFTPLL: 0x%x\n",(unsigned int)BASE_SOFTPLL);
	mprintf("PPS_GEN: 0x%x\n",(unsigned int)BASE_PPS_GEN);
	mprintf("SYSCON: 0x%x\n",(unsigned int)BASE_SYSCON);
	mprintf("UART: 0x%x\n",(unsigned int)BASE_UART);
	mprintf("ONEWIRE: 0x%x\n",(unsigned int)BASE_ONEWIRE);
	mprintf("ETHERBONE_CFG: 0x%x\n",(unsigned int)BASE_ETHERBONE_CFG);
	mprintf("WRPC_RAM: 0x%x\n",(unsigned int)BASE_WRPC_RAM);
}

int main(void)
{

obp_initialize();

mprintf("OBP: Found devices: \n");

print_found_devices();

shell_init();

mprintf("OBP: shell initialized! \n");

for(;;) {
	shell_interactive();
}

}

