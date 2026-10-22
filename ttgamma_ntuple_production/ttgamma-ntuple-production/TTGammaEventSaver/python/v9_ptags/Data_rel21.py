# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

import TopExamples.grid
import TopExamples.ami

samples = dict()
samples["Data15_TOPQ1"] = ['data15_13TeV.periodAllYear.physics_Main.PhysCont.DAOD_TOPQ1.grp15_v01_p4030']
samples["Data16_TOPQ1"] = ['data16_13TeV.periodAllYear.physics_Main.PhysCont.DAOD_TOPQ1.grp16_v01_p4030']
samples["Data17_TOPQ1"] = ['data17_13TeV.periodAllYear.physics_Main.PhysCont.DAOD_TOPQ1.grp17_v01_p4030']
samples["Data18_TOPQ1"] = ['data18_13TeV.periodAllYear.physics_Main.PhysCont.DAOD_TOPQ1.grp18_v01_p4030']

for entry in samples:
    TopExamples.grid.Add(entry).datasets = [ds for ds in samples[entry]]
