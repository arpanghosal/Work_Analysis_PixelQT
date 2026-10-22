#!/usr/bin/env python

# A short script that takes the samples registered in
# MC16a_TOPQ1, MC16d_TOPQ1 and performs a test whether their
# p-tags are the most recent or not. They interact with AMI
# through the AtlasAPI.

import sys
import TopExamples.ami
import TopExamples.grid
from TTGammaEventSaver import MC16a_TOPQ1, MC16d_TOPQ1, MC16e_TOPQ1, Data_rel21


###############################################################################

if __name__ == "__main__":
    TopExamples.ami.check_sample_status(TopExamples.grid.Samples(Data_rel21.samples))
    TopExamples.ami.check_sample_status(TopExamples.grid.Samples(MC16a_TOPQ1.samples))
    TopExamples.ami.check_sample_status(TopExamples.grid.Samples(MC16d_TOPQ1.samples))
    TopExamples.ami.check_sample_status(TopExamples.grid.Samples(MC16e_TOPQ1.samples))

    # Finalise and exit.
    sys.exit(0)
