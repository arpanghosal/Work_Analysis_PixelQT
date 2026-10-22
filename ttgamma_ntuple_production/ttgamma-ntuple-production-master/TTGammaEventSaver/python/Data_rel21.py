# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

import TopExamples.grid
import TopExamples.ami
#import json

samples = dict()
samples["Data15_TOPQ1"] = ['data15_13TeV.AllYear.physics_Main.PhysCont.DAOD_TOPQ1.grp15_v01_p4513']
samples["Data16_TOPQ1"] = ['data16_13TeV.AllYear.physics_Main.PhysCont.DAOD_TOPQ1.grp16_v01_p4513']
samples["Data17_TOPQ1"] = ['data17_13TeV.AllYear.physics_Main.PhysCont.DAOD_TOPQ1.grp17_v01_p4513']
samples["Data18_TOPQ1"] = ['data18_13TeV.AllYear.physics_Main.PhysCont.DAOD_TOPQ1.grp18_v01_p4513']

for entry in samples:
    TopExamples.grid.Add(entry).datasets = [ds for ds in samples[entry]]
