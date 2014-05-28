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
#include "syscon.h"
#include "i2c.h"

static int i2c_d = WRPC_FMC_I2C;

static int cmd_i2c(const char *args[])
{

char dev_addr;
char reg_addr;
char val;

	if (args[1] && !strcasecmp(args[0], "read")) {

				fromhex(args[1],&dev_addr);		

				mi2c_start(i2c_d);	
				
				if(mi2c_put_byte(i2c_d, (dev_addr<<1) | 1) < 0) {
					return -1;
				}

				if (args[2]) {
						fromhex(args[2],&reg_addr);	
						
						if(mi2c_put_byte(i2c_d, reg_addr) < 0) {
							return -1;
						}
				}

				mi2c_get_byte(i2c_d, &val, 1);	

				mi2c_stop(i2c_d);

				if (args[2]) {
					mprintf("[DEV: 0x%x, REG: 0x%x]: 0x%x\n",dev_addr,reg_addr,val);
				}
				else {
					mprintf("[DEV: 0x%x]: 0x%x\n",dev_addr,val);
				}

	} else if (args[1] && args[2] && !strcasecmp(args[0], "write")) {
				fromhex(args[1],&dev_addr);		

				mi2c_start(i2c_d);	
				
				if(mi2c_put_byte(i2c_d, (dev_addr<<1)) < 0) {
					return -1;
				}

				if (args[3]) {
						fromhex(args[2],&reg_addr);	
						fromhex(args[3],&val);
						
						if(mi2c_put_byte(i2c_d, reg_addr) < 0) {
							return -1;
						}
				}
				else {					
						fromhex(args[2],&val);
				}

				if(mi2c_put_byte(i2c_d, val) < 0) {
							return -1;
				}
				
				char cmd_val = val;

				/**mi2c_repeat_start(i2c_d);

				char cmd_val = val;

				if(mi2c_put_byte(i2c_d, (dev_addr<<1) | 1) < 0) {
					return -1;
				}

				if (args[3]) {						
						if(mi2c_put_byte(i2c_d, reg_addr) < 0) {
							return -1;
						}
				}				

				mi2c_get_byte(i2c_d, &val, 1);
				
				if (args[3]) {
					mprintf("[DEV: 0x%x, REG: 0x%x, VAL: 0x%x]: 0x%x\n",dev_addr,reg_addr,cmd_val,val);
				}
				else {
					mprintf("[DEV: 0x%x, VAL: 0x%x]: 0x%x\n",dev_addr,cmd_val,val);
				} 
					**/

				mi2c_stop(i2c_d);
				
				mprintf("[WRITE OK!]\n");	
	}

	return 0;
}

DEFINE_WRC_COMMAND(i2c) = {
	.name = "i2c",
	.exec = cmd_i2c,
};
