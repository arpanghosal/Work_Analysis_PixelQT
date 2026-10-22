#!/usr/bin/env python

# This script is essentially a wrapper around the main executable of the
# TTGammaNtupleSkimmer package. It allows to specify a head directory only, and
# it will pick up all files in that directory and start the skimming process.
# For executing this batch processing, specify the following options:
#  -o/--output-path ARG      output path for the skimmed ntuples
#  -i/--input-path ARG       input path for the unskimmed ntuples
#  -c/--config-file ARG      location of the configuration file
#
# In addition, the following options are possible:
#  -d/--dry                  make a dry run and only print the commands
#  -h/--help                 show the help message


import os, sys, re
import argparse
from TTGammaNtupleSkimmer import ProcessManager, SampleManager, HelperFunctions
from TTGammaEventSaver import MC16a_TOPQ1, MC16d_TOPQ1, MC16e_TOPQ1, Data_rel21

# Look up all files within a given path. Returns a dictionary of
# all subdirectories and their files.
def create_input_dictionary(path, wildcard):
    dictionary = dict()
    for subdir, subsubdirs, files in os.walk(path):
        # Only consider bottom-level directories.
        if subsubdirs: continue
        if not wildcard == '':
            match = re.search(wildcard, subdir)
            if not match: continue

        # Strip main path from the directory.
        subdir = subdir[len(path):]
        subdir = HelperFunctions.remove_leading_slash(subdir)
        subdir = HelperFunctions.ensure_trailing_slash(subdir)
        dictionary[subdir] = [subdir + f for f in files]
        print "Found %s files in input directory %s" % (len(dictionary[subdir]), subdir)
    print "Created dictionary with entries for %s dataset containers" % len(dictionary)
    return dictionary


# Get the correct luminosity for a dataset. This takes into
# account, which MC campaign the dataset belongs to, and also
# whether the dataset exists both in MC16a and MC16d. Luminosity
# values are taken from:
# https://twiki.cern.ch/twiki/bin/view/AtlasProtected/TopDerivationMC16aList
def get_luminosity(manager, ds):    # total: 139.0/fb
    lumi_mc16a = 3219.56 + 32988.1  # mc16a:  36.2/fb
    lumi_mc16d = 44307.4            # mc16d:  44.3/fb
    lumi_mc16e = 58450.1            # mc16e:  58.5/fb
    if SampleManager.Dataset(ds).IsMC16a():
        return lumi_mc16a
    if SampleManager.Dataset(ds).IsMC16d():
        return lumi_mc16d
    else:
        return lumi_mc16e


# Function to decide whether events from the sample-overlap
# region should be removed or not. This is the case for all
# inclusive samples X, where we have additional dedicated samples
# to simulate X+gamma. I.e. this concerns ttbar, Z+jets, W+jets.
def veto_sample_overlap_region(sample):
    if "ttbar" in sample: return True
    if "Zjets" in sample: return True
    if "Wjets" in sample: return True
    return False


# append some string in a filename
# Here string is a int and this function
# appends it just before the extension
def append_id(filename, string):
    parts = filename.split('.')
    return "".join(parts[:-1])+ '.' + str(string) + '.' + parts[-1]

# Take an input dictionary of dataset and file lists associated
# with it, and perform the skimming and slimming of the input
# files. This creates exactly one output file per dataset. The
# function returns a dictionary of process types and the created
# output files that belong to it.
def perform_skimming_step(args, input_dictionary):
    sample_manager = SampleManager.SampleManager()
    for s in MC16a_TOPQ1.samples:
        sample_manager.AddSample(s, MC16a_TOPQ1.samples[s])
    for s in MC16d_TOPQ1.samples:
        sample_manager.AddSample(s, MC16d_TOPQ1.samples[s])
    for s in MC16e_TOPQ1.samples:
        sample_manager.AddSample(s, MC16e_TOPQ1.samples[s])
    for s in Data_rel21.samples:
        sample_manager.AddSample(s, Data_rel21.samples[s])

    # Go through all subdirectories, processing a maximum of 8 at
    # a time. Otherwise, wait for a process to finish first.
    proc_manager = ProcessManager.ProcessManager(args.processes)
    output_files = []  # List of all files written as output
    dir_counter = 0    # Just to count which directory we're looking at
    for subdir, files in input_dictionary.iteritems():
        proc_manager.waitTillReady()
        # if argument (--input ...) is too long because of the large number of files
        # we should split the job
        command = "ntuple-skimmer"
        input_flag_1 = ""
        input_flag_2 = ""
        input_flag_3 = ""
        input_flag_4 = ""
        input_flag_5 = ""
        input_flag_list = []
        max_number_in_one_go=200
        file_counter = 0
        for f in files:
            file_counter = file_counter + 1
            if file_counter < max_number_in_one_go:
              input_flag_1 += " \\\n --input %s" % (args.input_path + f)
            elif file_counter < 2*max_number_in_one_go:
              input_flag_2 += " \\\n --input %s" % (args.input_path + f)
            elif file_counter < 3*max_number_in_one_go:
              input_flag_3 += " \\\n --input %s" % (args.input_path + f)
            elif file_counter < 4*max_number_in_one_go:
              input_flag_4 += " \\\n --input %s" % (args.input_path + f)
            elif file_counter < 5*max_number_in_one_go:
              input_flag_5 += " \\\n --input %s" % (args.input_path + f)


        input_flag_list += [input_flag_1]
        input_flag_list += [input_flag_2]
        input_flag_list += [input_flag_3]
        input_flag_list += [input_flag_4]
        input_flag_list += [input_flag_5]
        counter = 0
        for input_flag in input_flag_list:
            if(input_flag == ""): continue
            counter += 1
            command = "ntuple-skimmer"
            command += input_flag
            filename = sample_manager.GetShortTitle(subdir, args.tag)
            filename = append_id(filename, counter )
            output_files.append(filename)
            command += " \\\n --luminosity %s" % (get_luminosity(sample_manager, subdir))
            command += " \\\n --output %s" % (args.output_path + filename)
            command += " \\\n --config-file %s" % (args.config_file)
            if veto_sample_overlap_region(filename): command += " \\\n --remove-overlap"
            if (args.process_truth): command += " \\\n --process-truth"
            if (args.skip_systematics): command += " \\\n --skip-systematics"
            if SampleManager.Dataset(subdir).DSID() == "410389": command += " \\\n --kfactor %f" % (args.signal_kfactor/1.16)
            if SampleManager.Dataset(subdir).DSID() == "410395": command += " \\\n --kfactor %f" % (args.signal_kfactor/1.16)
            if SampleManager.Dataset(subdir).DSID() == "410404": command += " \\\n --kfactor %f" % (args.signal_kfactor/1.16)
            if SampleManager.Dataset(subdir).DSID() == "410405": command += " \\\n --kfactor %f" % (args.signal_kfactor/1.16)
            command += " \\\n 2>&1 > %s.log" % filename
            dir_counter += 1
            if args.dry:
                print '\n' + command + '\n'
            else:
                print "Processing subdirectory %s of %s: %s" % (dir_counter, len(input_dictionary), subdir)
                print "   --> Writing into file: %s" % filename
                print "   --> Storing logs into: %s.log" % filename
                proc_manager.addProcess(command)


    # Wait for all processes to finish before continuing.
    proc_manager.finalise()

    # Create a dictionary which files should be merged.
    output_map = dict()
    for s in sample_manager.samples:
        output_map[s] = []
        for f in output_files:
            if f.find(s.Name()) == -1: continue
            output_map[s].append(f)
    return output_map




if __name__ == "__main__":
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("-d", "--dry", help="make a dry run and only print the commands", action="store_true")
    parser.add_argument("-c", "--config-file", help="location of the configuration file", required=True)
    parser.add_argument("-o", "--output-path", help="output path for the skimmed ntuples", required=True)
    parser.add_argument("-i", "--input-path", help="input path for the unskimmed ntuples", required=True)
    parser.add_argument("-f", "--filter", dest="wildcard", help="additional string to filter input files", type=str, default='')
    parser.add_argument("--process-truth", help="process the truth tree during skimming", action="store_true")
    parser.add_argument("--skip-systematics", help="skip systematic trees during skimming", action="store_true")
    parser.add_argument("--processes", help="the number of parallel processes to run", type=int, default=6)
    parser.add_argument("--tag", help="version tag to append to the output file name", type=str, default='')
    parser.add_argument("--signal-kfactor", help="the kfactor to be applied to the signal (410389)", type=float, default=1.44)
    args = parser.parse_args()


    # ======================================================
    # Step 1: perform a couple of safety checks with the input to
    # ensure "sanity" with what we're doing.
    args.input_path = HelperFunctions.ensure_trailing_slash(args.input_path)
    args.output_path = HelperFunctions.ensure_trailing_slash(args.output_path)
    if not os.path.isfile(args.config_file):
        print "Configuration file does not exist. Exiting"
        sys.exit(1)
    if not os.path.isdir(args.input_path):
        print "Input path does not exist. Exiting"
        sys.exit(1)
    if not os.path.isdir(args.output_path):
        print "Output path does not exist. Exiting"
        sys.exit(1)


    # ======================================================
    # Step 2: store all files within the input_path in a
    # dictionary. The dictionary contains entries for all
    # subdirectories and their files.
    input_dictionary = create_input_dictionary(args.input_path, args.wildcard)


    # ======================================================
    # Step 3: perform the actual skimming/slimming step, based on
    # the c++ executable ntuple-skimmer.exe. This uses the
    # information from the input dictionary to create appropriate
    # jobs, run them in parallel and write into output. The
    # function returns a dictionary of samples and their
    # associated output files.
    unmerged_output = perform_skimming_step(args, input_dictionary)

    sys.exit(0)
