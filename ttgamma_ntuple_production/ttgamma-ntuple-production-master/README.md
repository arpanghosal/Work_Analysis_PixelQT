![AT version](https://img.shields.io/badge/AT%20version-21.2.167-green.svg)
![latest ntuples](https://img.shields.io/badge/latest%20ntuples-v11.0-blue.svg)

# ttgamma ntuple production

 * Ntuple contact: Amartya Rej, Andreas Kirchhoff (was: Knut Zoch)
 * Repository: [TopPropertiesTtGammaFullRun2/ttgamma-ntuple-production](https://gitlab.cern.ch/TopPropertiesTtGammaFullRun2/ttgamma-ntuple-production)
 * Storage location: `/eos/atlas/atlascerngroupdisk/phys-top/toproperties/ttgamma/`

The ntuple production for the tty analysis is coordinated centrally – all code
is version-controlled in git and stored in the above repository. The ntuple
production is based on
[AnalysisTop](https://twiki.cern.ch/twiki/bin/view/AtlasProtected/AnalysisTop21)
and uses the custom [TTGammaEventSaver](TTGammaEventSaver) package to calculate
and retrieve additional variables that are specific for the tty analysis. The
TTGammaEventSaver package also includes:
 * Cutfiles to use as input when running AnalysisTop. Different cutfiles are
   available, e.g. for dedicated channel productions, with/without systematics,
   with/without parton and particle level.
 * Lists of all Monte-Carlo samples and data containers used for ntuple
   production. For quick reference, click on the following links to see the
   latest lists of [mc16a
   samples](https://gitlab.cern.ch/TopPropertiesTtGammaFullRun2/ttgamma-ntuple-production/blob/master/TTGammaEventSaver/python/MC16a_TOPQ1.py)
   (corresponding to 2015/16 data), [mc16d
   samples](https://gitlab.cern.ch/TopPropertiesTtGammaFullRun2/ttgamma-ntuple-production/blob/master/TTGammaEventSaver/python/MC16d_TOPQ1.py)
   (corresponding to 2017 data), [mc16e
   samples](https://gitlab.cern.ch/TopPropertiesTtGammaFullRun2/ttgamma-ntuple-production/blob/master/TTGammaEventSaver/python/MC16e_TOPQ1.py)
   (corresponding to 2018 data) and the [list of data
   containers](https://gitlab.cern.ch/TopPropertiesTtGammaFullRun2/ttgamma-ntuple-production/blob/master/TTGammaEventSaver/python/Data_rel21.py).
 * A bunch of python scripts to communicate with the grid. This includes job
   submission and download scripts, scripts to cross-check the p-tag status of
   samples etc.

Conceptually, the cutfiles during ntuple production only apply very loose
preselections and don't cut tightly on any signal regions. This allows to use
them also for studies on control and validation regions. On the other hand, this
results in ntuples of several hundred GBs. As those are cumbersome to handle in
practice, the repository includes a second package,
[TTGammaNtupleSkimmer](TTGammaNtupleSkimmer), to reduce the size of the ntuples
through skimming (i.e. applying an additional event selection) and slimming
(i.e. removing variables/branches). The resulting _mini-ntuples_ are stored
under the same location on eos. The TTGammaNtupleSkimmer package includes:
 * The c++ program `ntuple-skimmer` to perform the actual slimming and skimming
   steps (source code:
   [util/ntuple-skimmer.cxx](https://gitlab.cern.ch/TopPropertiesTtGammaFullRun2/ttgamma-ntuple-production/blob/master/TTGammaNtupleSkimmer/util/ntuple-skimmer.cxx)).
   Branches can be slimmed by providing wildcards, and an additional event
   selection can be provided for skimming. The program also calculates the
   correct normalisation for each sample by looking at the TotalEventsWeighted
   values and by considering the sample cross-sections. The cross-sections are
   retrieved through the [TopDataPreparation
   package](https://gitlab.cern.ch/atlas/athena/tree/21.2/PhysicsAnalysis/TopPhys/TopPhysUtils/TopDataPreparation).
 * Several python scripts to run the =ntuple-skimmer= program on multiple raw
   ntuples at once. This includes merging of multiple samples of the same type
   (e.g. all Wy-type samples), injecting the correct luminosity values into the
   mini-ntuples, and parallel processing.

For a more detailed documentation of the [TTGammaEventSaver](TTGammaEventSaver)
and [TTGammaNtupleSkimmer](TTGammaNtupleSkimmer) packages, what exactly they do
and how to use them, click on them to get to the individual README pages. As the
code is version-controlled, different ntuple production rounds are tagged and
can be compared easily through the [GitLab compare
interface](https://gitlab.cern.ch/TopPropertiesTtGammaFullRun2/ttgamma-ntuple-production/compare).
A list of all version tags including a summary of changes can be found
[here](https://gitlab.cern.ch/TopPropertiesTtGammaFullRun2/ttgamma-ntuple-production/tags).



### Ntuple production workflow

The analysis uses TOPQ1 derivations as input. The ntuple production code of this
repository then performs the event loop, applies event selections and stores
data in flat ntuple format. The general workflow from TOPQ1 derivations to
mini-ntuples is meant to be as follows:
1. Local setup of the ntuple-production code, implementation of
   changes/additions to the code. The behaviour can then be tested easily by
   running AnalysisTop locally.
2. Submission of ntuple production jobs to the grid. The grid jobs receive
   tar-balled copies of the local code setup, so they should be working in
   exactly the same environment as the local setup.
3. Download of the raw ntuples from the grid.
4. Post-processing of the raw ntuples to mini-ntuples, which have a tighter
   event selection and all unnecessary branches/info removed.
5. Usage of these mini-ntuples for plotting, fitting, and studies.



### First-time setup of the code

The package configuration is fully based on cmake and the ATLAS-internal
`asetup` commands to set up releases. To allow smooth out-of-source builds, it
is best to start with an empty folder, which will host the ntuple-production
repository, a _build_ directory and any possible additional folders. The folder
structure should ideally be the following:

```
ttgamma-analysis/
    ├── build/
    ├── ttgamma-ntuple-production/
    └── [other directories]
```

The following commands take care of the entire setup:

```sh
mkdir -p ttgamma-analysis && cd ttgamma-analysis
setupATLAS --quiet
git clone https://:@gitlab.cern.ch:8443/TopPropertiesTtGammaFullRun2/ttgamma-ntuple-production.git
( cd ttgamma-ntuple-production && git submodule init && git submodule update )
mkdir build && cd build
. ../ttgamma-ntuple-production/setup.sh
. ./x86_64-slc6-gcc62-opt/setup.sh
cmake --build ./
```

In summary, these commands perform the following substeps:
1. Create a new directory called _ttgamma-analysis_.
2. Set up the ATLAS command environment with `setupATLAS`.
3. Make a clone of this repository.
4. Initialise the git submodules of this repository.
5. Create a _build_ folder which will contain the build files.
6. Source the setup script of this repository. This setup script sets up
   AnalysisTop and links the local copy of this repository against it.
7. Source the environment setup script generated by AnalysisTop.
8. Compile the code.



### Working with the code

After applying changes to the code, it can be recompiled via `cmake --build ./`
(while in the _build_ directory).



### How to recover an existing setup

To recover an existing _build_ directory and folder structure (e.g. after
logging out and back in), the above chain of commands does not need to be
executed again. The setup script will notice that the old setup can be resumed,
i.e. it is sufficient to switch back into the already existing _build_ folder
and source the setup scripts once again:

```sh
cd ttgamma-analysis/build
. ../ttgamma-ntuple-production/setup.sh
. ./x86_64-slc6-gcc62-opt/setup.sh
```


### Known issues in ntuple production using grid

* Sometimes jobs may fail while `Scouting` possibly due to grid-site's issue with handling multiple files as input. In that case the option `OneFilePerJob` can be tried in the grid submission script `tty-grid-submit.py`
