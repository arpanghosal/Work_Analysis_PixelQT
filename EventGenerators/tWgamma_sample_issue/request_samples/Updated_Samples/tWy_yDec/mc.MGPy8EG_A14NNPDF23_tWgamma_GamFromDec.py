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


mcdec = defs+"""
generate p p > t w-, (t > l+ vl b a), (w- > ds uc~)\n
add process p p > t w-, (t > l+ vl b), (w- > ds uc~ a)\n
add process p p > t w-, (t > uc ds~ b a), (w- > l- vl~)\n
add process p p > t w-, (t > uc ds~ b), (w- > l- vl~ a)\n
add process p p > t w-, (t > l+ vl b a), (w- > l- vl~)\n
add process p p > t w-, (t > l+ vl b), (w- > l- vl~ a)\n
add process p p > t~ w+, (w+ > uc ds~ a), (t~ > l- vl~ b~)\n
add process p p > t~ w+, (w+ > uc ds~), (t~ > l- vl~ b~ a)\n
add process p p > t~ w+, (w+ > l+ vl a), (t~ > ds uc~ b~)\n
add process p p > t~ w+, (w+ > l+ vl), (t~ > ds uc~ b~ a)\n
add process p p > t~ w+, (w+ > l+ vl a), (t~ > l- vl~ b~)\n
add process p p > t~ w+, (w+ > l+ vl), (t~ > l- vl~ b~ a)\n
"""


process = """
import model sm-no_b_mass
define p = g u c d s b u~ c~ d~ s~ b~
"""+mcdec+"""
output -f
"""


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


# Print cards
print_cards()
# set up
generate(process_dir=process_dir,runArgs=runArgs)
# run
outputDS=arrange_output(process_dir=process_dir,runArgs=runArgs,lhe_version=3,saveProcDir=True)  
check_reset_proc_number(opts)

## pythia shower
keyword=['SM','top', 'Wgamma', 'photon']
evgenConfig.generators += ["MadGraph", "Pythia8"]
evgenConfig.description = 'MadGraph_'+str(name)+'_GamFromDecay'
evgenConfig.keywords += keyword
evgenConfig.contact = ["arpan.ghosal@cern.ch"]

runArgs.inputGeneratorFile=outputDS+".events"
include("Pythia8_i/Pythia8_A14_NNPDF23LO_EvtGen_Common.py")
include("Pythia8_i/Pythia8_MadGraph.py")
include("Pythia8_i/Pythia8_ShowerWeights.py")
