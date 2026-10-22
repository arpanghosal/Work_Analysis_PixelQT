#!/usr/bin/env python

import sys

# A couple of helper functions used for the TTGammaNtupleSkimmer
# package. These functions currently only get called in the
# ntuple-skimmer-batch.py script.

# Ensure that path has a trailing slash.
def ensure_trailing_slash(path):
    if path[-1:] != '/': path = path + '/'
    return path

# Remove potential leading slashes from the path.
def remove_leading_slash(path):
    if path[:1] == '/': path = path[1:]
    return path

# Ask a yes/no question to the user. This is copied from:
# https://stackoverflow.com/questions/3041986/
def query_yes_no(question, default="yes"):
    valid = {"yes": True, "y": True, "ye": True,
                "no": False, "n": False}
    if default is None:
        prompt = " [y/n] "
    elif default == "yes":
        prompt = " [Y/n] "
    elif default == "no":
        prompt = " [y/N] "
    else:
        raise ValueError("invalid default answer: '%s'" % default)

    while True:
        sys.stdout.write(question + prompt)
        choice = raw_input().lower()
        if default is not None and choice == '':
            return valid[default]
        elif choice in valid:
            return valid[choice]
        else:
            sys.stdout.write("Please respond with 'yes' or 'no' "
                                "(or 'y' or 'n').\n")
