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


class TestBocBcf(TestBase):
    def _init__(self, rod):
        self.requires_is = True
        self.first_reading = True

    def execute(self):

	print("*********************************************************")
	print("BOC BCF REGISTERS CHECKED")
	print("*********************************************************")

	####################################
	#This test is a common test for both Pixel and IBL
	#Do something to generate the BOC dump and save in the path /pixeldaqtest/testlibrary/dumps
	#Name of the boc dump must be "boc_dump.csv" # Modify the name in /pixeldaqtest/testlibrary/py3_scripts/bcf_py3.py as well

	####################################
	py3_script="python3 testlibrary/py3_scripts/bcf_py3.py"
	try:
	    output = subprocess.check_output(['env', '-i', 'bash', '--norc', '-c', py3_script])
	except subprocess.CalledProcessError as e:
	    print e.output

	with open("output/BOC_BCF_out.txt", "a") as f:
	  print >> f, datetime.datetime.now()
	  print >> f, self.rod
	  print >> f, output

