import sys
import re

dis_mod = ['L1_B21_S1_A6_M1A', 'L1_B21_S1_A6_M6A', 'L0_B13_S1_C7_M0', 'L0_B13_S1_C7_M3C', 'L0_B13_S1_C7_M5C', 'L0_B13_S1_C7_M6C', 'L1_B21_S1_C7_M0', 'L1_B21_S1_C7_M5C', 'L2_B28_S1_C7_M0', 'L2_B28_S1_C7_M1C', 'L2_B28_S1_C7_M5C', 'L2_B28_S1_C7_M6C', 'L2_B28_S1_A6_M4A', 'L2_B28_S1_A6_M6A']

filecontent=open('final_list.txt', 'r')
for line in filecontent.readlines():
    f=line.split(',')
print "List of disabled modules:\n"
for i in dis_mod:
    print i

print "Length of disabled modules: ", len(dis_mod)

matches=[] 

def listing(w):
    x=0
    for i in dis_mod:
        regex=re.compile(i)
        if not re.search(regex,w):
            x=x+1
    if (x==14):
        return 1

for w in f:
    global matches
    r=listing(w)
    if (r==1):
        matches.append(w)

with open('new_final_list.txt', 'w') as g:
    for item in matches:
        print >> g, item

