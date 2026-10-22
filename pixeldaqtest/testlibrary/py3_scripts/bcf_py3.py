import pandas as pd
import numpy as np


with open('testlibrary/dumps/boc_dump_ibl.csv') as f:
  content = f.readlines()
content = [x.strip() for x in content] 

#check for keyword "INTERNAL"
res= [i for i in content if 'Internal' in i]

#Printing result
if (len(res)!=0):
  print("BOC incorrectly configured for data taking")
else:
  print("BOC correctly configured for data taking")

#Freq must be within +/-100KHz of 40MHz
  
exp_freq='40000000'
act_freq=content[18][21:-3]
#act_freq='39800000'
freq=int(act_freq)

if(39900000 <= freq <= 40100000):
  exp_freq=str(freq)
else:
  act_freq=str(freq)

clock=[i for i in content if 'BOC clock source:' in i]
delay=[i for i in content if 'Global Idelay:' in i]
  
#Creating lists with actual and expected values
act_reg=[clock[0].split(': ')[-1], delay[0].split(': ')[-1], act_freq]
exp_reg=['PLL buffered 40 MHz from VME', '0', exp_freq]
register=['BOC clock source', 'Global Idelay', 'VME Clock Frequency']


#Creating dataframes with actual and expected values
exp_data = {'Register' : register, 'Value' : exp_reg} 
act_data= {'Register' : register, 'Value' : act_reg} 
expected=pd.DataFrame.from_dict(exp_data)
actual=pd.DataFrame.from_dict(act_data)

exp=[]
act=[]
reg=[]

def reg_error():
  ne_stacked = (actual != expected).stack()
  changed = ne_stacked[ne_stacked]
  difference_locations = np.where(actual != expected)
  changed_from = expected.values[difference_locations]
  changed_to = actual.values[difference_locations]
  diff=pd.DataFrame({'Expected': changed_from, 'Actual': changed_to}, index=changed.index)

  for k in range(len(actual)):
    for i in range(len(diff)):
      if (diff.iloc[i].name[0]==k):
        exp.append(diff.iloc[i][0].strip())
        act.append(diff.iloc[i][1].strip())
        reg.append(actual.iloc[k][0])
      else:
        continue
  data = {'Register' : reg, 'Expected' : exp, 'Actual' : act} 
  print(pd.DataFrame.from_dict(data))

############################################################################
if(expected.equals(actual)):
  print("ALL REGISTERS MATCHED")
else:
  reg_error()
