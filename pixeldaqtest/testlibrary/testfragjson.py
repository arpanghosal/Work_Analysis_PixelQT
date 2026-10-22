from core.tools.partition import get_partition, get_daqpartition
from core.daqtest import TestBase, PASS, FAIL
from ispy import *
import datetime
import json
from collections import OrderedDict

class TestFragError(TestBase):
    def _init__(self, rod):
        self.requires_is = True
        self.first_reading = True

    def execute(self):

#        # do something on first reading
#        if(self.first_reading):
#            self.first_reading = False
#            self.initialize()
#            return PASS


	d = ISInfoDictionary(get_daqpartition())
	IS_val = ISInfoDynAny()

	#######################################################
	#VALUES TO BE READ

	d.getValue('DF.ROS.Appl_PixelROS05.ReadoutModule0', IS_val)
	L1_trig = ISInfoDynAny()
	d.getValue('DF.HLTSV.Events', L1_trig)

	print("*********************************************************")
	print("FRAGMENT ERRORS CHECKED")
	print("*********************************************************")

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

	#######################

	def frag_rec():
	    if (IS_val[23][rol_en[i]] == L1_trig[0]):
	      return "EQUAL"
	      #print >> f, ("Fragments received are equal to triggers sent"  )
	    else:
	      return "NOT EQUAL"
	      #print >> f, ('Fragments received are NOT equal to triggers sent'  )
	      #print >> f, ('Fragments Received = ', IS_val[23][rol_en[i]]  )
	      #print >> f, ('L1 triggers sent = ', L1_trig[0]  )

	######################################################################
	#ERROR CHECKS

	def checks(index, x):
	    if (IS_val[index][x]==0):
	      #print >> f, (err_name + " CHECKED"  ) 
	      return "CHECKED"   
	    else:
	      #print >> f, (err_name + "=" , IS_val[index][x]  )
	      return IS_val[index][x]
	######################################################################
	word=['numberOfMayCome', 'numberOfNeverToCome', 'numberOfNotDeleted', 'fragsSizeMismatch', 'fragsFormatError', 'fragsMarkerError', 'fragsFramingError', 'fragsCtrlWordError', 'fragsDataWordError', 'fragsRejected', 'fragsCorrupted', 'fragsTruncated', 'fragsOutofsync', 'fragsDiscarded']

	######################################################################
	#TX ERROR CHECK

	def tx_error():
	    global error1
	    error1=IS_val[35][rol_en[i]] + IS_val[36][rol_en[i]] + IS_val[37][rol_en[i]] + IS_val[38][rol_en[i]] + IS_val[39][rol_en[i]] + IS_val[40][rol_en[i]]
	    error2=IS_val[27][rol_en[i]] + IS_val[41][rol_en[i]]
	    if (error1==error2==0):
	      return "NONE"
	      #print >> f, ("NO Tx ERRORS"  )
	    elif (error1==error2):
	      return "IN SYNC"
	      #print >> f, ("Tx ERRORS IN SYNC"  )
	      #print >> f, ("Total Tx Errors = ", error1  )
	    else:
	      return "NOT IN SYNC"
	      #print >> f, ("Tx ERRORS NOT IN SYNC"  )

	#######################################################################
	#Xoff CHECK

	def xoff():

	    k=open('xoff.txt', 'r')
	    for line in k.readlines():
	      filecontent=line.split(',')

	    xoff_list=[]
	    for i in range(len(filecontent)):
	      xoff_list.append((filecontent[i]).strip())

	    val_init=[]
	    val_fin=[]
	    tmp=0
	    rol=[tmp] + rol_en
	    for i in range(1, (len(rol)), 1):
	      #print(rol[i])
	      val_init.append(int(xoff_list[rol[i]+1]))
	      val_fin.append(IS_val[16][rol[i]])

	    ####Test for Xoff####
	    exp = "Expected :", val_init
	    act = "Actual :", val_fin
	    if(val_init==val_fin):
	      return "SAME"
	      #print >> f, ("NO ERROR in number of Xoff")
	    else:
	      return "NOT SAME", exp, act
	      #print >> f, ("ERROR in number of Xoff") 
	      #print >> f, ("Expected :", val_init)
	      #print >> f, ("Actual :", val_fin)

	#######################################################################
	def convert_timestamp(o):
	  if isinstance(o, datetime.datetime):
	    return o.__str__()
	

	#######################################################################
	with open('output/frag_out.json', 'a') as f:
	  for i in range(len(rol_en)):
	    d={}
	    d[self.rod]={}
	    d[self.rod]['1_TIME'] = datetime.datetime.now()
	    d[self.rod]['2_ROB_ID'] = hex(IS_val[3][rol_en[i]])
	    d[self.rod]['3_FRAGMENTS RECEIVED'] = {}
	    d[self.rod]['3_FRAGMENTS RECEIVED']['Fragments received and Triggers sent'] = frag_rec()
	    d[self.rod]['3_FRAGMENTS RECEIVED']['Fragments received'] = IS_val[23][rol_en[i]]
	    d[self.rod]['3_FRAGMENTS RECEIVED']['Triggers sent'] = L1_trig[0]	
	    d[self.rod]['4_CHECKS'] = {}
	    d[self.rod]['4_CHECKS'][word[0]] = checks(0, rol_en[i])
	    d[self.rod]['4_CHECKS'][word[1]] = checks(1, rol_en[i])
	    d[self.rod]['4_CHECKS'][word[2]] = checks(2, rol_en[i])
	    d[self.rod]['4_CHECKS'][word[3]] = checks(40, rol_en[i])
	    d[self.rod]['4_CHECKS'][word[4]] = checks(35, rol_en[i])
	    d[self.rod]['4_CHECKS'][word[5]] = checks(36, rol_en[i])
	    d[self.rod]['4_CHECKS'][word[6]] = checks(37, rol_en[i])
	    d[self.rod]['4_CHECKS'][word[7]] = checks(38, rol_en[i])
	    d[self.rod]['4_CHECKS'][word[8]] = checks(39, rol_en[i])
	    d[self.rod]['4_CHECKS'][word[9]] = checks(41, rol_en[i])
	    d[self.rod]['4_CHECKS'][word[10]] = checks(27, rol_en[i])
	    d[self.rod]['4_CHECKS'][word[11]] = checks(26, rol_en[i])
	    d[self.rod]['4_CHECKS'][word[12]] = checks(29, rol_en[i])
	    d[self.rod]['4_CHECKS'][word[13]] = checks(31, rol_en[i])
	    d[self.rod]['5_TX'] = {}
	    d[self.rod]['5_TX']['TX ERRORS'] = tx_error()
	    d[self.rod]['5_TX']['Total TX ERRORS'] = error1
	    d[self.rod]['6_XOFFS'] = {}
	    d[self.rod]['6_XOFFS']['NO. OF XOFFS'] = xoff()
	    json.dump(d, f, indent=4, default=convert_timestamp, sort_keys=True)







	

       
