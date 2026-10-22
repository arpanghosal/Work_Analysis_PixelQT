#based on 412120
import MadGraphControl.MadGraph_NNPDF30NLO_Base_Fragment
from MadGraphControl.MadGraphUtils import *


# General settings
name='tWgamma_nonallhad'
evgenConfig.nEventsPerJob = 10000
nevents = runArgs.maxEvents*1.2 if runArgs.maxEvents>0 else 1.1*evgenConfig.nEventsPerJob

defs = """
define l+ = e+ mu+ ta+
define l- = e- mu- ta-
define vl = ve vm vt
define vl~ = ve~ vm~ vt~
define uc = u c
define uc~ = u~ c~
define ds = d s
define ds~ = d~ s~
"""

mcprod_maddec = defs+"""
define w+child = l+ vl uc ds~
define w-child = l- vl~ ds uc~
decay t > w+ b, w+ > w+child w+child
decay t~ > w- b~, w- > w-child w-child
decay w+ > w+child w+child
decay w- > w-child w-child
"""

process = """
import model sm-no_b_mass
define p = g u c d s b u~ c~ d~ s~ b~
define w = w+ w-
define ttbar = t t~
generate p p > ttbar w a 
output -f"""


#Fetch default LO run_card.dat and set parameters
settings = {'lhe_version'   :'3.0',
           'maxjetflavor'  :5,
           'cut_decays'    :'T',
           'ptl'           :5.,
           'pta'           :15.,
           'ptj'           :1.,
           'xptl'          :15.,
           'etal'          :5.0,
           'etaa'          :5.0,
           'etaj'          :-1,
           'etab'          :-1,
           'drjj'          :0.0,
           'drjl'          :0.0,
           'drll'          :0.0,
           'draa'          :0.0,
           'draj'          :0.2,
           'dral'          :0.2,
           'dynamical_scale_choice':'3',
           'nevents'    :int(nevents)}

process_dir = new_process(process)
modify_run_card(process_dir=process_dir,runArgs=runArgs,settings=settings)

# Decay with MadSpin
madspin_card=process_dir+'/Cards/madspin_card.dat'
if os.access(madspin_card,os.R_OK):
    os.unlink(madspin_card)
mscard = open(madspin_card,'w') 
mscard.write("""#************************************************************
#*                        MadSpin                           *
#*                                                          *
#*    P. Artoisenet, R. Frederix, R. Rietkerk, O. Mattelaer *
#*                                                          *
#*    Part of the MadGraph5_aMC@NLO Framework:              *
#*    The MadGraph5_aMC@NLO Development Team - Find us at   *
#*    https://server06.fynu.ucl.ac.be/projects/madgraph     *
#*                                                          *
#************************************************************
set max_weight_ps_point 500  # number of PS to estimate the maximum for each event
set BW_cut 50
set seed %i
%s
launch
"""%(runArgs.randomSeed, mcprod_maddec))
mscard.close()

# Print cards
print_cards()
# set up
generate(process_dir=process_dir,runArgs=runArgs)
# run
outputDS=arrange_output(process_dir=process_dir,runArgs=runArgs,lhe_version=3,saveProcDir=True)  
check_reset_proc_number(opts)

# pythia shower
keyword=['SM','top', 'Wgamma', 'photon']
evgenConfig.generators += ["MadGraph", "Pythia8"]
evgenConfig.description = 'MadGraph_'+str(name)+'_GamFromProd'
evgenConfig.keywords += keyword
evgenConfig.contact = ["arpan.ghosal@cern.ch"]

runArgs.inputGeneratorFile=outputDS+".events"
include("Pythia8_i/Pythia8_A14_NNPDF23LO_EvtGen_Common.py")
include("Pythia8_i/Pythia8_MadGraph.py")
