# TTGammaNtupleSkimmer

The TTGammaNtupleSkimmer package takes of skimming _and_ slimming the ttgamma
ntuples produced with this repository:

- skimming: removal of events based on selection cuts.
- slimming: removal of unneeded branches from the output trees.

Both steps are meant to bring down the size of the ntuples to allow an easy
processing of them in further steps. The idea with this is to produce the main
set of ntuples with AnalysisTop and the 
[TTGammaEventSaver](../TTGammaEventSaver) package once, and store them
centrally. Then this skimming package can be used to, in addition, have much
smaller ntuples that focus on specific regions of interest.


### What is injected into the ntuples?

In addition to the skimming and slimming performed, the TTGammaNtupleSkimmer
also adds a few variables to the trees of the output file:

- `event_norm`: this variable includes the event normalization, which is a
  combination of the cross section associated with the dataset container, and
  the total weighted number of events. The cross section is retrieved using the
  `TopDataPreparation` package provided by AnalysisTop, the total weighted
  number of events is taken from the `sumWeights` trees in the ntuples. The
  `event_norm` variable is then generated as:

```c++
event_norm = xsec / total_events_weighted;  // xsec in pb
```

- `event_lumi`: this variables includes the luminosity associated with the
  dataset container. The luminosity to be injected needs to be given as an
  option to the `ntuple-skimmer` executable (see above).


### What does the skimming code do?

The core of this package is the executable `ntuple-skimmer`, the main code of
which is defined in [util/ntuple-skimmer.cxx](util/ntuple-skimmer.cxx). This
executable takes one or multiple input files that belong to _one_ dataset,
selects events based on given selection cuts, removes branches, and stores the
skimmed and slimmed output into a new ROOT file. The following options are
supported:

- `--help, -h`: print a help message
- `--config-file`: a config file to provide any of the below options in a more
  comfortable format.
- `--input, -i`: the input file(s) for the skimming procedure (ROOT format).
  This option can be stated multiple times for multiple input files.
- `--output, -o`: the output file after skimming (ROOT format).
- `--cuts, -c`: a string of event selection cuts. These generally follow any
  format supported by the TSelection class of ROOT.
- `--luminosity, -l`: the luminosity value to be injected into the ntuple
- `--slim-branch`: branches to be slimmed. This option supports wildcards.
- `--skim-exclude`: branches to be explicitly excluded during slimming. This is
  to, for example, reinclude a branch that matches wildcard with the 
  `slim-branch` option, but should still be included in the skimmed ntuple.
  
Again, it is important to stress that this only runs on _one single_ dataset,
i.e. the executable `ntuple-skimmer` needs to be executed for all logical
dataset identifiers for all DSIDs and all Monte-Carlo sub-campaigns. To make
life easier, the TTGammaNtupleSkimmer package also contains a couple of scripts
to allow batch processing, described in more detail in the next section.


### How to process ntuples in a batch

The `TTGammaNtupleSkimmer` package contains a total of three python-based
scripts to allow batch processing of ntuples. These are:

1. [scripts/tty-skim-batch.py](scripts/tty-skim-batch.py), a script to walk
   through a given input directory and create `ntuple-skimmer` subprocesses for
   all datasets found in that directory. The script identifies all logical
   dataset names, creates a dictionary entry for each of them, and saves
   information about all files associated to the dataset. With a given
   configuration file, the script then starts `ntuple-skimmer` subprocesses,
   where skimming and slimming is applied as described above.
2. [scripts/tty-skim-merge-files.py](scripts/tty-skim-merge-files.py), a script
   to merge all output files of the previous script that belong to the same type
   of process. For example, there might be multiple logical dataset names for Wy
   processes, all of which will have one individual output file from the
   previous step. This step then combines these output files into one.
3. [scripts/tty-skim-delete-merged-files.py](scripts/tty-skim-delete-merged-files.py),
   a script to delete all files that were merged together in the previous step.
   This of course exclude files where only only logical dataset name belongs to
   one process type, and only removes files that went into a merge file in step
   before.

For the first script, which generates the `ntuple-skimmer` subprocesses, the
following options have to be specified:

- `--input-path, -i`: the input path to locate the ntuples files from.
- `--output-path, -o`: the output path to store the skimmed/slimmed ntuples into.
- `--config-file, -c`: the config file used for the `ntuple-skimmer` executable.

The batch script will automatically decide for appropriate names for the output
files, which will include the type of process and the DSID of the dataset
container. This identification is based on the
[MC16a_TOPQ1.py](../TTGammaEventSaver/python/MC16a_TOPQ1.py),
[MC16d_TOPQ1.py](../TTGammaEventSaver/python/MC16d_TOPQ1.py) and
[MC16e_TOPQ1.py](../TTGammaEventSaver/python/MC16e_TOPQ1.py) defined in the
`TTGammaEventSaver` package. That is, dataset containers can only be processed
with the batch script if they are registered in one of the two above files (or
in [Data_rel21.py](../TTGammaEventSaver/python/Data_rel21.py)). The luminosity
values for the processed files will be determined based on which of the two
Monte-Carlo campaigns the dataset container belongs to.


### Usage example for batch processing

As an example, say, the input path has the following folder structure:
```
$INPUT_PATH/
    ├── user.kzoch.410081.MadGraphPythia8EvtGen.DAOD_TOPQ1.e4404_s3126_r10201_p3404.v1.0.output_root
        ├── user.kzoch.16293019._000001.output.root
        └── user.kzoch.16293019._000002.output.root
    ├── user.kzoch.410082.MadGraphPythia8EvtGen.DAOD_TOPQ1.e4404_s3126_r10201_p3404.v1.0.output_root
        ├── user.kzoch.14444323._000001.output.root
        └── user.kzoch.14444323._000002.output.root
    └── user.kzoch.410082.MadGraphPythia8EvtGen.DAOD_TOPQ1.e4404_s3126_r9364_p3390.v1.0.output_root
        ├── user.kzoch.16208966._000001.output.root
        └── user.kzoch.16208966._000002.output.root
```
Both samples are registered in the above dataset container lists and belong to
the sample categories "mc16d_TOPQ1_ttgamma" and "mc16a_TOPQ1_ttgamma",
respectively. If the batch script gets called with a configuration file called
`example.cfg`, it would generate the following `ntuple-skimmer` commands:
```sh
ntuple-skimmer \
 --input $INPUT_PATH/user.kzoch.410081.MadGraphPythia8EvtGen.DAOD_TOPQ1.e4404_s3126_r10201_p3404.v1.0.output_root/user.kzoch.16293019._000001.output.root \
 --input $INPUT_PATH/user.kzoch.410081.MadGraphPythia8EvtGen.DAOD_TOPQ1.e4404_s3126_r10201_p3404.v1.0.output_root/user.kzoch.16293019._000002.output.root \
 --luminosity 43584.4 \
 --output $OUTPUT_PATH/mc16d_TOPQ1_ttgamma.410081.root \
 --config-file example.cfg

ntuple-skimmer \
 --input $INPUT_PATH/user.kzoch.410082.MadGraphPythia8EvtGen.DAOD_TOPQ1.e4404_s3126_r10201_p3404.v1.0.output_root/user.kzoch.14444323._000001.output.root \
 --input $INPUT_PATH/user.kzoch.410082.MadGraphPythia8EvtGen.DAOD_TOPQ1.e4404_s3126_r10201_p3404.v1.0.output_root/user.kzoch.14444323._000002.output.root \
 --luminosity 43584.4 \
 --output $OUTPUT_PATH/mc16d_TOPQ1_ttgamma.410082.root \
 --config-file example.cfg

ntuple-skimmer \
 --input $INPUT_PATH/user.kzoch.410082.MadGraphPythia8EvtGen.DAOD_TOPQ1.e4404_s3126_r9364_p3390.v1.0.output_root/user.kzoch.16208966._000001.output.root \
 --input $INPUT_PATH/user.kzoch.410082.MadGraphPythia8EvtGen.DAOD_TOPQ1.e4404_s3126_r9364_p3390.v1.0.output_root/user.kzoch.16208966._000002.output.root \
 --luminosity 36212.32 \
 --output $OUTPUT_PATH/mc16a_TOPQ1_ttgamma.410082.root \
 --config-file example.cfg
```

The second script, the one for the merging procedure, would then call the
following `hadd` merge commands:

```sh
hadd -f \
 $OUTPUT_PATH/mc16d_TOPQ1_ttgamma.merged.root \
 $OUTPUT_PATH/mc16d_TOPQ1_ttgamma.410081.root
 $OUTPUT_PATH/mc16d_TOPQ1_ttgamma.410082.root
```

In this case, the merging step would merge the first two input datasets, as they
both belong to the process category _ttgamma_ and both belong to the Monte-Carlo
sub-campaign _mc16d_. The third logical dataset name is from a different
Monte-Carlo sub-campaign and therefore stays separate.

The third script would then remove the redundant two files:
```sh
rm -f $OUTPUT_PATH/mc16d_TOPQ1_ttgamma.410081.root
rm -f $OUTPUT_PATH/mc16d_TOPQ1_ttgamma.410082.root
```
