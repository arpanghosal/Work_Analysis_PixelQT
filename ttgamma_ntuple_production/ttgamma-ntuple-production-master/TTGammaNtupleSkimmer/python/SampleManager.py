#!/usr/bin/env python

import sys
import re

# This file defines a couple of classes to allow easy managing of
# samples. Let's be clear about the naming conventions used in
# this file:
#   - dataset: the full identifier of the dataset container
#   - DSID: the 6-digit number associated to a dataset
#   - sample type: a group of datasets, associated to a certain type of process


# List of reconstruction tags associated with the MC16a, MC16d
# and MC16e production campaigns. These are used to identify to
# which campaign a datset belongs to.
MC16a_tags = ['r9364']
MC16d_tags = ['r10201']
MC16e_tags = ['r10724']


# Class to access information about a dataset. This class is
# intialised by giving it a string with the name of a dataset
# container.
class Dataset():
    def __init__(self, string):
        self.string = string

    def __repr__(self):
        return self.string

    def __str__(self):
        return self.string

    # Get the DSID of the dataset.
    def DSID(self):
        match = re.search("\.[0-9]{6}\.", self.string)
        if not match:
            return ""
        else:
            return match.group()[1:-1]

    # Return whether this dataset is of type data.
    def IsData(self):
        if 'AllYear' in self.string: return True
        return False

    # Return whether this dataset is part of MC16a.
    def IsMC16a(self):
        for tag in self.Rtags():
            if tag in MC16a_tags: return True
        return False

    # Return whether this dataset is part of MC16d.
    def IsMC16d(self):
        for tag in self.Rtags():
            if tag in MC16d_tags: return True
        return False

    # Return whether this dataset is part of MC16e.
    def IsMC16e(self):
        for tag in self.Rtags():
            if tag in MC16e_tags: return True
        return False

    # Get the full dataset container name.
    def Name(self):
        return self.string

    # Get the simulation tags of the dataset.
    def Stags(self):
        tags = set(re.findall("[as][0-9]+_", self.string))
        return [tag[:-1] for tag in tags]

    # Get the _first_ simulation tag. All the others usually
    # refer to merging and are not relevant.
    def Stag(self):
        if len(self.Stags()) == 0: return ""
        return self.Stags()[0]

    # Get the reconstruction tags of the dataset.
    def Rtags(self):
        tags = set(re.findall("r[0-9]+_", self.string))
        return [tag[:-1] for tag in tags]

    # Get the _first_ reconstruction tag. All the others usually
    # refer to merging and are not relevant.
    def Rtag(self):        
        if len(self.Rtags()) == 0: return ""
        if len(self.Rtags()) == 2 and self.Rtags()[0]=="r10726" and self.Rtags()[1]=="r10724": return self.Rtags()[1] #needed for Vgamma mc16e samples as for somereason the r10726 tag would be taken otherwise and the skimming would stop
        return self.Rtags()[0]


# Class to hold a collection of datasets that all belong to the
# same sample type. Essentially a bunch of accessors for the
# dataset information. The class is initialised given a string
# for the sample type and a list of strings for the dataset
# containers.
class SampleType():
    def __init__(self, name, ds_strings = []):
        self.name_ = name
        self.datasets_ = [Dataset(d) for d in ds_strings]

    def __repr__(self):
        return self.name_

    def __str__(self):
        return self.name_

    # Get the list of datasets.
    def Datasets(self):
        return self.datasets_

    # Get the set of DSIDs.
    def DSIDs(self):
        dsids = set()
        [dsids.add(d.DSID()) for d in self.datasets_]
        return dsids

    # Return whether the sample type is for fake photons only. In
    # consequence, prompt photons should be removed to avoid
    # sample overlap with a dedicated prompt photon sample.
    def ContainsFakes(self):
        match = re.search('ttbar|Wjets|Zjets', self.Name())
        if match: return True
        return False

    # Return whether this is a dedicated prompt photon sample
    # type. In that case, fake photons should be removed.
    def ContainsPrompt(self):
        match = re.search('ttgamma|Wgamma|Zgamma', self.Name())
        if match: return True
        return False

    # Get the name of this sample type.
    def Name(self):
        return self.name_

    # Get the set of Stags.
    def Stags(self):
        tags = set()
        [tags.add(d.Stag()) for d in self.datasets_]
        return tags

    # Get the set of Rtags.
    def Rtags(self):
        tags = set()
        [tags.add(d.Rtag()) for d in self.datasets_]
        return tags


# A short class to manage various sample types. The sample types
# get registered at the beginning and then this manager can be
# used to look up datasets, DSIDs etc., e.g. to identify to which
# sample type a dataset belongs to.
class SampleManager():
    def __init__(self):
        self.samples = []

    # Add a type of sample to the manager, provided a sample name
    # and a list of datasets.
    def AddSample(self, sample, ds_strings):
        self.samples.append(SampleType(sample, ds_strings))

    # Check which sample type a dataset belongs to.
    def GetSampleType(self, ds_string):
        ds = Dataset(ds_string)
        if ds.IsData():
            match = re.search("\.grp[0-9]{2}", ds.Name())
            if not match: return ""
            return "data.AllYear.20%s" % match.group()[4:]
        for sample in self.samples:
            if not ds.DSID() in sample.DSIDs():
		continue
            if not ds.Stag() in sample.Stags():
		continue
            if not ds.Rtag() in sample.Rtags(): 
		continue
            return sample.Name()
        return ""

    # Get all DSIDs associated to a sample type; returns a list.
    def GetDSIDsForSampleType(self, sample):
        if not sample in self.samples:
            print "SampleType %s not registered. Exiting" % sample
            sys.exit(1)
        dsids = []
        for ds in self.samples[sample]:
            dsids.append(ds.DSID())
        return dsids

    def GetShortTitle(self, ds, tag = ''):
        type = self.GetSampleType(ds)
        dsid = Dataset(ds).DSID()
        if type and dsid and tag:
            return "%s_%s.%s.root" % (type, tag, dsid)
        elif type and dsid:
            return "%s.%s.root" % (type, dsid)
        elif type:
            return "%s.root" % (type)
        else:
            print "Could not shorten dataset: %s" % ds
            sys.exit(1)
