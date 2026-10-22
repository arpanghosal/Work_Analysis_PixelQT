import csv
import numpy as np
import pandas as pd
import os

#Extracting relevant SLAVE A registers from the ROD dump
dump_A=pd.read_csv('testlibrary/rod_dump.csv', skiprows=109 , skipfooter=419, header=None, engine='python')
dump_A=dump_A.drop(dump_A.index[[2,3,4,5,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,55,56,57,58,59]])
dump_A.to_csv(r'dump_A.csv', index=False, header=None)

#Generating dataframe with ACTUAL values of ROD SLAVE A registers
actual_val_A=pd.read_csv('dump_A.csv',header=None, delimiter="=")

for i in range(len(actual_val_A)):
  actual_val_A[1][i]=actual_val_A[1][i].strip()
  actual_val_A[0][i]=actual_val_A[0][i].strip()


#Generating dataframe with EXPECTED values of ROD SLAVE A registers
expected_val_A=pd.read_csv('slave_a.csv',header=None, delimiter=",")

#delete extra files
os.remove('dump_A.csv')
os.remove("slave_a.csv")


exp=[]
act=[]
reg=[]

#Func. to compare both dataframes and create a third dataframe with unmatched register values
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
        exp.append(diff.iloc[i][0].strip())
        act.append(diff.iloc[i][1].strip())
        reg.append(actual[0][k])
      else:
        continue
  data = {'Register' : reg, 'Expected' : exp, 'Actual' : act} 
  print(pd.DataFrame.from_dict(data))

############################################################################
if(expected_val_A.equals(actual_val_A)):
  print("ALL MATCHED")
else:
  reg_error(actual_val_A, expected_val_A)

######################################################################################################################
#Extracting relevant SLAVE B registers from the ROD dump
dump_B=pd.read_csv('testlibrary/rod_dump.csv', skiprows=354 ,skipfooter=174, header=None, engine='python')
dump_B=dump_B.drop(dump_B.index[[2,3,4,5,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,55,56,57,58,59]])
dump_B.to_csv(r'dump_B.csv', index=False, header=None)

#Generating dataframe with ACTUAL values of ROD SLAVE B registers
actual_val_B=pd.read_csv('dump_B.csv',header=None, delimiter="=")

for i in range(len(actual_val_B)):
  actual_val_B[1][i]=actual_val_B[1][i].strip()
  actual_val_B[0][i]=actual_val_B[0][i].strip()

#Generating dataframe with EXPECTED values of ROD SLAVE B registers
expected_val_B=pd.read_csv('slave_b.csv',header=None, delimiter=",")

#delete extra files
os.remove('dump_B.csv')
os.remove("slave_b.csv")
os.remove("testlibrary/rod_dump.csv")

exp=[]
act=[]
reg=[]
####################################################
if(expected_val_B.equals(actual_val_B)):
  print("ALL MATCHED")
else:
  reg_error(actual_val_B, expected_val_B)


