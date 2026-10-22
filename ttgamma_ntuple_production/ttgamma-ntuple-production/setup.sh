#!/bin/sh

echo "************************************************************************"
echo -e "SETUP\t This is the ttgamma-ntuple-production setup script"
echo -e "SETUP\t "
echo -e "SETUP\t For more info, consult the README pages:"
echo -e "SETUP\t https://gitlab.cern.ch/TopPropertiesTtGammaFullRun2/ttgamma-ntuple-production"
echo ""


# Determine where this script is located (this is surprisingly difficult).
if [ "${BASH_SOURCE[0]}" != "" ]; then
    # This should work in bash.
    _src=${BASH_SOURCE[0]}
elif [ "${ZSH_NAME}" != "" ]; then
    # And this in zsh.
    _src=${(%):-%x}
elif [ "${1}" != "" ]; then
    # If none of the above works, we take it from the command line.
    _src="${1/setup.sh/}/setup.sh"
else
    echo -e "SETUP\t Failed to determine the TTGamma base directory. Aborting ..."
    echo -e "SETUP\t Can you give the source script location as additional argument? E.g. with"
    echo -e "SETUP\t . ../foo/bar/scripts/setup.sh ../foo/bar/scripts"
    return 1
fi


# Set up TTGamma environment variables that are picked up from the code.
export TTGAMMA_AT_VERSION="21.2.167"
export TTGAMMA_BASE_DIR="$(cd -P "$(dirname "${_src}")" && pwd)"
export TTGAMMA_NTUPLE_VERSION="v10"
export TTGAMMA_NTUPLE_LOCATION="/eos/atlas/atlascerngroupdisk/phys-top/toproperties/ttgamma/full_run2_v9.0"


# Make sure that the setupATLAS command has been called.
echo "************************************************************************"
echo -e "SETUP\t Executing \"setupATLAS --quiet\""
setupATLAS --quiet || { echo -e "\"setupATLAS\" failed. Aborting ..."; return 1; }


# Set up the provided AnalysisBase version. Ignore any previous
# .asetup.save files, i.e. don't use something like "asetup
# --restore", because we want to make sure we're setting up the
# correct version.
echo "************************************************************************"
echo -e "SETUP\t Executing \"asetup AnalysisBase,${TTGAMMA_AT_VERSION}\""
asetup AnalysisBase,${TTGAMMA_AT_VERSION} || { echo -e "Failed to set up AnalysisBase ${TTGAMMA_AT_VERSION}. Aborting ..."; }
#asetup master,latest,Athena,here

# Now set up the necessary versions of: cmake, git, panda, pyami,
# root and rucio.
echo "************************************************************************"
echo -e "SETUP\t Executing \"lsetup cmake git panda pyami rucio\""
lsetup cmake git panda pyami  || { echo  -e "SETUP\t \"lsetup\" command failed. Aborting ..."; return 1; }
#lsetup "rucio 1.21.11.patch1" || { echo  -e "SETUP\t \"lsetup\" command failed. Aborting ..."; return 1; }

# Then start configuring cmake (assuming all relevant code lies
# in the ttgamma base directory).
echo -e "SETUP\t Executing \"cmake \${TTGAMMA_BASE_DIR}\""
cmake ${TTGAMMA_BASE_DIR} || { echo  -e "SETUP\t \"cmake\" command failed. Aborting ..."; return 1; }


echo "************************************************************************"
echo -e "SETUP\t Configuration finished!"
echo -e "SETUP\t "
echo -e "SETUP\t Don't forget to source the environment setup, e.g. with:"
echo -e "SETUP\t . ./x86_64-slc6-gcc62-opt/setup.sh"
