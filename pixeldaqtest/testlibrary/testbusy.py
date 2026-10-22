from core.tools.partition import get_partition, get_daqpartition
from core.daqtest import TestBase, PASS, FAIL
from ispy import * 
from datetime import datetime

class TestBusy(TestBase):

    def execute(self):
        word='BUSYPERC'
	print("*********************************************************")
	print("BUSY SIGNAL CHECKED")
	print("*********************************************************")

        d = ISInfoDictionary(get_partition())
        busy_val = ISInfoDynAny()
        key = 'RunParams.ROD_CRATE_'+self.rod[5]+'/'+self.rod+'/BUSY'
        d.getValue(key, busy_val)

	with open('output/busy_out.txt', 'a') as f:
	  print >> f, (datetime.now())
          if (busy_val[0]==0):
            print >> f, ("{} IS NOT BUSY".format(self.rod))
            return PASS
          else:
            print >> f, ("{} IS BUSY".format(self.rod))


        ################################################

	##Opening the list with busy signals for all RODS##

          f = open('lists/busy_list.txt', 'r')
	  for line in f.readlines():
	    filecontent=line.split(',')

	##Obtaining the busy percentage for ROD specidfied##

          res= [i for i in filecontent if self.rod in i]
          result= [i for i in res if word in i]

        ##########################################


	  filtered_list=[[] for t in range(len(result))]
	  for i in range(len(result)):
	    filtered_list[i]=result[i][2:-1]

	with open('output/busy_out.txt', 'a') as f:
	  x = ISInfoDynAny()
	  #Cause of BUSY signal
	  print >> f, ("Cause of BUSY Signal:")
	  for i in range(len(filtered_list)):
	    d.getValue(filtered_list[i], x)
	    if (x[0]>0.0):
	      print >> f, (filtered_list[i], x[0])
	    else:
	      continue
	  
          return FAIL

######################################################################
