#! /usr/bin/python

import serial
import sys
import time
import re

def execute_file(f,port,tintchar=0.01,eol=0xd):
	for cmd in f:
		cmd = cmd[:-1]
		print "Executing "+cmd+"..."
		for c in cmd:
			port.write(c)
			time.sleep(tintchar)
			
		port.write(chr(eol))
		print "Press any key to continue..."
		wait = raw_input()

def execute_program(dev,interactive=False,tintchar=0.01,eol=0xd,fcmd="cmd.txt",cmd_exit="exit",cmd_load="load",pbaudrate=115200,pparity=serial.PARITY_NONE,pstopbits=serial.STOPBITS_ONE,pbytesize=serial.EIGHTBITS,pxonxoff=False,prtscts=False,pdsrdtr=False):
	if interactive == False:
		f = open(fcmd)
		port = serial.Serial(dev, baudrate=pbaudrate,parity=pparity,stopbits=pstopbits,bytesize=pbytesize,xonxoff=pxonxoff,rtscts=prtscts,dsrdtr=pdsrdtr)
		
		execute_file(f,port,tintchar=tintchar,eol=eol)
		
		port.close()
		f.close()
	else:	
		port = serial.Serial(dev, baudrate=pbaudrate,parity=pparity,stopbits=pstopbits,bytesize=pbytesize,xonxoff=pxonxoff,rtscts=prtscts,dsrdtr=pdsrdtr)
		end = False
		
		while not end:
			cmd = raw_input("wrc# ")
			
			if cmd == cmd_exit:
				break
			elif cmd == cmd_load:
				fload = raw_input("File: ")
				f2 = open(fload)
				execute_file(f2,port,tintchar=tintchar,eol=eol)
				f2.close()
			else:
				for c in cmd:
					port.write(c)
					time.sleep(tintchar)
					
				port.write(chr(eol))
				print "Press any key to continue..."
				wait = raw_input()
				
		port.close()
			
print
print "============================================================================"
print "\t\t White-Rabbit Executer v1.0  \t\t"
print "============================================================================"


if len(sys.argv) != 2 and len(sys.argv) != 3:
	print "ERROR: use ",sys.argv[0]," <USB device>"
	print "ERROR: use ",sys.argv[0]," <USB device> <program_file>"
	print "Made by: Miguel Jimenez Lopez <klyone@ugr.es>, UGR"
	print
	sys.exit(-1)
	
dev = sys.argv[1]

if len(sys.argv) == 2:
	interactive_b = True
	fc = None
else:
	interactive_b = False
	fc = sys.argv[2]
	
execute_program(dev,interactive=interactive_b,fcmd=fc)
