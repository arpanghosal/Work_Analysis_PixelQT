import csv
import numpy as np
import pandas as pd
import os

#Extracting relevant registers from the ROD dump
val=pd.read_csv('testlibrary/rod_dump.csv', skiprows=[40,42,78, 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,34,35,36,37,38,39,41,43,44,45,46,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,72,73,74,77, 79,84] , skipfooter=503, header=None,engine='python')


#Generating dataframe with ACTUAL values of ROD master registers
val.to_csv(r'val.csv', index=False, header=None)
actual_val=pd.read_csv('val.csv',header=None, delimiter="=")

for i in range(len(actual_val)):
  actual_val[1][i]=actual_val[1][i].strip()
  actual_val[0][i]=actual_val[0][i].strip()


#Generating dataframe with EXPECTED values of ROD master registers
expected_val=pd.read_csv('master.csv',header=None, delimiter=",")

#delete extra files
os.remove("val.csv")
os.remove("master.csv")
os.remove("testlibrary/rod_dump.csv")

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

if(expected_val.equals(actual_val)):
  print("ALL MATCHED")
else:
  reg_error(actual_val, expected_val)

