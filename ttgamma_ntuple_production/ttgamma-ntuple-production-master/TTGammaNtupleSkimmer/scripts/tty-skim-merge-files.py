#!/usr/bin/env python

# This script assumes that the tty-skimming-batch.py script was
# already run. As a follow-up step, this script can then be
# executed on top of the skimmed files to merge all of the same
# sample type into a single file. It takes the path as argument,
# and it also allows to set how many merge steps should be
# performed in parallel:

#  --path ARG         path to the skimmed ntuples
#  --processes ARG    number of parallel processes to run
#
# In addition, the following options are possible:
#  -d/--dry           make a dry run and only print the commands
#  -h/--help          show the help message


import os, sys, re
import argparse
from TTGammaNtupleSkimmer import ProcessManager, HelperFunctions


# Look up all files within a given path. Returns a dictionary of
# all sample types and the files associated with it. The files
# are expected to have the following naming pattern:
# SOME_SAMPLE_TYPE.xxxxxx.root with xxxxxx being the 6-digit DSID
# of the sample.
def lookup_files(path):
    dictionary = dict()
    for subdir, subsubdirs, files in os.walk(path):
        # Only consider bottom-level directories.
        if subsubdirs: continue

        # Strip away the DSIDs and anything after it from the
        # file name to create an appropriate dictionary entry.
        regex_dsid = re.compile("[0-9]{6}.*")
        for f in files:
            result = regex_dsid.search(f)
            if not result: continue
            sample_type = f.replace(result.group(0), '')
            if not sample_type in dictionary: dictionary[sample_type] = []
            dictionary[sample_type].append(f)
    for entry in dictionary:
        print "Found %s files of sample type %s" % (len(dictionary[entry]), entry)
    print ''
    return dictionary


# Take an input dictionary of sample types and files associated
# with them. If there are more than one file, create a
# SOME_SAMPLE_TYPE.merged.root output file by using the 'hadd'
# command from ROOT.
def merge_files(args, dictionary):
    proc_manager = ProcessManager.ProcessManager(args.processes)
    merged_files = []
    for entry in dictionary:
        proc_manager.waitTillReady()
        if len(dictionary[entry]) == 0: continue
        if len(dictionary[entry]) == 1:
            print "Skipping merge of single file %s" % dictionary[entry][0]
            continue

        command = "hadd -f"
        merge_file = "%s%s.merged.root" % (args.path, entry)
        merged_files.append(merge_file)
        command += " \\\n %s" % merge_file
        for f in dictionary[entry]:
            command += " \\\n %s%s" % (args.path, f)
        if args.dry:
            print '\n' + command + '\n'
        else:
            print "Merging all skimmed files of type: %s" % entry
            proc_manager.addProcess(command)

    # Wait for all processes to finish before continuing.
    proc_manager.finalise()

    return merged_files




if __name__ == "__main__":
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("-d", "--dry", help="make a dry run and only print the commands", action="store_true")
    parser.add_argument("--path", help="path to the skimmed ntuples", required=True)
    parser.add_argument("--processes", help="the number of parallel processes to run", type=int, default=4)
    args = parser.parse_args()


    # ======================================================
    # Step 1: perform a couple of safety checks with the input to
    # ensure "sanity" with what we're doing.
    args.path = HelperFunctions.ensure_trailing_slash(args.path)
    if not os.path.isdir(args.path):
        print "Input path does not exist. Exiting"
        sys.exit(1)


    # ======================================================
    # Step 2: go through the input path and create a dictionary
    # of all sample types and the files associated with them
    # (which are found under the given path).
    dictionary = lookup_files(args.path)


    # ======================================================
    # Step 3: take the created dictionary and perform the merging
    # step for all of its entries. If there is only one file
    # associated to an entry, don't perform merging. This returns
    # a list of all created merge files.
    merged_files = merge_files(args, dictionary)

    sys.exit(0)
