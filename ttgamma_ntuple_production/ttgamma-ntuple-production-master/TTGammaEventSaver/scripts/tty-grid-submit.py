#!/usr/bin/env python

###############################################################################
### Example: In build directory,
###
###           tty-grid-submit.py -typ reco -camp a -ch ljets -submit -trial 0 -OneFilePerJob
###
###############################################################################
# Make sure the cut file is in the build directory.


import os
import sys
from datetime import datetime

import TopExamples.grid
from TTGammaEventSaver import Data_rel21, MC16a_TOPQ1, MC16d_TOPQ1, MC16e_TOPQ1
import argparse

###############################################################################

# Add a time stamp and the ntuple version to the jobs.
now = datetime.now()
timestamp = '%s-%s-%s' % (now.year, now.strftime('%m'), now.strftime('%d'))
ntuple_version = os.getenv('TTGAMMA_NTUPLE_VERSION', 'test-ntuple')
user_name = os.getenv('RUCIO_ACCOUNT', 'ankirchh')

p = argparse.ArgumentParser()
p.add_argument('-typ', '--datatyp', type=str, help='Name of the datatype: "reco", "truth", "data", "topo"', default="reco")
p.add_argument('-camp', '--campaign', type=str, help='campaign name: "a", "d", "e"')
p.add_argument('-ch', '--channel', type=str, help='channel name: "ljets", "dilep_CR", "CR_hfakes"')
p.add_argument('-submit', '--submit_permission', help='Submit Permission', action='store_true')
p.add_argument('-trial', '--trial', type=str, default='0')
p.add_argument('-OneFilePerJob', '--OneFilePerJob', help='Submit 1 file per job', action='store_true')
args=p.parse_args()

datatype=''
if args.datatyp!='reco':
  datatype = args.datatyp
else:
  datatype = 'reco'

dont_submit = True
if args.submit_permission:
  dont_submit = False

campaign = args.campaign
channel = args.channel
trial = args.trial

oneFilePerJob = False
if args.OneFilePerJob:
    oneFilePerJob = True
  

cutfile = 'cutfiles/cutfile_%s_%s_%s.txt' % (datatype, campaign, channel)
if not os.path.isfile(cutfile):
    print "Cutfile \"%s\" not found. Please copy it into this directory:" % cutfile
    print "  --> cp ../ttgamma-ntuple-production/TTGammaEventSaver/config/%s" % cutfile + " ./%s" % cutfile
    sys.exit(1)
###############################################################################

config = TopExamples.grid.Config()
config.code          = 'top-xaod'
config.settingsFile  = cutfile
config.gridUsername  = user_name
config.suffix        = ntuple_version + '.' + timestamp + '_' + channel + '_' + trial
config.excludedSites = ''
config.noSubmit      = dont_submit
config.mergeType     = 'Default' #'None', 'Default' or 'xAOD'
config.destSE        = ''        #This is the default (anywhere), or try e.g. 'UKI-SOUTHGRID-BHAM-HEP_LOCALGROUPDISK'
config.reuseTarBall  = True 
config.skipShowerCheck = True
if datatype == 'truth':
  config.memory        = '3000'  # By default set to 2GB
if datatype == 'truth' or oneFilePerJob:
  config.maxNFilesPerJob	= '1'	# only to be used for truth

###############################################################################

samples_data15 = [
    'Data15_TOPQ1',
]
samples_data16 = [
    'Data16_TOPQ1',
]
samples_data17 = [
    'Data17_TOPQ1',
]
samples_data18 = [
    'Data18_TOPQ1',
]

###############################################################################

samples_mc16a_signal_ljets = [
    'mc16a_TOPQ1_ttgamma_NLO_prod_old',
    'mc16a_TOPQ1_ttgamma_LO_dec_old',
    'mc16a_TOPQ1_ttgamma_nominal',
    'mc16a_TOPQ1_ttgamma_NLO_prod',
    'mc16a_TOPQ1_ttgamma_LO_dec',
]

samples_mc16a_ljets = [
    'mc16a_TOPQ1_ttbar_FS',
    'mc16a_TOPQ1_singletop_schan',
    'mc16a_TOPQ1_singletop_Wt_inclusive_DR',
    'mc16a_TOPQ1_Wty_nominal',
    'mc16a_TOPQ1_singletop_tchan',
    'mc16a_TOPQ1_Zgamma',
    'mc16a_TOPQ1_Zjets',
    'mc16a_TOPQ1_Wgamma',
    'mc16a_TOPQ1_Wjets',
    'mc16a_TOPQ1_diboson',
    'mc16a_TOPQ1_ttV',
    'mc16a_TOPQ1_Vgamma',
]

samples_mc16a_modelling_ljets = [
    'mc16a_TOPQ1_ttgamma_NLO_prod_H7',
    'mc16a_TOPQ1_ttgamma_NLO_prod_var3cUp',
    'mc16a_TOPQ1_ttgamma_NLO_prod_var3cDown',
    'mc16a_TOPQ1_ttgamma_LO_dec_H7',
    'mc16a_TOPQ1_ttgamma_AFII',
    'mc16a_TOPQ1_ttgamma_Herwig7',
    'mc16a_TOPQ1_ttgamma_radiation_up',
    'mc16a_TOPQ1_ttgamma_radiation_down',
    'mc16a_TOPQ1_ttbar_AFII',
    'mc16a_TOPQ1_ttbar_hdamp_var',
    'mc16a_TOPQ1_ttbar_aMC',
    'mc16a_TOPQ1_ttbar_Herwig7',
    'mc16a_TOPQ1_singletop_Wt_inclusive_AFII',
    'mc16a_TOPQ1_singletop_Wt_inclusive_DS',
    'mc16a_TOPQ1_singletop_Wt_inclusive_MadGraph',
    'mc16a_TOPQ1_singletop_Wt_inclusive_Herwig7',
    'mc16a_TOPQ1_Wty_Herwig7',
]

samples_mc16a_EFT_ljets = [
    'mc16a_TOPQ1_ttgamma_NLO_prod_EFT_0',
    'mc16a_TOPQ1_ttgamma_NLO_prod_EFT_CtG_n1',
    'mc16a_TOPQ1_ttgamma_NLO_prod_EFT_CtG_p1',
    'mc16a_TOPQ1_ttgamma_NLO_prod_EFT_CtB_n1',
    'mc16a_TOPQ1_ttgamma_NLO_prod_EFT_CtB_p1',
    'mc16a_TOPQ1_ttgamma_NLO_prod_EFT_CtW_n1',
    'mc16a_TOPQ1_ttgamma_NLO_prod_EFT_CtW_p1',
]

samples_mc16a_signal_dilep = [
    'mc16a_TOPQ1_ttgamma_NLO_prod_old',
    'mc16a_TOPQ1_ttgamma_LO_dec_old',
    'mc16a_TOPQ1_ttgamma_nominal',
    'mc16a_TOPQ1_ttgamma_NLO_prod',
    'mc16a_TOPQ1_ttgamma_LO_dec',
]

samples_mc16a_dilep = [
    'mc16a_TOPQ1_ttbar_dilepton_FS',
    'mc16a_TOPQ1_singletop_schan',
    'mc16a_TOPQ1_singletop_Wt_dilepton_DR',
    'mc16a_TOPQ1_Wty_nominal',
    'mc16a_TOPQ1_singletop_tchan',
    'mc16a_TOPQ1_Zgamma',
    'mc16a_TOPQ1_Zjets',
    'mc16a_TOPQ1_Wgamma',
    'mc16a_TOPQ1_Wjets',
    'mc16a_TOPQ1_diboson',
    'mc16a_TOPQ1_ttV',
    'mc16a_TOPQ1_Vgamma',
]

samples_mc16a_modelling_dilep = [
    'mc16a_TOPQ1_ttgamma_NLO_prod_H7',
    'mc16a_TOPQ1_ttgamma_NLO_prod_var3cUp',
    'mc16a_TOPQ1_ttgamma_NLO_prod_var3cDown',
    'mc16a_TOPQ1_ttgamma_LO_dec_H7',
    'mc16a_TOPQ1_ttgamma_AFII',
    'mc16a_TOPQ1_ttgamma_Herwig7',
    'mc16a_TOPQ1_ttgamma_radiation_up',
    'mc16a_TOPQ1_ttgamma_radiation_down',
    'mc16a_TOPQ1_ttbar_dilepton_AFII',
    'mc16a_TOPQ1_ttbar_dilepton_hdamp_var',
    'mc16a_TOPQ1_ttbar_dilepton_aMC',
    'mc16a_TOPQ1_ttbar_dilepton_Herwig7',
    'mc16a_TOPQ1_singletop_Wt_dilepton_AFII',
    'mc16a_TOPQ1_singletop_Wt_dilepton_DS',
    'mc16a_TOPQ1_singletop_Wt_dilepton_MadGraph',
    'mc16a_TOPQ1_singletop_Wt_dilepton_Herwig7',
    'mc16a_TOPQ1_Wty_Herwig7',
]

###############################################################################

samples_mc16d_signal_ljets = [
    'mc16d_TOPQ1_ttgamma_NLO_prod_old',
    'mc16d_TOPQ1_ttgamma_LO_dec_old',
    'mc16d_TOPQ1_ttgamma_nominal',
    'mc16d_TOPQ1_ttgamma_NLO_prod',
    'mc16d_TOPQ1_ttgamma_LO_dec',
]

samples_mc16d_ljets = [
    'mc16d_TOPQ1_ttbar_FS',
    'mc16d_TOPQ1_singletop_schan',
    'mc16d_TOPQ1_singletop_Wt_inclusive_DR',
    'mc16d_TOPQ1_Wty_nominal',
    'mc16d_TOPQ1_singletop_tchan',
    'mc16d_TOPQ1_Zgamma',
    'mc16d_TOPQ1_Zjets',
    'mc16d_TOPQ1_Wgamma',
    'mc16d_TOPQ1_Wjets',
    'mc16d_TOPQ1_diboson',
    'mc16d_TOPQ1_ttV',
    'mc16d_TOPQ1_Vgamma',
]


samples_mc16d_modelling_ljets = [
    'mc16d_TOPQ1_ttgamma_NLO_prod_H7',
    'mc16d_TOPQ1_ttgamma_NLO_prod_var3cUp',
    'mc16d_TOPQ1_ttgamma_NLO_prod_var3cDown',
    'mc16d_TOPQ1_ttgamma_LO_dec_H7',
    'mc16d_TOPQ1_ttgamma_AFII',
    'mc16d_TOPQ1_ttgamma_Herwig7',
    'mc16d_TOPQ1_ttgamma_radiation_up',
    'mc16d_TOPQ1_ttgamma_radiation_down',
    'mc16d_TOPQ1_ttbar_AFII',
    'mc16d_TOPQ1_ttbar_hdamp_var',
    'mc16d_TOPQ1_ttbar_aMC',
    'mc16d_TOPQ1_ttbar_Herwig7',
    'mc16d_TOPQ1_singletop_Wt_inclusive_AFII',
    'mc16d_TOPQ1_singletop_Wt_inclusive_DS',
    'mc16d_TOPQ1_singletop_Wt_inclusive_MadGraph',
    'mc16d_TOPQ1_singletop_Wt_inclusive_Herwig7',
    'mc16d_TOPQ1_Wty_Herwig7',
]

samples_mc16d_EFT_ljets = [
    'mc16d_TOPQ1_ttgamma_NLO_prod_EFT_0',
    'mc16d_TOPQ1_ttgamma_NLO_prod_EFT_CtG_n1',
    'mc16d_TOPQ1_ttgamma_NLO_prod_EFT_CtG_p1',
    'mc16d_TOPQ1_ttgamma_NLO_prod_EFT_CtB_n1',
    'mc16d_TOPQ1_ttgamma_NLO_prod_EFT_CtB_p1',
    'mc16d_TOPQ1_ttgamma_NLO_prod_EFT_CtW_n1',
    'mc16d_TOPQ1_ttgamma_NLO_prod_EFT_CtW_p1',
]

samples_mc16d_signal_dilep = [
    'mc16d_TOPQ1_ttgamma_NLO_prod_old',
    'mc16d_TOPQ1_ttgamma_LO_dec_old',
    'mc16d_TOPQ1_ttgamma_nominal',
    'mc16d_TOPQ1_ttgamma_NLO_prod',
    'mc16d_TOPQ1_ttgamma_LO_dec',
]

samples_mc16d_dilep = [
    'mc16d_TOPQ1_ttbar_dilepton_FS',
    'mc16d_TOPQ1_singletop_schan',
    'mc16d_TOPQ1_singletop_Wt_dilepton_DR',
    'mc16d_TOPQ1_Wty_nominal',
    'mc16d_TOPQ1_singletop_tchan',
    'mc16d_TOPQ1_Zgamma',
    'mc16d_TOPQ1_Zjets',
    'mc16d_TOPQ1_Wgamma',
    'mc16d_TOPQ1_Wjets',
    'mc16d_TOPQ1_diboson',
    'mc16d_TOPQ1_ttV',
    'mc16d_TOPQ1_Vgamma',
]

samples_mc16d_modelling_dilep = [
    'mc16d_TOPQ1_ttgamma_NLO_prod_H7',
    'mc16d_TOPQ1_ttgamma_NLO_prod_var3cUp',
    'mc16d_TOPQ1_ttgamma_NLO_prod_var3cDown',
    'mc16d_TOPQ1_ttgamma_LO_dec_H7',
    'mc16d_TOPQ1_ttgamma_AFII',
    'mc16d_TOPQ1_ttgamma_Herwig7',
    'mc16d_TOPQ1_ttgamma_radiation_up',
    'mc16d_TOPQ1_ttgamma_radiation_down',
    'mc16d_TOPQ1_ttbar_dilepton_AFII',
    'mc16d_TOPQ1_ttbar_dilepton_hdamp_var',
    'mc16d_TOPQ1_ttbar_dilepton_aMC',
    'mc16d_TOPQ1_ttbar_dilepton_Herwig7',
    'mc16d_TOPQ1_singletop_Wt_dilepton_AFII',
    'mc16d_TOPQ1_singletop_Wt_dilepton_DS',
    'mc16d_TOPQ1_singletop_Wt_dilepton_MadGraph',
    'mc16d_TOPQ1_singletop_Wt_dilepton_Herwig7',
    'mc16d_TOPQ1_Wty_Herwig7',
]

###############################################################################

samples_mc16e_signal_ljets = [
    'mc16e_TOPQ1_ttgamma_NLO_prod_old',
    'mc16e_TOPQ1_ttgamma_LO_dec_old',
    'mc16e_TOPQ1_ttgamma_nominal',
    'mc16e_TOPQ1_ttgamma_NLO_prod',
    'mc16e_TOPQ1_ttgamma_LO_dec',
]

samples_mc16e_ljets = [
    'mc16e_TOPQ1_ttbar_FS',
    'mc16e_TOPQ1_singletop_schan',
    'mc16e_TOPQ1_singletop_Wt_inclusive_DR',
    'mc16e_TOPQ1_Wty_nominal',
    'mc16e_TOPQ1_singletop_tchan',
    'mc16e_TOPQ1_Zgamma',
    'mc16e_TOPQ1_Zjets',
    'mc16e_TOPQ1_Wgamma',
    'mc16e_TOPQ1_Wjets',
    'mc16e_TOPQ1_diboson',
    'mc16e_TOPQ1_ttV',
    'mc16e_TOPQ1_Vgamma',
]

samples_mc16e_modelling_ljets = [
    'mc16e_TOPQ1_ttgamma_NLO_prod_H7',
    'mc16e_TOPQ1_ttgamma_NLO_prod_var3cUp',
    'mc16e_TOPQ1_ttgamma_NLO_prod_var3cDown',
    'mc16e_TOPQ1_ttgamma_LO_dec_H7',
    'mc16e_TOPQ1_ttgamma_AFII',
    'mc16e_TOPQ1_ttgamma_Herwig7',
    'mc16e_TOPQ1_ttgamma_radiation_up',
    'mc16e_TOPQ1_ttgamma_radiation_down',
    'mc16e_TOPQ1_ttbar_AFII',
    'mc16e_TOPQ1_ttbar_hdamp_var',
    'mc16e_TOPQ1_ttbar_aMC',
    'mc16e_TOPQ1_ttbar_Herwig7',
    'mc16e_TOPQ1_singletop_Wt_inclusive_AFII',
    'mc16e_TOPQ1_singletop_Wt_inclusive_DS',
    'mc16e_TOPQ1_singletop_Wt_inclusive_MadGraph',
    'mc16e_TOPQ1_singletop_Wt_inclusive_Herwig7',
    'mc16e_TOPQ1_Wty_Herwig7',
]

samples_mc16e_EFT_ljets = [
    'mc16e_TOPQ1_ttgamma_NLO_prod_EFT_0',
    'mc16e_TOPQ1_ttgamma_NLO_prod_EFT_CtG_n1',
    'mc16e_TOPQ1_ttgamma_NLO_prod_EFT_CtG_p1',
    'mc16e_TOPQ1_ttgamma_NLO_prod_EFT_CtB_n1',
    'mc16e_TOPQ1_ttgamma_NLO_prod_EFT_CtB_p1',
    'mc16e_TOPQ1_ttgamma_NLO_prod_EFT_CtW_n1',
    'mc16e_TOPQ1_ttgamma_NLO_prod_EFT_CtW_p1',
]

samples_mc16e_signal_dilep = [
    'mc16e_TOPQ1_ttgamma_NLO_prod_old',
    'mc16e_TOPQ1_ttgamma_LO_dec_old',
    'mc16e_TOPQ1_ttgamma_nominal',
    'mc16e_TOPQ1_ttgamma_NLO_prod',
    'mc16e_TOPQ1_ttgamma_LO_dec',
]

samples_mc16e_dilep = [
    'mc16e_TOPQ1_ttbar_dilepton_FS',
    'mc16e_TOPQ1_singletop_schan',
    'mc16e_TOPQ1_singletop_Wt_dilepton_DR',
    'mc16e_TOPQ1_Wty_nominal',
    'mc16e_TOPQ1_singletop_tchan',
    'mc16e_TOPQ1_Zgamma',
    'mc16e_TOPQ1_Zjets',
    'mc16e_TOPQ1_Wgamma',
    'mc16e_TOPQ1_Wjets',
    'mc16e_TOPQ1_diboson',
    'mc16e_TOPQ1_ttV',
    'mc16e_TOPQ1_Vgamma',
]

samples_mc16e_modelling_dilep = [
    'mc16e_TOPQ1_ttgamma_NLO_prod_H7',
    'mc16e_TOPQ1_ttgamma_NLO_prod_var3cUp',
    'mc16e_TOPQ1_ttgamma_NLO_prod_var3cDown',
    'mc16e_TOPQ1_ttgamma_LO_dec_H7',
    'mc16e_TOPQ1_ttgamma_AFII',
    'mc16e_TOPQ1_ttgamma_Herwig7',
    'mc16e_TOPQ1_ttgamma_radiation_up',
    'mc16e_TOPQ1_ttgamma_radiation_down',
    'mc16e_TOPQ1_ttbar_dilepton_AFII',
    'mc16e_TOPQ1_ttbar_dilepton_hdamp_var',
    'mc16e_TOPQ1_ttbar_dilepton_aMC',
    'mc16e_TOPQ1_ttbar_dilepton_Herwig7',
    'mc16e_TOPQ1_singletop_Wt_dilepton_AFII',
    'mc16e_TOPQ1_singletop_Wt_dilepton_DS',
    'mc16e_TOPQ1_singletop_Wt_dilepton_MadGraph',
    'mc16e_TOPQ1_singletop_Wt_dilepton_Herwig7',
    'mc16e_TOPQ1_Wty_Herwig7',
]

###############################################################################

# Uncomment the following lines to do the actual submission.
# Also, don't forget to turn off the "no-submit" option at the
# beginning of the file.

if datatype == 'data':
  if campaign == 'a':
    TopExamples.grid.submit(config, TopExamples.grid.Samples(samples_data15))
    TopExamples.grid.submit(config, TopExamples.grid.Samples(samples_data16))
  elif campaign == 'd':
    TopExamples.grid.submit(config, TopExamples.grid.Samples(samples_data17))
  elif campaign == 'e':
    TopExamples.grid.submit(config, TopExamples.grid.Samples(samples_data18))

elif datatype == 'truth':
  if campaign == 'a' and channel=='ljets':
    TopExamples.grid.submit(config, TopExamples.grid.Samples(samples_mc16a_signal_ljets))
  elif campaign == 'a' and channel=='dilep_CR':
    TopExamples.grid.submit(config, TopExamples.grid.Samples(samples_mc16a_signal_dilep))
  elif campaign == 'd' and channel=='ljets':
    TopExamples.grid.submit(config, TopExamples.grid.Samples(samples_mc16d_signal_ljets))
  elif campaign == 'd' and channel=='dilep_CR':
    TopExamples.grid.submit(config, TopExamples.grid.Samples(samples_mc16d_signal_dilep))
  elif campaign == 'e' and channel=='ljets':
    TopExamples.grid.submit(config, TopExamples.grid.Samples(samples_mc16e_signal_ljets))
  elif campaign == 'e' and channel=='dilep_CR':
    TopExamples.grid.submit(config, TopExamples.grid.Samples(samples_mc16e_signal_dilep))

elif datatype == 'reco':
  if campaign == 'a' and (channel=='ljets' or channel=='CR_hfakes'):
    TopExamples.grid.submit(config, TopExamples.grid.Samples(samples_mc16a_ljets))
    TopExamples.grid.submit(config, TopExamples.grid.Samples(samples_mc16a_modelling_ljets))
    #TopExamples.grid.submit(config, TopExamples.grid.Samples(samples_mc16a_EFT_ljets))
    TopExamples.grid.submit(config, TopExamples.grid.Samples(samples_mc16a_signal_ljets))
  elif campaign == 'a' and channel=='dilep_CR':
    TopExamples.grid.submit(config, TopExamples.grid.Samples(samples_mc16a_dilep))
    TopExamples.grid.submit(config, TopExamples.grid.Samples(samples_mc16a_modelling_dilep))
    TopExamples.grid.submit(config, TopExamples.grid.Samples(samples_mc16a_signal_dilep))

  elif campaign == 'd' and (channel=='ljets' or channel=='CR_hfakes'):
    TopExamples.grid.submit(config, TopExamples.grid.Samples(samples_mc16d_ljets))
    TopExamples.grid.submit(config, TopExamples.grid.Samples(samples_mc16d_modelling_ljets))
    #TopExamples.grid.submit(config, TopExamples.grid.Samples(samples_mc16d_EFT_ljets))
    TopExamples.grid.submit(config, TopExamples.grid.Samples(samples_mc16d_signal_ljets))
  elif campaign == 'd' and channel=='dilep_CR':
    TopExamples.grid.submit(config, TopExamples.grid.Samples(samples_mc16d_dilep))
    TopExamples.grid.submit(config, TopExamples.grid.Samples(samples_mc16d_modelling_dilep))
    TopExamples.grid.submit(config, TopExamples.grid.Samples(samples_mc16d_signal_dilep))

  elif campaign == 'e' and (channel=='ljets' or channel=='CR_hfakes'):
    TopExamples.grid.submit(config, TopExamples.grid.Samples(samples_mc16e_ljets))
    TopExamples.grid.submit(config, TopExamples.grid.Samples(samples_mc16e_modelling_ljets))
    #TopExamples.grid.submit(config, TopExamples.grid.Samples(samples_mc16e_EFT_ljets))
    TopExamples.grid.submit(config, TopExamples.grid.Samples(samples_mc16e_signal_ljets))
  elif campaign == 'e' and  channel=='dilep_CR':
    TopExamples.grid.submit(config, TopExamples.grid.Samples(samples_mc16e_dilep))
    TopExamples.grid.submit(config, TopExamples.grid.Samples(samples_mc16e_modelling_dilep))
    TopExamples.grid.submit(config, TopExamples.grid.Samples(samples_mc16e_signal_dilep))

