import numpy as np
import pandas as pd


with open('testlibrary/dumps/boc_dump_ibl.csv') as f:
  content = f.readlines()
content = [x.strip() for x in content]

#Creating lists of registers that are to be checked

slink0_control= [i for i in content if 'SLINK0 Control:' in i]
slink1_control= [i for i in content if 'SLINK1 Control:' in i]
slink0_status= [i for i in content if 'SLINK0 Status:' in i]
slink1_status= [i for i in content if 'SLINK1 Status:' in i]
slink0_xoff= [i for i in content if 'SLINK0 HOLA0 (TDAQ) XOFF occurrencies :' in i]
slink0_ldown= [i for i in content if 'SLINK0 HOLA0 (TDAQ) LDOWN occurrencies:' in i]
slink1_xoff= [i for i in content if 'SLINK1 HOLA0 (TDAQ) XOFF occurrencies :' in i]
slink1_ldown= [i for i in content if 'SLINK1 HOLA0 (TDAQ) LDOWN occurrencies:' in i]

#Generate N, S lists for each check
def bmf_N_S(reg):
  bmf_N=[]
  bmf_S=[]
  bmf_N_val=[]
  bmf_S_val=[]
  for i in range(0,1,1):
    bmf_N.append(reg[i])
    bmf_S.append(reg[i+1])
  for i in range(len(bmf_N)):
    bmf_N_val.append(bmf_N[i].split(': ')[-1])
    bmf_S_val.append(bmf_S[i].split(': ')[-1])
  return bmf_N,(''.join(bmf_N_val)), bmf_S,(''.join(bmf_S_val))

#Default values for BMF North slink status regs
#default vaules
exp_status_N0=0xe4
exp_status_N1=0xe4
act_status_N0=bmf_N_S(slink0_status)[1]
act_status_N1=bmf_N_S(slink1_status)[1]

#Change default values if condition matches

#SLINK0
if ((act_status_N0==0xe4)|(act_status_N0==0xf4)):
  exp_status_N0=(act_status_N0)  
else:
  act_status_N0=(act_status_N0)
  
#SLINK1  
if ((act_status_N1==0xe4)|(act_status_N1==0xf4)):
  exp_status_N1=(act_status_N1)  
else:
  act_status_N1=(act_status_N1)


#Lists containing BMF North actual and expected values
act_reg_N=[bmf_N_S(slink0_control)[1], act_status_N0,  bmf_N_S(slink1_control)[1] , act_status_N1, bmf_N_S(slink0_xoff)[1], bmf_N_S(slink0_ldown)[1],bmf_N_S(slink1_xoff)[1], bmf_N_S(slink1_ldown)[1]]
exp_reg_N=['0x90', hex(exp_status_N0), '0x90', hex(exp_status_N1), '0', '0', '0', '0' ]
register=['SLINK0 Control', 'SLINK0 Status', 'SLINK1 Control','SLINK1 Status', 'SLINK0 HOLA0 (TDAQ) XOFF occurrencies', 'SLINK0 HOLA0 (TDAQ) LDOWN occurrencies',
          'SLINK1 HOLA0 (TDAQ) XOFF occurrencies','SLINK1 HOLA0 (TDAQ) LDOWN occurrencies']


#Creating Dataframe with actual and expected values for BMF North
exp_data_N = {'Register' : register, 'Value' : exp_reg_N} 
act_data_N = {'Register' : register, 'Value' : act_reg_N} 
expected_N=pd.DataFrame.from_dict(exp_data_N)
actual_N=pd.DataFrame.from_dict(act_data_N)


#Default values for BMF South slink status regs
#default vaules
exp_status_S0=0xe4
exp_status_S1=0xe4
act_status_S0=bmf_N_S(slink0_status)[3]
act_status_S1=bmf_N_S(slink1_status)[3]

#Change default values if condition matches

#SLINK0
if ((act_status_S0==0xe4)|(act_status_S0==0xf4)):
  exp_status_S0=(act_status_S0)  
else:
  act_status_S0=(act_status_S0)
  
#SLINK1  
if ((act_status_S1==0xe4)|(act_status_S1==0xf4)):
  exp_status_S1=(act_status_S1)  
else:
  act_status_S1=(act_status_S1)

#Lists containing BMF South actual and expected values
act_reg_S=[bmf_N_S(slink0_control)[3], act_status_S0,  bmf_N_S(slink1_control)[3] , act_status_S1, bmf_N_S(slink0_xoff)[3], bmf_N_S(slink0_ldown)[3],bmf_N_S(slink1_xoff)[3], bmf_N_S(slink1_ldown)[3]]
exp_reg_S=['0x90', hex(exp_status_S0), '0x90', hex(exp_status_S1), '0', '0', '0', '0' ]

#Creating Dataframe with actual and expected values for BMF South
exp_data_S = {'Register' : register, 'Value' : exp_reg_S} 
act_data_S = {'Register' : register, 'Value' : act_reg_S} 
expected_S=pd.DataFrame.from_dict(exp_data_S)
actual_S=pd.DataFrame.from_dict(act_data_S)


#Function to compare actual and expected values and output the differences
exp=[]
act=[]
reg=[]
def reg_error(actual, expected):
  ne_stacked = (actual != expected).stack()
  changed = ne_stacked[ne_stacked]
  difference_locations = np.where(actual != expected)
  changed_from = expected.values[difference_locations]
  changed_to = actual.values[difference_locations]
  diff=pd.DataFrame({'Expected': changed_from, 'Actual': changed_to}, index=changed.index)

  for k in range(len(actual)):
    for i in range(len(diff)):
      if (diff.iloc[i].name[0]==k):
        exp.append(diff.iloc[i][0])
        act.append(diff.iloc[i][1])
        reg.append(actual.iloc[k][0])
      else:
        continue
  data = {'Register' : reg, 'Expected' : exp, 'Actual' : act} 
  print(pd.DataFrame.from_dict(data))


#Printing results
print("IBL BMF NORTH")
reg_error(actual_N, expected_N)

print("IBL BMF SOUTH")
exp=[]
act=[]
reg=[]
reg_error(actual_S, expected_S)
