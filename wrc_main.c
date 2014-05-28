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
#include <w1.h>
#include "syscon.h"
#include "uart.h"
#include "endpoint.h"
#include "minic.h"
#include "pps_gen.h"
#include "ptpd_netif.h"
#include "i2c.h"
#include "eeprom.h"
#include "softpll_ng.h"
#include "onewire.h"
#include "pps_gen.h"
#include "shell.h"
#include "lib/ipv4.h"
#include "rxts_calibrator.h"

#include "wrc_ptp.h"

int wrc_ui_mode = UI_SHELL_MODE;
int wrc_ui_refperiod = TICS_PER_SECOND; /* 1 sec */
int wrc_phase_tracking = 1;

///////////////////////////////////
//Calibration data (from EEPROM if available)
int32_t sfp_alpha = 73622176;	//default values if could not read EEPROM
int32_t sfp_deltaTx = 46407;
int32_t sfp_deltaRx = 167843;
uint32_t cal_phase_transition = 2389;

uint8_t vc709_init_i2c_switch(uint8_t i2cif) {
	uint8_t rcode = 0;
	uint8_t sw1_addr = 0x74;
	uint8_t sw1_read = (sw1_addr<<1) | 0x01;
	uint8_t sw1_write = (sw1_addr<<1);
	uint8_t sw2_addr = 0x75;
	uint8_t sw2_read = (sw2_addr<<1) | 0x01;
	uint8_t sw2_write = (sw2_addr<<1);
	uint8_t sw1_config = 0x18;
	uint8_t sw2_config = 0x01;
	unsigned char d;
	
	mi2c_start(i2cif);
	
	mprintf("READ -> 0x%x\n",sw1_read);
	if ((mi2c_put_byte(i2cif, sw1_read)) < 0) {
		mi2c_stop(i2cif);
		mprintf("ERROR 1...\n");
		return -1;
	}
	
	mi2c_get_byte(i2cif,&d,1);
	
	mi2c_stop(i2cif);
	
	mprintf("I2C SWITCH1 INIT -> 0x%x!!\n",d);

	mi2c_start(i2cif);
	
	mprintf("WRITE -> 0x%x\n",sw1_write);
	if ((mi2c_put_byte(i2cif, sw1_write)) < 0) {
		mi2c_stop(i2cif);
		mprintf("ERROR 2...\n");
		return -1;
	}
	
	mprintf("CONFIG -> 0x%x\n",sw1_config);
	if((rcode = mi2c_put_byte(i2cif, sw1_config)) < 0) {
		mi2c_stop(i2cif);
		mprintf("ERROR 3...\n");
		return -1;
	}
	
	mi2c_stop(i2cif);
	
	mi2c_start(i2cif);

	mprintf("READ -> 0x%x\n",sw1_read);
	if ((mi2c_put_byte(i2cif, sw1_read)) < 0) {
		mi2c_stop(i2cif);
		mprintf("ERROR 4...\n");
		return -1;
	}
	
	mi2c_get_byte(i2cif,&d,1);
	
	mi2c_stop(i2cif);
	
	mprintf("I2C SWITCH1 CONFIGURED -> 0x%x!!\n",d);
	
	
	// To enable SFP I2C bus
	
	mi2c_start(i2cif);
	
	mprintf("READ -> 0x%x\n",sw2_read);
	if ((mi2c_put_byte(i2cif, sw2_read) ) < 0) {
		mi2c_stop(i2cif);
		mprintf("ERROR 5...\n");
		return -1;
	}
	
	mi2c_get_byte(i2cif,&d,1);
	
	mi2c_stop(i2cif);
	
	mprintf("I2C SWITCH2 INIT -> 0x%x!!\n",d);

	mi2c_start(i2cif);
	
	mprintf("WRITE -> 0x%x\n",sw2_write);
	if (mi2c_put_byte(i2cif, sw2_write) < 0) {
		mi2c_stop(i2cif);
		mprintf("ERROR 6...\n");
		return -1;
	}
	
	mprintf("CONFIG -> 0x%x\n",sw2_config);
	if((rcode = mi2c_put_byte(i2cif, sw2_config)) < 0) {
		mi2c_stop(i2cif);
		mprintf("ERROR 7...\n");
		return -1;
	}
	
	mi2c_stop(i2cif);
	
	mi2c_start(i2cif);

	mprintf("READ -> 0x%x\n",sw2_read);
	if ((mi2c_put_byte(i2cif, sw2_read)) < 0) {
		mi2c_stop(i2cif);
		mprintf("ERROR 8...\n");
		return -1;
	}
	
	mi2c_get_byte(i2cif,&d,1);
	
	mi2c_stop(i2cif);
	
	mprintf("I2C SWITCH2 CONFIGURED -> 0x%x!!\n",d);
	
	return rcode;
}

static void wrc_initialize()
{
	uint8_t mac_addr[6];

	sdb_find_devices();
	uart_init_sw();
	uart_init_hw();

	mprintf("WR Core: starting up...\n");

	timer_init(1);
	wrpc_w1_init();
	wrpc_w1_bus.detail = ONEWIRE_PORT;
	w1_scan_bus(&wrpc_w1_bus);

	/*initialize I2C bus*/
	mi2c_init(WRPC_FMC_I2C);
	vc709_init_i2c_switch(WRPC_FMC_I2C); // INIT SW1 and SW2 of VC709
	/*check if EEPROM is onboard*/ 
	eeprom_present(WRPC_FMC_I2C, FMC_EEPROM_ADR);

	mac_addr[0] = 0x08;	//
	mac_addr[1] = 0x00;	// CERN OUI
	mac_addr[2] = 0x30;	//

	if (get_persistent_mac(ONEWIRE_PORT, mac_addr) == -1) {
		mprintf("Unable to determine MAC address\n");
		mac_addr[0] = 0x11;	//
		mac_addr[1] = 0x22;	//
		mac_addr[2] = 0x33;	// fallback MAC if get_persistent_mac fails
		mac_addr[3] = 0x44;	//
		mac_addr[4] = 0x55;	//
		mac_addr[5] = 0x66;	//
	}

	TRACE_DEV("Local MAC address: %x:%x:%x:%x:%x:%x\n", mac_addr[0],
		  mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4],
		  mac_addr[5]);

	ep_init(mac_addr);
	ep_enable(1, 1);

	minic_init();
	shw_pps_gen_init();
	wrc_ptp_init();
	//try reading t24 phase transition from EEPROM
	calib_t24p(WRC_MODE_MASTER, &cal_phase_transition);

#ifdef CONFIG_ETHERBONE
	ipv4_init("wru1");
	arp_init("wru1");
#endif
}

#define LINK_WENT_UP 1
#define LINK_WENT_DOWN 2
#define LINK_UP 3
#define LINK_DOWN 4

static int wrc_check_link()
{
	static int prev_link_state = -1;
	int link_state = ep_link_up(NULL);
	int rv = 0;

	if (!prev_link_state && link_state) {
		TRACE_DEV("Link up.\n");
		gpio_out(GPIO_LED_LINK, 1);
		rv = LINK_WENT_UP;
	} else if (prev_link_state && !link_state) {
		TRACE_DEV("Link down.\n");
		gpio_out(GPIO_LED_LINK, 0);
		rv = LINK_WENT_DOWN;
	} else
		rv = (link_state ? LINK_UP : LINK_DOWN);
	prev_link_state = link_state;

	return rv;
}

void wrc_debug_printf(int subsys, const char *fmt, ...)
{
	va_list ap;

	if (wrc_ui_mode)
		return;

	va_start(ap, fmt);

	if (subsys & (1 << 5) /* was: TRACE_SERVO -- see commit message */)
		vprintf(fmt, ap);

	va_end(ap);
}

int wrc_man_phase = 0;

static void ui_update()
{

	if (wrc_ui_mode == UI_GUI_MODE) {
		wrc_mon_gui();
		if (uart_read_byte() == 27 || wrc_ui_refperiod == 0) {
			shell_init();
			wrc_ui_mode = UI_SHELL_MODE;
		}
	} else if (wrc_ui_mode == UI_STAT_MODE) {
		wrc_log_stats(0);
		if (uart_read_byte() == 27 || wrc_ui_refperiod == 0) {
			shell_init();
			wrc_ui_mode = UI_SHELL_MODE;
		}
	} else
		shell_interactive();

}

extern uint32_t _endram;
extern uint32_t _fstack;
#define ENDRAM_MAGIC 0xbadc0ffe

static void check_stack(void)
{
	while (_endram != ENDRAM_MAGIC) {
		mprintf("Stack overflow!\n");
		timer_delay_ms(1000);
	}
}

#ifdef CONFIG_CHECK_RESET

static void check_reset(void)
{
	extern void _reset_handler(void); /* user to reset again */
	/* static variables to preserve stack (for dumping it) */
	static uint32_t *p, *save;

	/* _endram is set to ENDRAM_MAGIC after calling this function */
	if (_endram != ENDRAM_MAGIC)
		return;

	/* Before calling anything, find the beginning of the stack */
	p = &_endram + 1;
	while (!*p)
		p++;
	p = (void *)((unsigned long)p & ~0xf); /* align */

	/* Copy it to the beginning of the stack, then reset pointers */
	save = &_endram;
	while (p <= &_fstack)
		*save++ = *p++;
	p -= (save - &_endram);
	save = &_endram;

	/* Ok, now init the devices so we can printf and delay */
	sdb_find_devices();
	uart_init_sw();
	uart_init_hw();
	timer_init(1);

	pp_printf("\nWarning: the CPU was reset\nStack trace:\n");
	while (p < &_fstack) {
		pp_printf("%08x: %08x %08x %08x %08x\n",
			  (int)p, save[0], save[1], save[2], save[3]);
		p += 4;
		save += 4;
	}
	pp_printf("Rebooting in 1 second\n\n\n");
	timer_delay_ms(1000);

	/* Zero the stack and start over (so we dump correctly next time) */
	for (p = &_endram; p < &_fstack; p++)
		*p = 0;
	_endram = 0;
	_reset_handler();
}

# else /* no CONFIG_CHECK_RESET */

static void check_reset(void) {}

#endif


int main(void)
{
	check_reset();
	wrc_ui_mode = UI_SHELL_MODE;
	_endram = ENDRAM_MAGIC;

	wrc_initialize();
	usleep_init();
	shell_init();

	wrc_ptp_set_mode(WRC_MODE_SLAVE);
	wrc_ptp_start();

	//try to read and execute init script from EEPROM
	shell_boot_script();

	for (;;) {
		int l_status = wrc_check_link();

		switch (l_status) {
#ifdef CONFIG_ETHERBONE
		case LINK_WENT_UP:
			needIP = 1;
			break;
#endif

		case LINK_UP:
			update_rx_queues();
#ifdef CONFIG_ETHERBONE
			ipv4_poll();
			arp_poll();
#endif
			break;

		case LINK_WENT_DOWN:
			if (wrc_ptp_get_mode() == WRC_MODE_SLAVE) {
				spll_init(SPLL_MODE_FREE_RUNNING_MASTER, 0, 1);
				shw_pps_gen_enable_output(0);
			}
			break;
		}

		ui_update();
		wrc_ptp_update();
		spll_update_aux_clocks();
		check_stack();
	}
}
