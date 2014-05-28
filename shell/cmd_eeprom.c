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
#include "eeprom.h"
#include "syscon.h"
#include "i2c.h"

static int cmd_eeprom(const char *args[])
{

uint8_t d;
uint8_t z = 0;
uint16_t i;
const uint16_t EEPROM_SIZE=1024;
uint32_t addr;
int ret;
uint8_t d2;
char mode;

	if (!args[0] || !strcasecmp(args[0], "show")) {

		for(i = 0 ; i < EEPROM_SIZE ; i++) {
			ret = eeprom_read(WRPC_FMC_I2C,FMC_EEPROM_ADR,i,&d,1);
			
			if(ret != 1) {
				mprintf("ERROR 0x%x\n",i);
				return -1;
			}
			else
				mprintf("[0x%x]: 0x%x\n",i,d);
		}
		
	} else if (!strcasecmp(args[0], "erase")) {
			for(i = 0 ; i < EEPROM_SIZE ; i++) {
				ret = eeprom_write(WRPC_FMC_I2C,FMC_EEPROM_ADR,i,&z,1);
				
				if(ret != 1) {
					mprintf("ERROR 0x%x\n",i);
					return -1;
				}
			}
			mprintf("OK\n");
			
	} else if (args[1] && !strcasecmp(args[0], "read")) {
				fromhex(args[1],(int *)&addr);		
				
				if(addr < EEPROM_SIZE)		
					ret = eeprom_read(WRPC_FMC_I2C,FMC_EEPROM_ADR,addr,&d,1);
				else {
					mprintf("ERROR\n");
					return -1;
				}
				
				if(ret != 1)
					mprintf("ERROR\n");
				else
					mprintf("[0x%x]: 0x%x\n",addr,d);
					
	} else if (args[1] && args[2] && !strcasecmp(args[0], "cmp")) {
				
				fromhex(args[1],(int *)&d2);
				mode = (args[2][0]);
				
				mprintf("%s -> %x ; %s \n",args[1],d2,args[2]);		
				
				for(i = 0 ; i < EEPROM_SIZE ; i++) {
					ret = eeprom_read(WRPC_FMC_I2C,FMC_EEPROM_ADR,i,&d,1);	
				
					if(ret != 1) {
						mprintf("ERROR 0x%x\n",i);
						return -1;
					}
					else {
						if (mode == 'e') {
							if (d2 == d)
								mprintf("[0x%x]: 0x%x\n",i,d);
						}
						else {
							if (d2 != d)
								mprintf("[0x%x]: 0x%x\n",i,d);
						}
					}
				}
					
	} else if (args[1] && args[2] && !strcasecmp(args[0], "write")) {
				fromhex(args[1],(int *)&addr);
				fromhex(args[2],(int *)&d);
				
				if(addr < EEPROM_SIZE) {
					//d = d & 0xff;
					ret = eeprom_write(WRPC_FMC_I2C,FMC_EEPROM_ADR,addr,&d,1);
				}
				else {
					mprintf("ERROR\n");
					return -1;
				}
				
				if(ret != 1)
					mprintf("ERROR\n");
				else
					mprintf("OK\n");
	}

	return 0;
}

DEFINE_WRC_COMMAND(eeprom) = {
	.name = "eeprom",
	.exec = cmd_eeprom,
};
