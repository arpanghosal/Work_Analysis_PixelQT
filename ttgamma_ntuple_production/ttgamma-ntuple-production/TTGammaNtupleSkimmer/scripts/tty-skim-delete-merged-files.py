#!/usr/bin/env python

# This script assumes that the tty-skimming-batch.py script and
# the tty-skimming-merge-files.py script were already run. As a
# final step, this script goes through the directory of skimmed
# ntuples and picks those files, that were merged together to
# mark them for deletion. The script takes the following
# argument:
#  --path ARG         path to the skimmed ntuples
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
        regex_dsid = re.compile("\.[0-9]{6}.*")
        regex_merged = re.compile("\.merged.*")
        for f in files:
            result = regex_dsid.search(f)
            if not result: result = regex_merged.search(f)
            if not result: continue
            sample_type = f.replace(result.group(0), '')
            if not sample_type in dictionary: dictionary[sample_type] = []
            dictionary[sample_type].append(f)
    for entry in dictionary:
        print "Found %s files of sample type %s" % (len(dictionary[entry]), entry)
    print ''
    return dictionary


def delete_merged_files(args, dictionary):
    proc_manager = ProcessManager.ProcessManager(1)
    for entry in dictionary:
        proc_manager.waitTillReady()
        if not "%s.merged.root" % entry in dictionary[entry]:
            print "Skip deletion for sample type %s: no merged file" % entry
            continue
        if len(dictionary[entry]) < 2:
            print "Skip deletion for sample type %s: one or less files available" % entry
            continue
        command = "rm -f"
        file_count = 0
        for f in dictionary[entry]:
            if f == "%s.merged.root" % entry: continue
            command += " \\\n %s%s" % (args.path, f)
            file_count += 1
        if args.dry:
            print '\n' + command + '\n'
        else:
            print "Deleting %s files of type: %s" % (file_count, entry)
            proc_manager.addProcess(command)

    # Wait for all processes to finish before continuing.
    proc_manager.finalise()




if __name__ == "__main__":
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("-d", "--dry", help="make a dry run and only print the commands", action="store_true")
    parser.add_argument("--path", help="path to the skimmed ntuples", required=True)
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

    delete_merged_files(args, dictionary)

    sys.exit(0)
