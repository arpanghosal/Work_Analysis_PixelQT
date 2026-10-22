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


class TestRodSlave(TestBase):
    def _init__(self, rod):
        self.requires_is = True
        self.first_reading = True

    def execute(self):

	#Function for SourceID:
	d = ISInfoDictionary(get_daqpartition())
	IS_val = ISInfoDynAny()
	d.getValue('DF.ROS.Appl_PixelROS05.ReadoutModule0', IS_val)


	print("*********************************************************")
	print("ROD SLAVE REGISTERS CHECKED")
	print("*********************************************************")

	#Obtaining the ROD DUMP and saving as a csv
	rod=self.rod
	curr_path=os.getcwd()
	save_csv="$HCP/bin/dumpRodRegisters --rodName "+rod+" > "+curr_path+"/testlibrary/rod_dump.csv"
	output = subprocess.check_output(['bash','-c', save_csv])

#	rol_en=[]
#	for i in range(0,12,1):
#	  if (IS_val[4][i] == 1):
#	    rol_en.append(i)
#	  else:
#	    continue

	######################################################################
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

	#Function for Run Number:
	f = ISInfoDictionary(get_daqpartition())
	run_num = ISInfoDynAny()
	f.getValue('RunParams.RunParams', run_num)

	run=(hex(run_num[0]))

	#Function to distinguish between pixel and IBL:
	#for i in range(len(rol_en)):
	if((rol_en[0]==0)|(rol_en[0]==1)|(rol_en[0]==2)|(rol_en[0]==8)):
	  SourceID = (hex(IS_val[3][rol_en[0]]))
	  CodeVersion = '0xfe13'
	  ReadoutTimeout = '0x801'
	  DataOverflowLimit = '0x801'
	  HeaderTrailerLimit = '0x801'
	  RodBusyLimit = '0x3c1'
	else:
	  SourceID = (hex(IS_val[3][rol_en[0]]))
	  CodeVersion = '0xfe14'
	  ReadoutTimeout = '0x500'
	  DataOverflowLimit = '0x320'
	  HeaderTrailerLimit = '0x700'
	  RodBusyLimit = '0x776'


	#Generating csv with expected values for ROD slave A & B registers
	### SLAVE A ###

	row1 = ['SlaveId_A', '0x0'] 
	row2 = ['CalibrationMode_A', '0x0']
	row3 = ['SourceId_A', SourceID] 
	row4 = ['RunNumber_A', run]
	row5 = ['CodeVersion_A', CodeVersion] 
	row6 = ['FmtReadoutTimeoutLimit_A', ReadoutTimeout]
	row7 = ['FmtDataOverflowLimit_A', DataOverflowLimit] 
	row8 = ['FmtHeaderTrailerLimit_A', HeaderTrailerLimit]
	row9 = ['FmtRodBusyLimit_A', RodBusyLimit] 
	row10 = ['FmtTrailerTimeoutLimit_A', '0xf50']
	row11 = ['Fmt0OccupancyReg_A', '0x0']
	row12 = ['Fmt1OccupancyReg_A', '0x0']
	row13 = ['Fmt2OccupancyReg_A', '0x0']
	row14 = ['Fmt3OccupancyReg_A', '0x0']
	with open('slave_a.csv', 'a') as csvFile:
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
	csvFile.close()

	### SLAVE B ###

	row1 = ['SlaveId_B', '0x1'] 
	row2 = ['CalibrationMode_B', '0x0']
	row3 = ['SourceId_B', SourceID] 
	row4 = ['RunNumber_B', run]
	row5 = ['CodeVersion_B', CodeVersion] 
	row6 = ['FmtReadoutTimeoutLimit_B', ReadoutTimeout]
	row7 = ['FmtDataOverflowLimit_B', DataOverflowLimit] 
	row8 = ['FmtHeaderTrailerLimit_B', HeaderTrailerLimit]
	row9 = ['FmtRodBusyLimit_B', RodBusyLimit] 
	row10 = ['FmtTrailerTimeoutLimit_B', '0xf50']
	row11 = ['Fmt0OccupancyReg_B', '0x0']
	row12 = ['Fmt1OccupancyReg_B', '0x0']
	row13 = ['Fmt2OccupancyReg_B', '0x0']
	row14 = ['Fmt3OccupancyReg_B', '0x0']
	with open('slave_b.csv', 'a') as csvFile:
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
	csvFile.close()

	
	py3_script="python3 testlibrary/py3_scripts/rod_slave_py3.py"
	output = subprocess.check_output(['env', '-i', 'bash', '--norc', '-c', py3_script])
	with open("output/Rod_Slave_out.txt", "a") as f:
	  print >> f, datetime.datetime.now()
	  print >> f, self.rod
	  print >> f, output







