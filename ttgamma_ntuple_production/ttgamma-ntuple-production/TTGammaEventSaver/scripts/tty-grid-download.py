#!/usr/bin/env python

import os
import TopExamples.grid

ntuple_version = os.getenv('TTGAMMA_NTUPLE_VERSION', 'test-ntuple')
ntuple_location = os.getenv('TTGAMMA_NTUPLE_LOCATION', '/tmp/ntuples')
user_name = os.getenv('RUCIO_ACCOUNT', 'ankirchh')

scope          = 'user.' + user_name
datasetPattern = '*' + ntuple_version + '*_output_root'
directory      = ntuple_location + '/' + ntuple_version

print "Searching for patterns %s in scope %s" % (datasetPattern, scope)
print "Downloading to directory %s" % directory

TopExamples.grid.download(scope, datasetPattern, directory)
