from core.tools.partition import get_partition, get_daqpartition
from core.daqtest import TestBase, PASS, FAIL
from ispy import *
#from __future__ import print_function
from ipc import IPCPartition, getPartitions
import rcpy
import sys
import subprocess
import os
import csv
import datetime


class TestRodMaster(TestBase):
    def _init__(self, rod):
        self.requires_is = True
        self.first_reading = True

    def execute(self):
	d = ISInfoDictionary(get_daqpartition())
	IS_val = ISInfoDynAny()
	d.getValue('DF.ROS.Appl_PixelROS05.ReadoutModule0', IS_val)


	print("*********************************************************")
	print("ROD MASTER REGISTERS CHECKED")
	print("*********************************************************")

	#Obtaining the ROD DUMP and saving as a csv
	rod=self.rod
	curr_path=os.getcwd()
	save_csv="$HCP/bin/dumpRodRegisters --rodName "+rod+" > "+curr_path+"/testlibrary/rod_dump.csv"
	output = subprocess.check_output(['bash','-c', save_csv])


	#FRAGMENTS RECEIVED
	#Generating ROL enabled list for each ROD
	if (self.rod == 'ROD_C1_S7'):
	  rol_en=[0]
	elif (self.rod == 'ROD_C1_S17'):
	  rol_en=[1]
	elif (self.rod == 'ROD_C3_S8'):
	  rol_en=[2,3]
	elif (self.rod == 'ROD_C3_S11'):
	  rol_en=[4,5,6,7]
	elif (self.rod == 'ROD_C3_S15'):
	  rol_en=[8]
	else:
	  rol_en=[9,10]

	mostRecentL1ID = hex(IS_val[10][rol_en[0]])[2:]
	msb = hex(int('0x'+mostRecentL1ID[:-6], 16))
	#print(msb)
	lsb = hex(int('0x'+mostRecentL1ID[2:], 16))
	#print(lsb)


	################################################################
	#Generating csv with expected values for ROD master registers

	row1 = ['RrifCtrlReg', '0x88001523'] 
	row2 = ['SprCtrlReg', '0x878']
	row3 = ['EcrCntInReg', msb] 
	row4 = ['CalL1Id0Reg', lsb]
	row5 = ['CalL1Id1Reg', '0xffffff'] 
	row6 = ['CalPulseCount0', '0x0']
	row7 = ['CalPulseCount1', '0x0']
	row8 = ['BusytimeAfterEcrReg', '0x2328']
	row9 = ['VetoAfterEcrReg', '0x1fa4'] 
	row10 = ['ResetAfterEcrReg', '0x1f40']
	row11 = ['EcrVetoedAfterEcrReg', '0x0'] 
	row12 = ['L1AVetoedAfterEcrReg', '0x0']
	row13 = ['EcrDroppedReg', '0x0']
	row14 = ['BcrDroppedReg', '0x0'] 
	row15 = ['L1ADroppedReg', '0x0']
	with open('master.csv', 'a') as csvFile:
	    writer = csv.writer(csvFile)
	    writer.writerow(row1)
	    writer.writerow(row2)
	    writer.writerow(row3)
	    writer.writerow(row4)
	    writer.writerow(row5)
	    writer.writerow(row6)
	    writer.writerow(row7)
	    writer.writerow(row8)
	    writer.writerow(row9)
	    writer.writerow(row10)
	    writer.writerow(row11)
	    writer.writerow(row12)
	    writer.writerow(row13)
	    writer.writerow(row14)
	    writer.writerow(row15)
	csvFile.close()
	##############################################
	py3_script="python3 testlibrary/py3_scripts/rod_master_py3.py"
	try:
	    output = subprocess.check_output(['env', '-i', 'bash', '--norc', '-c', py3_script])
	except subprocess.CalledProcessError as e:
	    print e.output

	with open("output/Rod_Master_out.txt", "a") as f:
	  print >> f, datetime.datetime.now()
	  print >> f, self.rod
	  print >> f, output










