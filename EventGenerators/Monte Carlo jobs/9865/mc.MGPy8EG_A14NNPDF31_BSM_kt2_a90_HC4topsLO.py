#--------------------------------------------------------------
# EVGEN configuration
#--------------------------------------------------------------
evgenConfig.generators += ["MadGraph"]
evgenConfig.keywords += ['BSM','top', '4top','LO']
evgenConfig.contact  = ["Hongtao.Yang@cern.ch"]
evgenConfig.generators += ["Pythia8"]
evgenConfig.description = 'Standard-Model 4tops production at LO with MadGraph5 and Pythia8'
evgenConfig.nEventsPerJob = 10000

# --------------------------------------------------------------
# Generate events
# --------------------------------------------------------------
from MadGraphControl.MadGraphUtils import *
from MadGraphControl.MadGraphParamHelpers import *
import fileinput

# General settings
nevents = runArgs.maxEvents*1.1 if runArgs.maxEvents>0 else 1.1*evgenConfig.nEventsPerJob

gridpack_mode=False

# PDF
pdflabel = 'lhapdf'
lhaid = 315000 # NNPDF31_lo_as_0118

# MadSpin
bwcut = 50

# Systematics
syst = "T"

parameters={
    'frblock':{
        'Lambda':'1.000000e+03',
        'cosa':  '0.000001e+00',
        'kSM':   '1000000.0',
        'kHtt':  '1.000000e+00',
        'kAtt':  '1.000000e+00',
        'kHll':  '0.000000e+00',
        'kAll':  '0.000000e+00',
        'kHaa':  '0.000000e+00',
        'kAaa':  '0.000000e+00',
        'kHza':  '0.000000e+00',
        'kAza':  '0.000000e+00',
        'kHzz':  '1.000000e+00',
        'kAzz':  '0.000000e+00',
        'kHww':  '1.000000e+00',
        'kAww':  '0.000000e+00',
        'kHda':  '0.000000e+00',
        'kHdz':  '0.000000e+00',
        'kHdwR':  '0.000000e+00',
        'kHdwI':  '0.000000e+00'},
    'yukawa':{
        'ymt':   '3.45000e+02'}
    }

#---------------------------------------------------------------------------
# MG5 Proc card
#---------------------------------------------------------------------------

mgproc = """generate p p > t t~ t t~ QED=99 QCD=99"""
topdecay = "decay t > w+ b, w+ > all all \ndecay t~ > w- b~, w- > all all \n"

process_string="""
import model HC_NLO_X0_UFO-no_b_mass
define p = g u c d s u~ c~ d~ s~
define j = g u c d s u~ c~ d~ s~
"""+mgproc+"""
output -f
"""

#---------------------------------------------------------------------------
# MG5 Run Card
#---------------------------------------------------------------------------

#Fetch default LO run_card.dat and set parameters
extras = {
    'dynamical_scale_choice':0,
    'lhe_version'  : '3.0',
    'pdlabel'      : pdflabel,
    'lhaid'        : lhaid,
    'use_syst'     : syst,
    'sys_scalefact': '1 0.5 2',
    'sys_pdf'      : "NNPDF31_lo_as_0118",
    'event_norm'   : "sum",
    'nevents'      : nevents,
    'bwcutoff'     : '50.0' #     ! (M+/-bwcutoff*Gamma)
    }

process_dir = new_process(process_string)
modify_run_card(runArgs=runArgs,
                process_dir=process_dir,
                settings=extras)
set_top_params(process_dir=process_dir)                     
modify_param_card(process_dir=process_dir,params=parameters)
#---------------------------------------------------------------------------
# Cook the setscales file for the user defined dynamical scale
#---------------------------------------------------------------------------

fileN = process_dir+'/SubProcesses/setscales.f'
mark  = '      elseif(dynamical_scale_choice.eq.0) then'
rmLines = ['ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc',
           'cc      USER-DEFINED SCALE: ENTER YOUR CODE HERE                                 cc',
           'cc      to use this code you must set                                            cc',
           'cc                 dynamical_scale_choice = 0                                    cc',
           'cc      in the run_card (run_card.dat)                                           cc',
           'write(*,*) "User-defined scale not set"',
           'stop 1',
           'temp_scale_id=\'User-defined dynamical scale\' ! use a meaningful string',
           'tmp = 0',
           'cc      USER-DEFINED SCALE: END OF USER CODE                                     cc',
           'stop 21',
           'rscale = 0'
           ]

flag=0
for line in fileinput.input(fileN, inplace=1):
    toKeep = True
    for rmLine in rmLines:
        if line.find(rmLine) >= 0:
           toKeep = False
           break
    if toKeep:
        print line,
    if line.startswith(mark) and flag==0:
        flag +=1
        print """
c         sum of the transverse mass divide by 4
c         m^2+pt^2=p(0)^2-p(3)^2=(p(0)+p(3))*(p(0)-p(3))
          rscale=0d0
          do i=3,nexternal
            rscale=rscale+dsqrt(max(0d0,(P(0,i)+P(3,i))*(P(0,i)-P(3,i))))
          enddo
          rscale=rscale/4d0
     """

#---------------------------------------------------------------------------
# MadSpin Card
#---------------------------------------------------------------------------

madspin_card_loc=process_dir+'/Cards/madspin_card.dat'
mscard = open(madspin_card_loc,'w')
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
set max_weight_ps_point 400  # number of PS to estimate the maximum for each event
set Nevents_for_max_weight 75
set BW_cut %i
set seed %i
%s
launch
"""%(bwcut, runArgs.randomSeed, topdecay))
mscard.close()

#---------------------------------------------------------------------------
# MG5 Generation
#---------------------------------------------------------------------------
generate(process_dir=process_dir,runArgs=runArgs,grid_pack=gridpack_mode)

outputDS = arrange_output(process_dir=process_dir,
                   runArgs=runArgs,
                   lhe_version=3)

#---------------------------------------------------------------------------
# Parton Showering Generation
#---------------------------------------------------------------------------
check_reset_proc_number(opts)

runArgs.inputGeneratorFile=outputDS

include("Pythia8_i/Pythia8_A14_NNPDF23LO_EvtGen_Common.py")
include("Pythia8_i/Pythia8_MadGraph.py")
