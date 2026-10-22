from core.tools.partition import get_partition, get_daqpartition
from core.daqtest import TestBase, PASS, FAIL
from ispy import * 
import json
import datetime

class TestRodMon(TestBase):

    def execute(self):
	print("*********************************************************")
	print("RODMON VARIABLES CHECKED")
	print("*********************************************************")

	###########################################################

	#Creating headers for result tables
	header=[[['ROD_NAME', 'CRATE/LINK', 'OCCUPANCY']], [['ROD_NAME', 'CRATE/LINK', 'LINK USAGE']], [['ROD_NAME', 'CRATE/LINK', 'BUSY FRACTION']], [['ROD_NAME', 'CRATE/LINK', 'BOC FRAME ERROR']], [['ROD_NAME', 'CRATE/LINK', 'BOC DECODING ERROR']], [['ROD_NAME', 'CRATE/LINK', 'BOC FRAME COUNT']], [['ROD_NAME', 'CRATE/LINK', 'HT']], [['ROD_NAME', 'CRATE/LINK', 'TOT_DESYNC']]]

	#Creating a keyword list
	word=['TEST', 'FMTOccupancy', 'FMTLinkUsage', 'BusyFrac', 'BOCFrameError', 'BOCDecodingError', 'BOCFrameCount', 'FMTSTAT', 'DAQEnabled', 'AVAILABLE', 'HT', 'TOT_DESYNC'] 

	#Default vaules for the above keywords
	default=[0.0, 0.0, 80, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0] 


	#Opening the list with all signals corresponding to all RODS
	f = open('lists/final_list.txt', 'r')
	for line in f.readlines():
	  filecontent=line.split(',')

	filecontent.remove(filecontent[2323])
	filecontent.append(" 'RunParams.ROD_CRATE_3/ROD_C3_S8/L2_B28_S1_C7_M6C/TOT_DESYNC'")

	res= [i for i in filecontent if self.rod in i]
	d = ISInfoDictionary(get_partition())
	val = ISInfoDynAny()

	############################################################
	#Func. to parse through the total list, look for the above keywords, obtain corresponding values 

	def general_func(num, index):
	    result= [i for i in res if word[num] in i]
	    filtered_list=[[] for t in range(len(result))]
	    for i in range(len(result)):
	      filtered_list[i]=result[i][2:-1]

	    val_list=[]
	    name_list=[]
	    for i in range(len(filtered_list)):
	      d.getValue(filtered_list[i], val)
              if (val[0] >=default[num])& (val[0] !=0.0):
	      #if (val[0] != 0.0):
	        val_list.append(val[0])
		name_list.append(filtered_list[i])
	      else:
	        continue
	
	    count=len(val_list)

	    if (len(val_list)==0):
	      return "no error", "NO ERROR", 0
	    else: 
	      return name_list, val_list, count

	##########################################################

	# Check Modules Enabled (check if they match data in FMSTAT)

	result= [i for i in res if word[7] in i]
	filtered_list=[[] for t in range(len(result))]
	for i in range(len(result)):
	  filtered_list[i]=result[i][2:-1]

	fmtstat=0
	val_list=[]
	for i in range(len(filtered_list)):
	  d.getValue(filtered_list[i], val)
	  val_list.append(val[0].strip())
	#'69' is the ASCII value of E. This is used to check if a module is enabled or not.
	#Enables module is published as E in the IS monitor
	  if (ord(val_list[i])==69):
	    fmtstat+=1
	  else:
	    continue

	##########################################################
	#DAQEnabled for each ROD must match the FMTSTAT calculated above
	result= [i for i in res if word[8] in i]
	filtered_list=[[] for t in range(len(result))]
	for i in range(len(result)):
	  filtered_list[i]=result[i][2:-1]

	daq_en=-1
	val_list=[]
	for i in range(len(filtered_list)):
	  d.getValue(filtered_list[i], val)
	  val_list.append(val[0])
	  if (val_list[i]==0):
	    daq_en+=1
	  else:
	    continue


	############################################################
	def rand():
	  result= [i for i in res if word[9] in i]
	  filtered_list=[[] for t in range(len(result))]
	  for i in range(len(result)):
	    filtered_list[i]=result[i][2:-1]

	  d.getValue(filtered_list[0], val)

	  if (len(filtered_list)==1) & (val[0]==0):
	    return "{} IS AVAILABLE".format(self.rod)
	  else:
	    return "{} IS NOT AVAILABLE".format(self.rod)

	######################################
	def num_gen(x,y):
	
	  pop[self.rod][word[x]] = {}	
	  for i in range(general_func(x,y)[2]):
	    pop[self.rod][word[x]][general_func(x,y)[0][i]] = general_func(x,y)[1][i]

	#######################################################################
	def convert_timestamp(o):
	  if isinstance(o, datetime.datetime):
	    return o.__str__()
	

	#######################################################################

	with open('output/RodMon_out.json', 'a') as f:
	    if (fmtstat==daq_en):
	      pop = {}
	      pop[self.rod] = {}
	      pop[self.rod]['1_TIME'] = datetime.datetime.now()
	      pop[self.rod]['2_AVAILABILITY'] = rand()
	      pop[self.rod]['3_MODULES ENABLED'] = 'MATCH'
	      pop[self.rod]['4_NUMBER OF MODULES ENABLED'] = fmtstat
  	      num_gen(1, 0)
	      num_gen(2, 1)
	      num_gen(3, 2)
	      num_gen(4, 3)
	      num_gen(5, 4)
	      num_gen(6, 5)
	      num_gen(10, 6)
	      num_gen(11, 7)

	    else:
	      pop = {}
	      pop[self.rod] = {}
	      pop[self.rod]['1_MODULES ENABLED'] = 'DO NOT MATCH'
	      pop[self.rod]['2_FMTSTAT'] = fmtstat
	      pop[self.rod]['3_DAQEnabled'] = daq_en

	    json.dump(pop, f, indent=4, default=convert_timestamp, sort_keys=True)

	##############################################################



