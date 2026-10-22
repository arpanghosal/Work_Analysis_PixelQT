#from datetime import datetime
import sys
import re
import fnmatch
#dis_mod = ['*.L1_B21_S1_A6_M1A*.', '*.L1_B21_S1_A6_M6A*.', '*.L0_B13_S1_C7_M0*.', '*.L0_B13_S1_C7_M3C*.', '*.L0_B13_S1_C7_M5C*.', '*.L0_B13_S1_C7_M6C*.', '*.L1_B21_S1_C7_M0*.', '*.L1_B21_S1_C7_M5C*.', '*.L1_B28_S1_C7_M0*.', '*.L1_B28_S1_C7_M1C*.', '*.L1_B28_S1_C7_M5C*.', '*.L1_B28_S1_C7_M6C*.', '*.L2_B28_S1_A6_M4A*.', '*.L2_B28_S1_A6_M6A*.'] 


dis_mod = ['L1_B21_S1_A6_M1A', 'L1_B21_S1_A6_M6A', 'L0_B13_S1_C7_M0', 'L0_B13_S1_C7_M3C', 'L0_B13_S1_C7_M5C', 'L0_B13_S1_C7_M6C', 'L1_B21_S1_C7_M0', 'L1_B21_S1_C7_M5C', 'L2_B28_S1_C7_M0', 'L2_B28_S1_C7_M1C', 'L2_B28_S1_C7_M5C', 'L2_B28_S1_C7_M6C', 'L2_B28_S1_A6_M4A', 'L2_B28_S1_A6_M6A']

filecontent=open('final_list.txt', 'r')
for line in filecontent.readlines():
    f=line.split(',')
#with open('final_list.txt') as flist:
#    f = flist.read().splitlines(',')
#print(type(f))
#print f
#print sys.getsizeof(f)
#for i in dis_mod:
 #   r=re.compile(.*i*)
 #   newlist = list(filter(r.match, f))
for i in dis_mod:
    print i

print "Length of f list: ", len(f)
print "Length of disabled modules: ", len(dis_mod)
#g = f
#for x, elem in enumerate(g):
#    g[x] = " ".join(filter(lambda x: x not in dis_mod, elem.split()))
#print(f)

#with open('g.txt','a') as g:
#    print >> g
#for j in f:
#    print j
#print(len(f))
print "Type of f list: ", type(f)
print "Type of disabled modules: ", type(dis_mod)
#for i in f:
#    print i
matches=[] 
'''
def listing:

    for i in dis_mod:
        regex=re.compile(i)
        #print regex
        #matches=[word for word in f if re.match(i, word)]
        matches=[word for word in f if re.search(regex,word)]
        #print matches
        print len(matches)

'''        

'''
for i in dis_mod:
    regex=re.compile(i)
    matches=[word for word in f if re.search(regex,word)]
    print matches
    print "++++++++++++++++++++++++"

'''
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
        print ":::::::::::::::"
print matches
print type(matches)
print len(matches)


'''

x=0
for w in f:
    for i in dis_mod:
        regex=re.compile(i)
        if not re.search(regex,w):
            x=x+1
    if(x==14):
        matches.append(w)
    print matches
'''
'''
x=0
for w in f:
    for i in dis_mod:
        regex=re.compile(i)
        if not re.search(regex,w):
            x=x+1
    if(x==14):
        matches.append(w)
    print matches
'''    

#with open('regex_list.txt', 'a') as matches:
#    print >> matches
#list1 =[]
#x=0
'''
for i in f:
    for j in dis_mod:
        var = "*."+j+"*."
        if (i == var):
            x=x+1
    if (x==0):
        list1.append(i)
'''
#print list1
#print len(list1)

           
'''
list1 = []
for word in f:
    word1=''
    for remove_word in dis_mod:
        word1=\*remove_word\*
        word1=word.replace(word1, '')
    list1.append(word1)
'''
with open('new_list.txt', 'w') as g:
    for item in matches:
        print >> g, item
#new = [i for i in f if all(ch not in i for ch in dis_mod)]
#print str(new)

print " f done"
