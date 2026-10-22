# TTGammaEventSaver

The TTGammaEventSaver package adds a custom event saver on top of the standard
flat-ntuple event saver within AnalysisTop
([EventSaverFlatNtuple](https://gitlab.cern.ch/atlas/athena/blob/21.2/PhysicsAnalysis/TopPhys/xAOD/TopAnalysis/TopAnalysis/EventSaverFlatNtuple.h)).
The custom event saver mostly adds photon-related variables. Some of them e.g.
photon isolation variables only need to be retrieved using the approriate tools
and then be stored in dedicated branches of the output trees; others are
combined variables and are calculated during execution:

- `EventSaver::storeXXX()` functions retrieve information and store them.
- `EventSaver::calculateXXX()` functions calculate variables during execution.

The event saver is executed like an ASG tool, i.e. it gets initialised at the
beginning of execution. Then the `EventSaver::SaveEvent()` gets executed for
each event, which subsequently calls all the above functions to calculate and
store variables.

One important aspect during event saving is to categorise the reconstructed
photons into prmopt/non-prompt categories based on their origin. This
categorisation can then later be used to decide whether the event contained a
prompt or a fake photon.

The following categories are currently defined:

```c++
  enum PhotonCategory {
    NotDefined = 0,   ///< Event was not classified
    ISR = 1,          ///< Prompt photon from production radiation
    FromTop = 2,      ///< Prompt photon radiated from top
    FromW = 3,        ///< Prompt photon radiated from W
    FromB = 4,        ///< Prompt photon radiated from B
    FromLepton = 5,   ///< Prompt photon radiated from lepton
    Other = 6,        ///< Prompt photon from other sources
    Hfake = 10,       ///< Hadronic fake photon
    Efake = 20,       ///< Electronic fake photon
  };
```

Apart from photon-related information, the event saver also decides whether an
events falls into any sample-overlap region. As the tty analysis uses inclusive,
but also dedicated samples with extra photons on matrix-element level, some of
the samples might overlap (in which case contributions should be taken from the
dedicated samples). The sample overlap is treated with the
[VGammaORTool](https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/VGammaORTool)
which is part of athena. More details can be found in the [dedicated README
section](sample_overlap_removal.md).


### Content of the package

Apart from the main event saver, this package also contains several helper
scripts and data files that can/need to be used in conjunction with the code.
More specifically, this package contains:
- Cutfiles to be used when running AnalysisTop with the custom tty event saver.
  There are multiple cutfiles for different purposes, e.g. with/without
  systematics, dedicated channel cutfiles, with/without particle and parton
  level information. The most important ones are:
  - [default cutfile](config/cutfile.txt): all five channels with a very loose
    preselection. This cutfile does _not_ include systematics.
  - [emu-only cutfile](config/cutfile_syst_emu_only.txt): only emu channel with
    a tight selection on the emu signal region. This cutfile includes full
    systematics.
  - [ljets-only cutfile](config/cutfile_ljets_only.txt): only l+jets channels
    with a loose preselection. This cutfile does _not_ include systematics. The
    reason this cutfile is not tightened to the signal regions is that it also
    needs to be used for hadron-fake studies.
  - [truth-level cutfile](config/cutfile_truth.txt): to be used only for the
    signal sample(s). This cutfile activates parton level and includes
    particle-level event selections.
- Text files with locations of example inputs for AnalysisTop. These are only
  for local testing purposes of the AnalysisTop output. Some example files of
  TOPQ1 derivations were downloaded and their locations saved in text files.
- Lists of the currently used Monte-Carlo samples (see below for more details).
- Several scripts to communicate with the grid:
  - [tty-grid-submit.py](scripts/tty-grid-submit.py): python script to
    submit jobs to the grid. This script reads the above lists of samples and
    provides functionality to zip tarballs of the build environment and send it
    to the grid.
  - [tty-grid-download.py](scripts/tty-grid-download.py): python script to
    download finished jobs from the grid.
  - [tty-ami-lookup-ptags.py](scripts/tty-ami-lookup-ptags.py): python script
    which takes the above sample lists and compares them to information from the
    AMI database. It outputs whether the samples can be found and whether they
    are using the latest p-tag or not.
  - [tty-ami-lookup-xsecs.py](scripts/tty-ami-lookup-xsecs.py): python script
    which takes the above sample lists and looks up their cross-sections from
    the AMI database.


### Detailed information about the event saver

#### What are "good" photons?

Currently, photons are identified as "good" if they pass the following criteria:
- p_T > 20 GeV
- _Tight_ identification criteria

Good photons are not stored separately, but their index is saved in the variable
`std::vector<int> m_ph_good_index`. The size of that vector is also an indicator
how many good photons the event contains. Reminder: in the ROOT tree viewer and
in cut strings, the size of a vector can be retrieved via
`@ph_good_index->size()`.


#### Which samples are being processed?

The TTGammaEventSaver package includes lists of Monte-Carlo samples for the
ntuple production. The samples are categorised according to processes they
describe:

- ttgamma: for the signal processes (ttbar + prompt photon)
- ttbar: for ttbar with fake photons
- singletop: for singletop processes with prompt _or_ fake photons
- ttV: for ttbar+Z/W processes with prompt _or_ fake photons
- diboson: for diboson processes with prompt _or_ fake photons
- Wjets, Zjets: for Wjets and Zjets with fake photons
- Wgamma, Zgamma: for Wjets and Zjets with prompt photons (dedicated samples)

All categories are defined for the mc16a, mc16d and mc16e production campaigns
separately (a: pile-up profile of 2015+16 data, d: that of 2017 data, e: that of
2018 data). The complete list of datasets in those categories are found under
[MC16a_TOPQ1.py](python/MC16a_TOPQ1.py), 
[MC16d_TOPQ1.py](python/MC16d_TOPQ1.py) and
[MC16e_TOPQ1.py](python/MC16e_TOPQ1.py). The list of dataset containers used for
the data processing are listed under [Data_rel21.py](python/Data_rel21.py).



#### Which variables are added to the ntuples?

First of all: everything that is being saved by AnalysisTop by default. For more
information, refer to the source code of the
[EventSaverFlatNtuple class](https://gitlab.cern.ch/atlas/athena/blob/21.2/PhysicsAnalysis/TopPhys/xAOD/TopAnalysis/TopAnalysis/EventSaverFlatNtuple.h).
In addition to that, the ttgamma event saver adds several variables to the output:

- PDG IDs of the leptons in the W decays
- General event variables (transverse W mass, total scalar transverse energy, number of jets ...)
- Index of photons passing "good" criteria (see below)
- Photon MC-based variables
- Photon reconstruction variables
- Photon identification variables
- Photon isolation variables
- Photon shower shape variables
- Lepton-bjet variables (dR and m_lb between lepton(s) and closest bjet)
- B-tagging variables

Here is a list of all variables defined in the event saver (the branch name is
identical to the variable name modulo the `m_` at the beginning). This list is
directly copied from the corresponding
[EventSaver.h](TTGammaEventSaver/TTGammaEventSaver/EventSaver.h) header file.

```c++
  /// General event information variables.
  float m_event_dphill;                 ///< Modulus of dPhi of the two leptons between [0, pi] (if available).
  float m_event_mll;                    ///< Invariant mass of the two leptons (if available).
  float m_event_mwt;                    ///< Transverse W mass.
  float m_event_HT;                     ///< Sum over hadronic transverse momenta.
  float m_event_ST;                     ///< Sum over all transverse momenta (including MET).
  int m_event_njets;                    ///< Number of jets.
  int m_event_nbjets70;                 ///< Number of b-tagged jets (70% WP).
  int m_event_nbjets77;                 ///< Number of b-tagged jets (77% WP).
  int m_event_nbjets85;                 ///< Number of b-tagged jets (85% WP).
  char m_event_in_overlap;              ///< Boolean whether event is in overlap region (using the VGammaORTool).
  char m_event_in_overlap_TTH;          ///< Boolean whether event is in overlap region (TTH method).
  char m_event_in_overlap_REL20;        ///< Boolean whether event is in overlap region (rel. 20.7 method).

  /// Photon quality variables.
  std::vector<int> m_ph_good_index;     ///< Index of photons passing "good" criteria.

  /// Photon MC-based variables.
  std::vector<int> m_ph_true_category;  ///< Category of the photon, according to TTGamma::PhotonCategory.
  std::vector<char> m_ph_true_topparent;  ///< Ancestry of the xAOD::TruthParticle leads to a top.
  std::vector<int> m_ph_true_parent;    ///< PDG ID of the parent of the xAOD::TruthParticle.
  std::vector<int> m_ph_true_barcode;   ///< barcode of the corresponding xAOD::TruthParticle.
  std::vector<int> m_ph_true_origin;    ///< Defined in MCTruthClassifier/MCTruthClassifierDefs.h.
  std::vector<int> m_ph_true_pdgid;     ///< PDG ID of the corresponding xAOD::TruthParticle.
  std::vector<int> m_ph_true_type;      ///< Defined in MCTruthClassifier/MCTruthClassifierDefs.h.
  std::vector<float> m_ph_true_eta;     ///< eta of the corresponding xAOD::TruthParticle.
  std::vector<float> m_ph_true_phi;     ///< phi of the corresponding xAOD::TruthParticle.
  std::vector<float> m_ph_true_pt;      ///< p_T of the corresponding xAOD::TruthParticle.

  /// Photon reconstruction variables.
  std::vector<int> m_ph_conversion_type;  ///< Defined in xAODEgamma/EgammaEnums.h.
  std::vector<float> m_ph_drlph;        ///< dR between photon and closest lepton.
  std::vector<float> m_ph_mlph;         ///< Invariant mass of photon and closest lepton.
  std::vector<float> m_ph_mllph;        ///< Invariant mass of leptons and photon (dilepton only).

  /// Photon identification variables.
  std::vector<char> m_ph_id_loose;
  std::vector<char> m_ph_id_tight;
  std::vector<char> m_ph_id_tight_relaxed;  ///< Relaxed tight ID for photons without the below calo-strips cuts.
  std::vector<char> m_ph_id_fail_deltaE;    ///< Boolean whether deltaE cut of tight ID was failed.
  std::vector<char> m_ph_id_fail_fside;     ///< Boolean whether f_side cut of tight ID was failed.
  std::vector<char> m_ph_id_fail_ws3;       ///< Boolean whether ws3 cut of tight ID was failed.
  std::vector<char> m_ph_id_fail_Eratio;    ///< Boolean whether Eratio cut of tight ID was failed.
  std::vector<int> m_ph_id_fail_count;      ///< Counter how many of the above ID criteria were failed.

  /// Photon isolation variables.
  std::vector<float> m_ph_iso_ptcone20;
  std::vector<float> m_ph_iso_topoetcone20;
  std::vector<float> m_ph_iso_topoetcone40;
  std::vector<char>  m_ph_iso_FCL;
  std::vector<char>  m_ph_iso_FCT;
  std::vector<char>  m_ph_iso_FCTCO;

  /// Photon shower-shape variables.
  std::vector<float> m_ph_ss_deltaE;
  std::vector<float> m_ph_ss_e277;
  std::vector<float> m_ph_ss_emaxs1;
  std::vector<float> m_ph_ss_eratio;    ///< a.k.a. DEmaxs1
  std::vector<float> m_ph_ss_f1;
  std::vector<float> m_ph_ss_fracs1;    ///< a.k.a. fside
  std::vector<float> m_ph_ss_reta;
  std::vector<float> m_ph_ss_rhad1;
  std::vector<float> m_ph_ss_rhad;
  std::vector<float> m_ph_ss_rphi;
  std::vector<float> m_ph_ss_weta1;     ///< a.k.a. ws3
  std::vector<float> m_ph_ss_weta2;
  std::vector<float> m_ph_ss_wtots1;    ///< a.k.a. wstot

  /// Photon-bjet variables.
  std::vector<float> m_ph_drphb70;      ///< dR between photon and closest bjet (70% WP).
  std::vector<float> m_ph_drphb77;      ///< dR between photon and closest bjet (77% WP).
  std::vector<float> m_ph_drphb85;      ///< dR between photon and closest bjet (85% WP).
  std::vector<float> m_ph_mphb70;       ///< Invariant mass between photon and closest bjet (70% WP).
  std::vector<float> m_ph_mphb77;       ///< Invariant mass between photon and closest bjet (77% WP).
  std::vector<float> m_ph_mphb85;       ///< Invariant mass between photon and closest bjet (85% WP).

  /// Lepton-bjet variables.
  std::vector<float> m_el_drlb70;       ///< dR between electron and closest bjet (70% WP).
  std::vector<float> m_el_drlb77;       ///< dR between electron and closest bjet (77% WP).
  std::vector<float> m_el_drlb85;       ///< dR between electron and closest bjet (85% WP).
  std::vector<float> m_el_mlb70;        ///< Invariant mass between electron and closest bjet (70% WP).
  std::vector<float> m_el_mlb77;        ///< Invariant mass between electron and closest bjet (77% WP).
  std::vector<float> m_el_mlb85;        ///< Invariant mass between electron and closest bjet (85% WP).
  std::vector<float> m_mu_drlb70;       ///< dR between muon and closest bjet (70% WP).
  std::vector<float> m_mu_drlb77;       ///< dR between muon and closest bjet (77% WP).
  std::vector<float> m_mu_drlb85;       ///< dR between muon and closest bjet (85% WP).
  std::vector<float> m_mu_mlb70;        ///< Invariant mass between muon and closest bjet (70% WP).
  std::vector<float> m_mu_mlb77;        ///< Invariant mass between muon and closest bjet (77% WP).
  std::vector<float> m_mu_mlb85;        ///< Invariant mass between muon and closest bjet (85% WP).

  /// B-tagging variables.
  std::vector<int> m_jet_mv2c10_ranked_index;    ///< Int vector with mv2c10-ranked jet indices.
```
