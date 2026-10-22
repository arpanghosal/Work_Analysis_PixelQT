import numpy as np
import pandas as pd
with open('testlibrary/dumps/boc_dump_ibl.csv') as f:
  content = f.readlines()
content = [x.strip() for x in content] 

#####################################################
#THIS SCRIPT NEEDS CHANGES. LOOK FOR KEYWORD "MODIFY"
#####################################################

config= [i for i in content if 'Configuration:' in i]
trigger=[i for i in content if 'Trigger count:' in i]
ecr=[i for i in content if 'ECR count:' in i]
slow=[i for i in content if 'Slow command count:' in i]
corrupted=[i for i in content if 'Corrupted command count:' in i]

#Generate N, S lists for each check
def tx_N_S(reg):
  tx_N=[]
  tx_S=[]
  for i in range(0,8,1):
    tx_N.append(reg[i])
    tx_S.append(reg[i+16])
  return tx_N, tx_S

#Name and expected values of tx contents
name=[config, trigger, ecr, slow, corrupted]


#MODIFY
#These values are hardcoded at the moment, must be modified. (Obtain trigger count, ecr count etc from IS and replace here)
val=[40, 10000, 20, 10, 0]

### Func to check if all values match ###
def all_same(items):
    return all(x == items[0] for x in items)


#Function to compare the values and print differences
err_name = ['Configuration', 'Trigger count', 'ECR count', 'Slow command count', 'Corrupted command count']
def count_error(name, err_name, num):
  count=[]
  tx_channel=[]
  if(all_same(name)==False):
    print("ERROR IN", err_name)
    for i in range(len(name)):
      if(name[i]!=val[num]):
        tx_channel.append(i)
        count.append(name[i])
        #print(direction)
        dictionary={'TX Channel' : tx_channel, err_name : count}
      else:
        continue
    print(pd.DataFrame.from_dict(dictionary))
  else:
    print("NO ERROR IN", err_name)
  #return ("************************************")


#Printing result
#0-> North, 1-> South

print("NORTH")
(count_error(tx_N_S(config)[0], err_name[0], 0))
(count_error(tx_N_S(trigger)[0], err_name[1],1))
(count_error(tx_N_S(ecr)[0], err_name[2], 2))
(count_error(tx_N_S(slow)[0],err_name[3], 3))
(count_error(tx_N_S(corrupted)[0], err_name[4],4))

print("SOUTH")
(count_error(tx_N_S(config)[1],err_name[0], 0))
(count_error(tx_N_S(trigger)[1], err_name[1],1))
(count_error(tx_N_S(ecr)[1],err_name[2], 2))
(count_error(tx_N_S(slow)[1],err_name[3], 3))
(count_error(tx_N_S(corrupted)[1],err_name[4], 4))
