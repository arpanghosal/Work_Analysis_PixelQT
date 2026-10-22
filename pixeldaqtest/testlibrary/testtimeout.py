from core.tools.partition import get_partition, get_daqpartition
from core.daqtest import TestBase, PASS, FAIL
from ispy import *
from datetime import datetime 

class TestTimeout(TestBase):

    def execute(self):
        word='TIMEOUT'
	print("*********************************************************")
	print("TIMEOUT SIGNAL CHECKED")
	print("*********************************************************")
	d = ISInfoDictionary(get_partition())
	timeout_val = ISInfoDynAny()
	d.getValue('RunCtrl.RootController', timeout_val)
	timeout_val = d['RunCtrl.RootController']

	##Opening the list with timeout signals for all RODS##

	f = open('lists/timeout_list.txt', 'r')
	for line in f.readlines():
	  filecontent=line.split(',')

	##Filtering for the ROD specified##


	res= [i for i in filecontent if self.rod in i]
	result= [i for i in res if word in i]

	filtered_list=[[] for t in range(len(result))]
	for i in range(len(result)):
	  filtered_list[i]=result[i][2:-1]


	##########################################
	with open('output/timeout_out.txt', 'a') as f:
	  print >> f, (datetime.now())
	  for i in range(len(filtered_list)):
	    d.getValue(filtered_list[i], timeout_val)
	    if (timeout_val[0]==0):
	      print >> f, ("{} IS NOT IN TIMEOUT".format(self.rod))
	      return PASS
	    else :
	      print >> f, ("{} IS IN TIMEOUT".format(self.rod))
	      print >> f, (filtered_list[i])
	      #continue
	


        return FAIL
