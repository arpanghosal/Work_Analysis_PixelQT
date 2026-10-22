#!/usr/bin/env python

# A short script that takes the samples registered in MC16a_TOPQ1
# and retrieves their cross sections, generator filter
# efficiencies and k-factors from AMI. The values are then
# outputted as "DSID \tab xsec * filtEff \tab kfactor".

import re, os, sys
import pyAMI.client
import pyAMI.atlas.api as AtlasAPI
import TopExamples.grid
import TopExamples.logger as log
from TTGammaEventSaver import MC16a_TOPQ1


client = pyAMI.client.Client('atlas')
AtlasAPI.init()

###############################################################################

def catch_pyami_exception():
    print "Could not establish pyAMI session. Are you sure you have a valid certificate?"
    print " --> voms-proxy-init -voms atlas"
    sys.exit(1)

# Return the type of LDN we're looking at (EVNT, HITS, AOD, TOPQ1
# derivation etc.).
def get_type(ldn):
    match = re.search("\.deriv\.DAOD_TOPQ1\.", ldn)
    if match: return "DAOD_TOPQ1"
    # If none of the above, exit with an error.
    print "Could not identify type of LDN %s" % ldn
    sys.exit(1)

# Check whether an LDN is actually available and valid on AMI.
def is_available(ldn):
    try: results = AtlasAPI.list_datasets(client, patterns = [ldn], fields = ['ldn'])
    except pyAMI.exception.Error: catch_pyami_exception()
    for d in results:
        if d['ldn'] == ldn: return True
    return False

def get_dictionary(ldn):
    try: results = AtlasAPI.get_dataset_info(client, ldn)
    except pyAMI.exception.Error: catch_pyami_exception()
    return results[0]

###############################################################################

if __name__ == "__main__":
    samples = TopExamples.grid.Samples(MC16a_TOPQ1.samples)

    for s in samples:
        print log.WARNING + s.name + log.ENDC + " (%s %s)" % (len(s.datasets), "dataset" if len(s.datasets) == 1 else "datasets")

        for ldn in s.datasets:
            d = get_dictionary(ldn)
            if not d:
                print "No information available for LDN %s" % ldn
                continue

            info_incomplete = False
            if not 'crossSection@MCGN' in d: info_incomplete = True
            if not 'genFiltEff@MCGN' in d: info_incomplete = True

            xsec = 1.
            if 'crossSection@MCGN' in d: xsec = xsec * float(d['crossSection@MCGN'])
            if 'genFiltEff@MCGN' in d: xsec = xsec * float(d['genFiltEff@MCGN'])
            kfactor = 1.
            if 'kFactor@PMG' in d: kfactor = float(d['kFactor@PMG'])

            if info_incomplete:
                print "%s\t%s" % (d['datasetNumber'], "no information available")
            else:
                print "%s\t%s\t%s" % (d['datasetNumber'], xsec * 1e3, kfactor)

    # Finalise and exit.
    sys.exit(0)
