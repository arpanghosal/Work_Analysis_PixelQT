#include "TTGammaEventSaver/EventSaver.h"
#include "TTGammaEventSaver/PartonLevelInfo.h"
#include "TTGammaEventSaver/SampleOverlapRemover.h"
#include "TTGammaEventSaver/TruthElectronExtractor.h"           ///< To extract truth electrons from event

#include "ElectronPhotonSelectorTools/egammaPIDdefs.h"          ///< Photon ID definitions
#include "fastjet/ClusterSequence.hh"
#include "TopConfiguration/TopConfig.h"
#include "TopEvent/Event.h"
#include "TopEvent/EventTools.h"
#include "TopEventSelectionTools/TreeManager.h"
#include "xAODTracking/VertexContainer.h"

#include <numeric>
#include <regex>

namespace {
/**
 * Return whether sample is in a list of samples associated with
 * signal processes and, hence, needs information about the decay
 * channel. This is later used for sample overlap removal and
 * event categoriation. This list of samples includes tty and
 * Wty, but also ttbar as well as Wt (inclusive and dilepton).
 */
bool needs_channel_info(const unsigned int dsid) {
  std::set<unsigned int> sample_dsids = {{
      410389,   ///< tty nominal
      410404,   ///< tty radiation up
      410405,   ///< tty radiation down
      410395,   ///< tty Herwig7
      //504554,   ///< tty production mode @NLO
      //412112,	///< tty production mode @NLO old
      //504553,   ///< tty production mode @NLO Herwig7
      //502957,   ///< tty production mode @NLO var3cDown
      //502958,   ///< tty production mode @NLO var3cUp
      412113,	///< tty production mode @LO
      //500800,   ///< tty decay mode @LO
      //412114,	///< tty decay mode @LO old
      //504689,   ///< tty decay mode @LO Herwig7
      410470,   ///< ttbar (inclusive) nominal
      410480,   ///< ttbar (inclusive) hdamp
      410464,   ///< ttbar (inclusive) aMC
      600666,   ///< ttbar (inclusive) Herwig7
      //410557,   ///< ttbar (inclusive) Herwig7
      410472,   ///< ttbar (dilepton) nominal
      410482,   ///< ttbar (dilepton) hdamp
      410465,   ///< ttbar (dilepton) aMC
      //410558,   ///< ttbar (dilepton) Herwig7
      600667,   ///< ttbar (dilepton) Herwig7
      410646,   ///< Wt (inclusive) nominal top
      410647,   ///< Wt (inclusive) nominal antitop
      410654,   ///< Wt (inclusive) DS top
      410655,   ///< Wt (inclusive) DS antitop
      412002,   ///< Wt (inclusive) aMC
      411036,   ///< Wt (inclusive) Herwig7 top
      411037,   ///< Wt (inclusive) Herwig7 antitop
      410648,   ///< Wt (dilepton) nominal top
      410649,   ///< Wt (dilepton) nominal antitop
      410656,   ///< Wt (dilepton) DS top
      410657,   ///< Wt (dilepton) DS antitop
      412003,   ///< Wt (dilepton) aMC
      411038,   ///< Wt (dilepton) Herwig7 top
      411039,   ///< Wt (dilepton) Herwig7 antitop
      412006,   ///< Wty (y decay) nominal
      412007,   ///< Wty (y decay) Herwig7
      412120,   ///< Wty nominal
      412122,   ///< Wty Herwig7
    }};

  if (sample_dsids.find(dsid) != sample_dsids.end()) {
    return true;
  } else {
    return false;
  }
}

/**
 * Return whether sample is in a list of signal samples and,
 * hence, needs parton-level information for unfolding. This list
 * includes tty and Wty samples.
 */
 bool needs_parton_level(const unsigned int dsid) {
   std::set<unsigned int> sample_dsids = {{
       410389,   ///< tty nominal
       410404,   ///< tty radiation up
       410405,   ///< tty radiation down
       410395,   ///< tty Herwig7
       //504554,   ///< tty production mode @NLO
       //412112,	 ///< tty production mode @NLO old
       //504553,   ///< tty production mode @NLO Herwig7
       //502957,   ///< tty production mode @NLO var3cDown
       //502958,   ///< tty production mode @NLO var3cUp
       412113,	 ///< tty production mode @LO
       //500800,   ///< tty decay mode @LO
       //412114,	 ///< tty decay mode @LO old
       //504689,   ///< tty decay mode @LO Herwig7
       412006,   ///< Wty (y decay) nominal
       412007,   ///< Wty (y decay) Herwig7
       412120,   ///< Wty nominal
       412122,   ///< Wty Herwig7
     }};

   if (sample_dsids.find(dsid) != sample_dsids.end()) {
     return true;
   } else {
     return false;
   }
 }

/**
 * For a given event saver, tree manager and variable name,
 * decide whether the variable should be vetoed in the tree
 * output. This uses regular expressions to test the variable
 * name for specific patterns, and if it is matched, the variable
 * is vetoed. The function parameters take this specific form to
 * allow direct binding to the top::TreeManager::BranchFilter
 * class that is used for filtering branches.
 *
 * @param variable The variable to be tested.
 * @return A return value where "1" accepts the variable, "0"
 * vetoes the variable, and "-1" means "don't decide anything,
 * pass on to the next branch filter".
 */
int veto_branches(const top::ttgamma::EventSaver*, const top::TreeManager*, const std::string& variable) {
  // Lambda to test a wildcard on the variable.
  auto matches_wildcard = [&variable] (const std::string& wildcard) {
    std::smatch match;
    std::regex_search(variable, match, std::regex(wildcard));
    if (!match.empty()) return true;
    return false;
  };

  //if (matches_wildcard("^weight_bTagSF_MV2c10_[0-9]+_")) return 0;  //< Variations of fixed b-tag WPs (not used)
  if (matches_wildcard("^weight_globalLeptonTriggerSF")) return 0;  //< Global trigger SF variations (redundant)
  if (matches_wildcard("^weight_indiv_SF")) return 0;               //< Individual lepton SF variations (redundant)
  if (matches_wildcard("^weight_oldTriggerSF")) return 0;           //< Old trigger SF variations (superseded by global triggers)
  //if (matches_wildcard("^jet_DL1")) return 0;                       //< DL1-related variables (not used)
  if (matches_wildcard("^el_true_firstEgMother")) return 0;         //< Electron mother info (not used)
  if (matches_wildcard("^el_trigMatch_HLT")) return 0;              //< HLT info (not used)
  if (matches_wildcard("^mu_trigMatch_HLT")) return 0;              //< HLT info (not used)
  if (matches_wildcard("^HLT_")) return 0;                          //< HLT info (not used)

  // Deactivate branches for scale-factor systematics if needed.
  // if (matches_wildcard("_DOWN$")) return 0;
  // if (matches_wildcard("_UP$")) return 0;
  // if (matches_wildcard("_down$")) return 0;
  // if (matches_wildcard("_up$")) return 0;
  return -1;
}
}  // anonymous namespace



namespace top {
namespace ttgamma {
// -----------------------------------------------
EventSaver::EventSaver()
    : m_vgamma_tool{"VGammaORTool/VGammaORTool"}
      //m_photonpointing_tool{"PhotonPointingTool/PhotonPointingTool"}
    {
  // A not-very-nice implementation of a branch filter in the
  // EventSaverFlatNtuple class that we use here. To make things
  // easier, the actual branch vetoing happens in a separate
  // function veto_branches(). It's functional arguments are
  // identical to the top::TreeManager::BranchFilter class, so
  // that we can directly use std::bind to push it into the
  // m_branchFilters object of the EventSaverFlatNtuple.
  //
  // More info in the ANALYSISTO-61 JIRA ticket, under:
  // https://its.cern.ch/jira/browse/ANALYSISTO-61
  branchFilters().push_back(std::bind(&veto_branches, this, std::placeholders::_1, std::placeholders::_2));
}

// -----------------------------------------------
EventSaver::~EventSaver() = default;

// -----------------------------------------------
void EventSaver::initialize(std::shared_ptr<top::TopConfig> config, TFile* file, const std::vector<std::string>& extraBranches) {
  // Let the base class do all the hard work. It will setup
  // TTrees for each systematic with a standard set of variables.
  top::EventSaverFlatNtuple::initialize(config, file, extraBranches);

  // Configure and initialise the VGammaORTool, which handles
  // flags for sample overlap removal.
  top::check(m_vgamma_tool.setProperty("dR_lepton_photon_cut", 0.2), "Could not set VGammaORTool lepton-photon dR cut");
  top::check(m_vgamma_tool.setProperty("photon_pT_cuts", std::vector<float>{15e3}), "Could not set VGammaORTool photon pT cut");
  top::check(m_vgamma_tool.initialize(), "Could not initialize VGammaORTool");

  // Configure and initialise the PhotonPointingTool. This is
  // used to get information about the z-origin of photons (for
  // detection of photons from pile-up).
  ASG_SET_ANA_TOOL_TYPE(m_photonpointing_tool, CP::PhotonPointingTool);
  m_photonpointing_tool.setName("PhotonPointingTool");
  top::check(m_photonpointing_tool.setProperty("isSimulation", config->isMC()), "fail to set isSimulation");
  top::check(m_photonpointing_tool.initialize(), "Could not initialize PhotonPointingTool");

  // Set whether we're looking at a Wty sample. This is used to
  // identify those samples where we do not expect to find the
  // spectator b-quark (Wty 5-flavour scheme). If we end up using
  // a 4-flavour scheme sample, this flag should be set to
  // 'false' (and maybe renamed to avoid confusion).
  if (config->getDSID() == 412120 || config->getDSID() == 412122 || config->getDSID() == 412006 || config->getDSID() == 412007) m_is_tWy = true;

  // Store parton-level information in the truth tree.
  if (config->isMC()) {
    if(needs_parton_level(config->getDSID())) {
      truthTreeManager()->makeOutputVariable(m_MC_tty_ph_pt_before,                "MC_tty_ph_pt_before");
      truthTreeManager()->makeOutputVariable(m_MC_tty_ph_pt_after1st,              "MC_tty_ph_pt_after1st");
      truthTreeManager()->makeOutputVariable(m_MC_tty_ph_pt_afterall,              "MC_tty_ph_pt_afterall");
      truthTreeManager()->makeOutputVariable(m_MC_tty_ph_eta_before,               "MC_tty_ph_eta_before");
      truthTreeManager()->makeOutputVariable(m_MC_tty_ph_eta_after1st,             "MC_tty_ph_eta_after1st");
      truthTreeManager()->makeOutputVariable(m_MC_tty_ph_eta_afterall,             "MC_tty_ph_eta_afterall");
      truthTreeManager()->makeOutputVariable(m_MC_tty_ph_phi_before,               "MC_tty_ph_phi_before");
      truthTreeManager()->makeOutputVariable(m_MC_tty_ph_phi_after1st,             "MC_tty_ph_phi_after1st");
      truthTreeManager()->makeOutputVariable(m_MC_tty_ph_phi_afterall,             "MC_tty_ph_phi_afterall");
      truthTreeManager()->makeOutputVariable(m_MC_tty_ph_isIso_afterall,           "MC_tty_ph_isIso_afterall");
      truthTreeManager()->makeOutputVariable(m_MC_tty_Wplus_decay1_pt_before,      "MC_tty_Wplus_decay1_pt_before");
      truthTreeManager()->makeOutputVariable(m_MC_tty_Wplus_decay1_pt_after1st,    "MC_tty_Wplus_decay1_pt_after1st");
      truthTreeManager()->makeOutputVariable(m_MC_tty_Wplus_decay1_pt_afterall,    "MC_tty_Wplus_decay1_pt_afterall");
      truthTreeManager()->makeOutputVariable(m_MC_tty_Wplus_decay1_eta_before,     "MC_tty_Wplus_decay1_eta_before");
      truthTreeManager()->makeOutputVariable(m_MC_tty_Wplus_decay1_eta_after1st,   "MC_tty_Wplus_decay1_eta_after1st");
      truthTreeManager()->makeOutputVariable(m_MC_tty_Wplus_decay1_eta_afterall,   "MC_tty_Wplus_decay1_eta_afterall");
      truthTreeManager()->makeOutputVariable(m_MC_tty_Wplus_decay1_phi_before,     "MC_tty_Wplus_decay1_phi_before");
      truthTreeManager()->makeOutputVariable(m_MC_tty_Wplus_decay1_phi_after1st,   "MC_tty_Wplus_decay1_phi_after1st");
      truthTreeManager()->makeOutputVariable(m_MC_tty_Wplus_decay1_phi_afterall,   "MC_tty_Wplus_decay1_phi_afterall");
      truthTreeManager()->makeOutputVariable(m_MC_tty_Wplus_decay2_pt_before,      "MC_tty_Wplus_decay2_pt_before");
      truthTreeManager()->makeOutputVariable(m_MC_tty_Wplus_decay2_pt_after1st,    "MC_tty_Wplus_decay2_pt_after1st");
      truthTreeManager()->makeOutputVariable(m_MC_tty_Wplus_decay2_pt_afterall,    "MC_tty_Wplus_decay2_pt_afterall");
      truthTreeManager()->makeOutputVariable(m_MC_tty_Wplus_decay2_eta_before,     "MC_tty_Wplus_decay2_eta_before");
      truthTreeManager()->makeOutputVariable(m_MC_tty_Wplus_decay2_eta_after1st,   "MC_tty_Wplus_decay2_eta_after1st");
      truthTreeManager()->makeOutputVariable(m_MC_tty_Wplus_decay2_eta_afterall,   "MC_tty_Wplus_decay2_eta_afterall");
      truthTreeManager()->makeOutputVariable(m_MC_tty_Wplus_decay2_phi_before,     "MC_tty_Wplus_decay2_phi_before");
      truthTreeManager()->makeOutputVariable(m_MC_tty_Wplus_decay2_phi_after1st,   "MC_tty_Wplus_decay2_phi_after1st");
      truthTreeManager()->makeOutputVariable(m_MC_tty_Wplus_decay2_phi_afterall,   "MC_tty_Wplus_decay2_phi_afterall");
      truthTreeManager()->makeOutputVariable(m_MC_tty_Wminus_decay1_pt_before,     "MC_tty_Wminus_decay1_pt_before");
      truthTreeManager()->makeOutputVariable(m_MC_tty_Wminus_decay1_pt_after1st,   "MC_tty_Wminus_decay1_pt_after1st");
      truthTreeManager()->makeOutputVariable(m_MC_tty_Wminus_decay1_pt_afterall,   "MC_tty_Wminus_decay1_pt_afterall");
      truthTreeManager()->makeOutputVariable(m_MC_tty_Wminus_decay1_eta_before,    "MC_tty_Wminus_decay1_eta_before");
      truthTreeManager()->makeOutputVariable(m_MC_tty_Wminus_decay1_eta_after1st,  "MC_tty_Wminus_decay1_eta_after1st");
      truthTreeManager()->makeOutputVariable(m_MC_tty_Wminus_decay1_eta_afterall,  "MC_tty_Wminus_decay1_eta_afterall");
      truthTreeManager()->makeOutputVariable(m_MC_tty_Wminus_decay1_phi_before,    "MC_tty_Wminus_decay1_phi_before");
      truthTreeManager()->makeOutputVariable(m_MC_tty_Wminus_decay1_phi_after1st,  "MC_tty_Wminus_decay1_phi_after1st");
      truthTreeManager()->makeOutputVariable(m_MC_tty_Wminus_decay1_phi_afterall,  "MC_tty_Wminus_decay1_phi_afterall");
      truthTreeManager()->makeOutputVariable(m_MC_tty_Wminus_decay2_pt_before,     "MC_tty_Wminus_decay2_pt_before");
      truthTreeManager()->makeOutputVariable(m_MC_tty_Wminus_decay2_pt_after1st,   "MC_tty_Wminus_decay2_pt_after1st");
      truthTreeManager()->makeOutputVariable(m_MC_tty_Wminus_decay2_pt_afterall,   "MC_tty_Wminus_decay2_pt_afterall");
      truthTreeManager()->makeOutputVariable(m_MC_tty_Wminus_decay2_eta_before,    "MC_tty_Wminus_decay2_eta_before");
      truthTreeManager()->makeOutputVariable(m_MC_tty_Wminus_decay2_eta_after1st,  "MC_tty_Wminus_decay2_eta_after1st");
      truthTreeManager()->makeOutputVariable(m_MC_tty_Wminus_decay2_eta_afterall,  "MC_tty_Wminus_decay2_eta_afterall");
      truthTreeManager()->makeOutputVariable(m_MC_tty_Wminus_decay2_phi_before,    "MC_tty_Wminus_decay2_phi_before");
      truthTreeManager()->makeOutputVariable(m_MC_tty_Wminus_decay2_phi_after1st,  "MC_tty_Wminus_decay2_phi_after1st");
      truthTreeManager()->makeOutputVariable(m_MC_tty_Wminus_decay2_phi_afterall,  "MC_tty_Wminus_decay2_phi_afterall");
      truthTreeManager()->makeOutputVariable(m_MC_tty_bottom_pt_before,            "MC_tty_bottom_pt_before");
      truthTreeManager()->makeOutputVariable(m_MC_tty_bottom_pt_after1st,          "MC_tty_bottom_pt_after1st");
      truthTreeManager()->makeOutputVariable(m_MC_tty_bottom_pt_partonjet,         "MC_tty_bottom_pt_partonjet");
      truthTreeManager()->makeOutputVariable(m_MC_tty_bottom_pt_rbf_had,           "MC_tty_bottom_pt_rbf_had");
      truthTreeManager()->makeOutputVariable(m_MC_tty_bottom_eta_before,           "MC_tty_bottom_eta_before");
      truthTreeManager()->makeOutputVariable(m_MC_tty_bottom_eta_after1st,         "MC_tty_bottom_eta_after1st");
      truthTreeManager()->makeOutputVariable(m_MC_tty_bottom_eta_partonjet,        "MC_tty_bottom_eta_partonjet");
      truthTreeManager()->makeOutputVariable(m_MC_tty_bottom_eta_rbf_had,          "MC_tty_bottom_eta_rbf_had");
      truthTreeManager()->makeOutputVariable(m_MC_tty_bottom_phi_before,           "MC_tty_bottom_phi_before");
      truthTreeManager()->makeOutputVariable(m_MC_tty_bottom_phi_after1st,         "MC_tty_bottom_phi_after1st");
      truthTreeManager()->makeOutputVariable(m_MC_tty_bottom_phi_partonjet,        "MC_tty_bottom_phi_partonjet");
      truthTreeManager()->makeOutputVariable(m_MC_tty_bottom_phi_rbf_had,          "MC_tty_bottom_phi_rbf_had");
      truthTreeManager()->makeOutputVariable(m_MC_tty_bottom_m_partonjet,          "MC_tty_bottom_m_partonjet");
      truthTreeManager()->makeOutputVariable(m_MC_tty_bottom_m_rbf_had,            "MC_tty_bottom_m_rbf_had");
      truthTreeManager()->makeOutputVariable(m_MC_tty_antibottom_pt_before,        "MC_tty_antibottom_pt_before");
      truthTreeManager()->makeOutputVariable(m_MC_tty_antibottom_pt_after1st,      "MC_tty_antibottom_pt_after1st");
      truthTreeManager()->makeOutputVariable(m_MC_tty_antibottom_pt_partonjet,     "MC_tty_antibottom_pt_partonjet");
      truthTreeManager()->makeOutputVariable(m_MC_tty_antibottom_pt_rbf_had,       "MC_tty_antibottom_pt_rbf_had");
      truthTreeManager()->makeOutputVariable(m_MC_tty_antibottom_eta_before,       "MC_tty_antibottom_eta_before");
      truthTreeManager()->makeOutputVariable(m_MC_tty_antibottom_eta_after1st,     "MC_tty_antibottom_eta_after1st");
      truthTreeManager()->makeOutputVariable(m_MC_tty_antibottom_eta_partonjet,    "MC_tty_antibottom_eta_partonjet");
      truthTreeManager()->makeOutputVariable(m_MC_tty_antibottom_eta_rbf_had,      "MC_tty_antibottom_eta_rbf_had");
      truthTreeManager()->makeOutputVariable(m_MC_tty_antibottom_phi_before,       "MC_tty_antibottom_phi_before");
      truthTreeManager()->makeOutputVariable(m_MC_tty_antibottom_phi_after1st,     "MC_tty_antibottom_phi_after1st");
      truthTreeManager()->makeOutputVariable(m_MC_tty_antibottom_phi_partonjet,    "MC_tty_antibottom_phi_partonjet");
      truthTreeManager()->makeOutputVariable(m_MC_tty_antibottom_phi_rbf_had,      "MC_tty_antibottom_phi_rbf_had");
      truthTreeManager()->makeOutputVariable(m_MC_tty_antibottom_m_partonjet,      "MC_tty_antibottom_m_partonjet");
      truthTreeManager()->makeOutputVariable(m_MC_tty_antibottom_m_rbf_had,        "MC_tty_antibottom_m_rbf_had");
      truthTreeManager()->makeOutputVariable(m_MC_tty_partonjet_pdgIds,            "MC_tty_partonjet_pdgIds");
      truthTreeManager()->makeOutputVariable(m_MC_tty_partonjet_status,            "MC_tty_partonjet_status");
      truthTreeManager()->makeOutputVariable(m_MC_tty_partonjet_avpt,              "MC_tty_partonjet_avpt");
      truthTreeManager()->makeOutputVariable(m_MC_tty_Wplus_decay1_pdgId,          "MC_tty_Wplus_decay1_pdgId");
      truthTreeManager()->makeOutputVariable(m_MC_tty_Wplus_decay2_pdgId,          "MC_tty_Wplus_decay2_pdgId");
      truthTreeManager()->makeOutputVariable(m_MC_tty_Wminus_decay1_pdgId,         "MC_tty_Wminus_decay1_pdgId");
      truthTreeManager()->makeOutputVariable(m_MC_tty_Wminus_decay2_pdgId,         "MC_tty_Wminus_decay2_pdgId");
    }
    if (needs_parton_level(config->getDSID()) || needs_channel_info(config->getDSID())) {
      truthTreeManager()->makeOutputVariable(m_MC_tty_event_in_overlap,    "MC_tty_event_in_overlap");
      truthTreeManager()->makeOutputVariable(m_MC_tty_photon_pts_overlap,  "MC_tty_photon_pts_overlap");
      truthTreeManager()->makeOutputVariable(m_MC_tty_is_dilepton,         "MC_tty_is_dilepton");
      truthTreeManager()->makeOutputVariable(m_MC_tty_is_emu_channel,      "MC_tty_is_emu_channel");
    }
  }

  // Loop over all trees and add the custom variables.
  for (auto& tree : treeManagers()) {
    /// General event information variables.
    if (config->isMC() && needs_channel_info(config->getDSID())) {
      tree->makeOutputVariable(m_MC_tty_is_dilepton, "event_is_dilepton");
      tree->makeOutputVariable(m_MC_tty_is_emu_channel, "event_is_emu_channel");
    }
    tree->makeOutputVariable(m_event_dphill, "event_dphill");
    tree->makeOutputVariable(m_event_detall, "event_detall");
    tree->makeOutputVariable(m_event_drll, "event_drll");
    tree->makeOutputVariable(m_event_mll, "event_mll");
    tree->makeOutputVariable(m_event_mwt, "event_mwt");
    tree->makeOutputVariable(m_event_HT, "event_HT");
    tree->makeOutputVariable(m_event_ST, "event_ST");
    tree->makeOutputVariable(m_event_njets, "event_njets");
    tree->makeOutputVariable(m_event_nbjets70, "event_nbjets70");
    tree->makeOutputVariable(m_event_nbjets77, "event_nbjets77");
    tree->makeOutputVariable(m_event_nbjets85, "event_nbjets85");
    tree->makeOutputVariable(m_event_in_overlap, "event_in_overlap");
    tree->makeOutputVariable(m_event_in_overlap_TTH, "event_in_overlap_TTH");
    tree->makeOutputVariable(m_event_in_overlap_REL20, "event_in_overlap_REL20");

    // Photon quality variables.
    tree->makeOutputVariable(m_ph_good_index, "ph_good_index");

    // Photon truth variables.
    tree->makeOutputVariable(m_ph_true_category, "ph_true_category");
    tree->makeOutputVariable(m_ph_true_topparent, "ph_true_topparent");
    tree->makeOutputVariable(m_ph_true_parent, "ph_true_parent");
    tree->makeOutputVariable(m_ph_true_barcode, "ph_true_barcode");
    tree->makeOutputVariable(m_ph_true_origin, "ph_true_origin");
    tree->makeOutputVariable(m_ph_true_pdgid, "ph_true_pdgid");
    tree->makeOutputVariable(m_ph_true_type, "ph_true_type");
    tree->makeOutputVariable(m_ph_true_eta, "ph_true_eta");
    tree->makeOutputVariable(m_ph_true_phi, "ph_true_phi");
    tree->makeOutputVariable(m_ph_true_pt, "ph_true_pt");

    // Photon reconstruction variables.
    tree->makeOutputVariable(m_ph_conversion_type, "ph_conversion_type");
    tree->makeOutputVariable(m_ph_drmuph, "ph_drmuph");
    tree->makeOutputVariable(m_ph_drelph, "ph_drelph");
    tree->makeOutputVariable(m_ph_drlph_leading, "ph_drlph_leading");
    tree->makeOutputVariable(m_ph_drlph_subleading, "ph_drlph_subleading");
    tree->makeOutputVariable(m_ph_drlph_closest, "ph_drlph_closest");
    tree->makeOutputVariable(m_ph_mlph_closest, "ph_mlph_closest");
    tree->makeOutputVariable(m_ph_mllph, "ph_mllph");

    // Photon identification variables.
    tree->makeOutputVariable(m_ph_id_loose, "ph_id_loose");
    tree->makeOutputVariable(m_ph_id_tight, "ph_id_tight");
    tree->makeOutputVariable(m_ph_id_tight_relaxed, "ph_id_tight_relaxed");
    tree->makeOutputVariable(m_ph_id_fail_deltaE, "ph_id_fail_deltaE");
    tree->makeOutputVariable(m_ph_id_fail_fside, "ph_id_fail_fside");
    tree->makeOutputVariable(m_ph_id_fail_ws3, "ph_id_fail_ws3");
    tree->makeOutputVariable(m_ph_id_fail_Eratio, "ph_id_fail_Eratio");
    tree->makeOutputVariable(m_ph_id_fail_count, "ph_id_fail_count");

    // Photon isolation variables.
    tree->makeOutputVariable(m_ph_iso_ptcone20,     "ph_iso_ptcone20");
    // tree->makeOutputVariable(m_ph_iso_ptcone40,     "ph_iso_ptcone40");
    // tree->makeOutputVariable(m_ph_iso_ptvarcone20,  "ph_iso_ptvarcone20");
    // tree->makeOutputVariable(m_ph_iso_ptvarcone40,  "ph_iso_ptvarcone40");
    tree->makeOutputVariable(m_ph_iso_topoetcone20, "ph_iso_topoetcone20");
    tree->makeOutputVariable(m_ph_iso_topoetcone40, "ph_iso_topoetcone40");
    tree->makeOutputVariable(m_ph_iso_FCL,          "ph_iso_FCL");
    tree->makeOutputVariable(m_ph_iso_FCT,          "ph_iso_FCT");
    tree->makeOutputVariable(m_ph_iso_FCTCO,        "ph_iso_FCTCO");

    // Photon shower-shape variables.
    tree->makeOutputVariable(m_ph_ss_deltaE, "ph_ss_deltaE");
    tree->makeOutputVariable(m_ph_ss_e277,   "ph_ss_e277");
    tree->makeOutputVariable(m_ph_ss_emaxs1, "ph_ss_emaxs1");
    tree->makeOutputVariable(m_ph_ss_eratio, "ph_ss_eratio");
    tree->makeOutputVariable(m_ph_ss_f1,     "ph_ss_f1");
    tree->makeOutputVariable(m_ph_ss_fracs1, "ph_ss_fracs1");
    tree->makeOutputVariable(m_ph_ss_reta,   "ph_ss_reta");
    tree->makeOutputVariable(m_ph_ss_rhad,   "ph_ss_rhad");
    tree->makeOutputVariable(m_ph_ss_rhad1,  "ph_ss_rhad1");
    tree->makeOutputVariable(m_ph_ss_rphi,   "ph_ss_rphi");
    tree->makeOutputVariable(m_ph_ss_weta1,  "ph_ss_weta1");
    tree->makeOutputVariable(m_ph_ss_weta2,  "ph_ss_weta2");
    tree->makeOutputVariable(m_ph_ss_wtots1, "ph_ss_wtots1");

    // Photon-bjet variables.
    tree->makeOutputVariable(m_ph_drphb70_leading_pt, "ph_drphb70_leading_pt");
    tree->makeOutputVariable(m_ph_drphb77_leading_pt, "ph_drphb77_leading_pt");
    tree->makeOutputVariable(m_ph_drphb85_leading_pt, "ph_drphb85_leading_pt");
    tree->makeOutputVariable(m_ph_drphb70_subleading_pt, "ph_drphb70_subleading_pt");
    tree->makeOutputVariable(m_ph_drphb77_subleading_pt, "ph_drphb77_subleading_pt");
    tree->makeOutputVariable(m_ph_drphb85_subleading_pt, "ph_drphb85_subleading_pt");
    tree->makeOutputVariable(m_ph_drphb70_leading_score, "ph_drphb70_leading_score");
    tree->makeOutputVariable(m_ph_drphb77_leading_score, "ph_drphb77_leading_score");
    tree->makeOutputVariable(m_ph_drphb85_leading_score, "ph_drphb85_leading_score");
    tree->makeOutputVariable(m_ph_drphb70_subleading_score, "ph_drphb70_subleading_score");
    tree->makeOutputVariable(m_ph_drphb77_subleading_score, "ph_drphb77_subleading_score");
    tree->makeOutputVariable(m_ph_drphb85_subleading_score, "ph_drphb85_subleading_score");
    tree->makeOutputVariable(m_ph_drphb70_closest, "ph_drphb70_closest");
    tree->makeOutputVariable(m_ph_drphb77_closest, "ph_drphb77_closest");
    tree->makeOutputVariable(m_ph_drphb85_closest, "ph_drphb85_closest");
    tree->makeOutputVariable(m_ph_mphb70_closest,  "ph_mphb70_closest");
    tree->makeOutputVariable(m_ph_mphb77_closest,  "ph_mphb77_closest");
    tree->makeOutputVariable(m_ph_mphb85_closest,  "ph_mphb85_closest");

    // Lepton-bjet variables.
    tree->makeOutputVariable(m_el_mlb70,  "el_mlb70");
    tree->makeOutputVariable(m_el_mlb77,  "el_mlb77");
    tree->makeOutputVariable(m_el_mlb85,  "el_mlb85");
    tree->makeOutputVariable(m_el_drlb70, "el_drlb70");
    tree->makeOutputVariable(m_el_drlb77, "el_drlb77");
    tree->makeOutputVariable(m_el_drlb85, "el_drlb85");
    tree->makeOutputVariable(m_mu_mlb70,  "mu_mlb70");
    tree->makeOutputVariable(m_mu_mlb77,  "mu_mlb77");
    tree->makeOutputVariable(m_mu_mlb85,  "mu_mlb85");
    tree->makeOutputVariable(m_mu_drlb70, "mu_drlb70");
    tree->makeOutputVariable(m_mu_drlb77, "mu_drlb77");
    tree->makeOutputVariable(m_mu_drlb85, "mu_drlb85");

    // B-tagging variables.
    tree->makeOutputVariable(m_jet_mv2c10_ranked_index, "jet_mv2c10_ranked_index");

    // Variables to check for pile-up photons.
    tree->makeOutputVariable(m_event_PV_z, "event_PV_z");
    tree->makeOutputVariable(m_ph_z, "ph_z");
    tree->makeOutputVariable(m_ph_dz, "ph_dz");
    tree->makeOutputVariable(m_ph_dz_ratio, "ph_dz_ratio");
  }
}

// -----------------------------------------------
bool EventSaver::qualifiesAsGood(const xAOD::Photon& photon) {
  if (photon.pt() < 20e3) return false;
  if (!photon.isAvailable<char>("DFCommonPhotonsIsEMTight")) return false;
  if (!photon.auxdataConst<char>("DFCommonPhotonsIsEMTight")) return false;

  return true;
}

// -----------------------------------------------
void EventSaver::resizePhotonVectors(const top::Event& event) {
  const size_t n_photons{event.m_photons.size()};
  m_ph_true_category.resize(n_photons);
  m_ph_true_topparent.resize(n_photons);
  m_ph_true_parent.resize(n_photons);
  m_ph_true_barcode.resize(n_photons);
  m_ph_true_origin.resize(n_photons);
  m_ph_true_pdgid.resize(n_photons);
  m_ph_true_type.resize(n_photons);
  m_ph_true_eta.resize(n_photons);
  m_ph_true_phi.resize(n_photons);
  m_ph_true_pt.resize(n_photons);
  m_ph_conversion_type.resize(n_photons);
  m_ph_drmuph.resize(n_photons);
  m_ph_drelph.resize(n_photons);
  m_ph_drlph_leading.resize(n_photons);
  m_ph_drlph_subleading.resize(n_photons);
  m_ph_drlph_closest.resize(n_photons);
  m_ph_mlph_closest.resize(n_photons);
  m_ph_mllph.resize(n_photons);
  m_ph_id_loose.resize(n_photons);
  m_ph_id_tight.resize(n_photons);
  m_ph_id_tight_relaxed.resize(n_photons);
  m_ph_id_fail_deltaE.resize(n_photons);
  m_ph_id_fail_fside.resize(n_photons);
  m_ph_id_fail_ws3.resize(n_photons);
  m_ph_id_fail_Eratio.resize(n_photons);
  m_ph_id_fail_count.resize(n_photons);
  m_ph_iso_ptcone20.resize(n_photons);
  // m_ph_iso_ptcone40.resize(n_photons);
  // m_ph_iso_ptvarcone20.resize(n_photons);
  // m_ph_iso_ptvarcone40.resize(n_photons);
  m_ph_iso_topoetcone20.resize(n_photons);
  m_ph_iso_topoetcone40.resize(n_photons);
  m_ph_iso_FCL.resize(n_photons);
  m_ph_iso_FCT.resize(n_photons);
  m_ph_iso_FCTCO.resize(n_photons);
  m_ph_ss_deltaE.resize(n_photons);
  m_ph_ss_e277.resize(n_photons);
  m_ph_ss_emaxs1.resize(n_photons);
  m_ph_ss_eratio.resize(n_photons);
  m_ph_ss_f1.resize(n_photons);
  m_ph_ss_fracs1.resize(n_photons);
  m_ph_ss_reta.resize(n_photons);
  m_ph_ss_rhad.resize(n_photons);
  m_ph_ss_rhad1.resize(n_photons);
  m_ph_ss_rphi.resize(n_photons);
  m_ph_ss_weta1.resize(n_photons);
  m_ph_ss_weta2.resize(n_photons);
  m_ph_ss_wtots1.resize(n_photons);
  m_ph_drphb70_leading_pt.resize(n_photons);
  m_ph_drphb77_leading_pt.resize(n_photons);
  m_ph_drphb85_leading_pt.resize(n_photons);
  m_ph_drphb70_subleading_pt.resize(n_photons);
  m_ph_drphb77_subleading_pt.resize(n_photons);
  m_ph_drphb85_subleading_pt.resize(n_photons);
  m_ph_drphb70_leading_score.resize(n_photons);
  m_ph_drphb77_leading_score.resize(n_photons);
  m_ph_drphb85_leading_score.resize(n_photons);
  m_ph_drphb70_subleading_score.resize(n_photons);
  m_ph_drphb77_subleading_score.resize(n_photons);
  m_ph_drphb85_subleading_score.resize(n_photons);
  m_ph_drphb70_closest.resize(n_photons);
  m_ph_drphb77_closest.resize(n_photons);
  m_ph_drphb85_closest.resize(n_photons);
  m_ph_mphb70_closest.resize(n_photons);
  m_ph_mphb77_closest.resize(n_photons);
  m_ph_mphb85_closest.resize(n_photons);
  m_ph_z.resize(n_photons);
  m_ph_dz.resize(n_photons);
  m_ph_dz_ratio.resize(n_photons);
}


// -----------------------------------------------
void EventSaver::saveTruthEvent() {
  // Skip entire calculation if no parton level info needed.
  if (!needs_parton_level(topConfig()->getDSID()) && !needs_channel_info(topConfig()->getDSID())) {
    top::EventSaverFlatNtuple::saveTruthEvent();
    return;
  }

  // Store sample overlap information.
  bool in_overlap;
  if (topConfig()->isMC()) {
    top::check(m_vgamma_tool->inOverlap(in_overlap), "Could not execute VGammaORTool::inOverlap(bool)");
    top::check(m_vgamma_tool->photonPtsOutsideDr(m_MC_tty_photon_pts_overlap), "Could not execute VGammaORTool::photonPtsOutsideDr(vector)");
  }
  m_MC_tty_event_in_overlap = static_cast<char>(in_overlap);

  const xAOD::TruthParticleContainer* truth(nullptr);
  top::check( evtStore()->retrieve(truth, topConfig()->sgKeyMCParticle()) , "Failed to retrieve TruthParticleContainer" );

  // Reset all variables to dummy values first.
  m_MC_tty_ph_pt_before = -1;
  m_MC_tty_ph_pt_after1st = -1;
  m_MC_tty_ph_pt_afterall = -1;
  m_MC_tty_ph_eta_before = -10;
  m_MC_tty_ph_eta_after1st = -10;
  m_MC_tty_ph_eta_afterall = -10;
  m_MC_tty_ph_phi_before = -10;
  m_MC_tty_ph_phi_after1st = -10;
  m_MC_tty_ph_phi_afterall = -10;
  m_MC_tty_Wplus_decay1_pt_before = -1;
  m_MC_tty_Wplus_decay1_pt_after1st = -1;
  m_MC_tty_Wplus_decay1_pt_afterall = -1;
  m_MC_tty_Wplus_decay1_eta_before = -10;
  m_MC_tty_Wplus_decay1_eta_after1st = -10;
  m_MC_tty_Wplus_decay1_eta_afterall = -10;
  m_MC_tty_Wplus_decay1_phi_before = -10;
  m_MC_tty_Wplus_decay1_phi_after1st = -10;
  m_MC_tty_Wplus_decay1_phi_afterall = -10;
  m_MC_tty_Wplus_decay2_pt_before = -1;
  m_MC_tty_Wplus_decay2_pt_after1st = -1;
  m_MC_tty_Wplus_decay2_pt_afterall = -1;
  m_MC_tty_Wplus_decay2_eta_before = -10;
  m_MC_tty_Wplus_decay2_eta_after1st = -10;
  m_MC_tty_Wplus_decay2_eta_afterall = -10;
  m_MC_tty_Wplus_decay2_phi_before = -10;
  m_MC_tty_Wplus_decay2_phi_after1st = -10;
  m_MC_tty_Wplus_decay2_phi_afterall = -10;
  m_MC_tty_Wminus_decay1_pt_before = -1;
  m_MC_tty_Wminus_decay1_pt_after1st = -1;
  m_MC_tty_Wminus_decay1_pt_afterall = -1;
  m_MC_tty_Wminus_decay1_eta_before = -10;
  m_MC_tty_Wminus_decay1_eta_after1st = -10;
  m_MC_tty_Wminus_decay1_eta_afterall = -10;
  m_MC_tty_Wminus_decay1_phi_before = -10;
  m_MC_tty_Wminus_decay1_phi_after1st = -10;
  m_MC_tty_Wminus_decay1_phi_afterall = -10;
  m_MC_tty_Wminus_decay2_pt_before = -1;
  m_MC_tty_Wminus_decay2_pt_after1st = -1;
  m_MC_tty_Wminus_decay2_pt_afterall = -1;
  m_MC_tty_Wminus_decay2_eta_before = -10;
  m_MC_tty_Wminus_decay2_eta_after1st = -10;
  m_MC_tty_Wminus_decay2_eta_afterall = -10;
  m_MC_tty_Wminus_decay2_phi_before = -10;
  m_MC_tty_Wminus_decay2_phi_after1st = -10;
  m_MC_tty_Wminus_decay2_phi_afterall = -10;
  m_MC_tty_bottom_pt_before = -1;
  m_MC_tty_bottom_pt_after1st = -1;
  m_MC_tty_bottom_pt_partonjet = -1;
  m_MC_tty_bottom_pt_rbf_had = -1;
  m_MC_tty_bottom_eta_before = -10;
  m_MC_tty_bottom_eta_after1st = -10;
  m_MC_tty_bottom_eta_partonjet = -10;
  m_MC_tty_bottom_eta_rbf_had = -10;
  m_MC_tty_bottom_phi_before = -10;
  m_MC_tty_bottom_phi_after1st = -10;
  m_MC_tty_bottom_phi_partonjet = -10;
  m_MC_tty_bottom_phi_rbf_had = -10;
  m_MC_tty_bottom_m_partonjet = -10;
  m_MC_tty_bottom_m_rbf_had = -10;
  m_MC_tty_antibottom_pt_before = -1;
  m_MC_tty_antibottom_pt_after1st = -1;
  m_MC_tty_antibottom_pt_partonjet = -1;
  m_MC_tty_antibottom_pt_rbf_had = -1;
  m_MC_tty_antibottom_eta_before = -10;
  m_MC_tty_antibottom_eta_after1st = -10;
  m_MC_tty_antibottom_eta_partonjet = -10;
  m_MC_tty_antibottom_eta_rbf_had = -10;
  m_MC_tty_antibottom_phi_before = -10;
  m_MC_tty_antibottom_phi_after1st = -10;
  m_MC_tty_antibottom_phi_partonjet = -10;
  m_MC_tty_antibottom_phi_rbf_had = -10;
  m_MC_tty_antibottom_m_partonjet = -10;
  m_MC_tty_antibottom_m_rbf_had = -10;
  m_MC_tty_Wplus_decay1_pdgId = 0;
  m_MC_tty_Wplus_decay2_pdgId = 0;
  m_MC_tty_Wminus_decay1_pdgId = 0;
  m_MC_tty_Wminus_decay2_pdgId = 0;

  m_MC_tty_is_dilepton = false;
  m_MC_tty_is_emu_channel = false;

  PartonLevelInfo parton_level{truth, m_is_tWy};
  const auto& photon = parton_level.getPhotonBefore();
  const auto& photon_after1st = parton_level.getPhotonAfter1st();
  const auto& photon_afterall = parton_level.getPhotonAfterall();
  ParticleSet photons_afterall = parton_level.getPhotonsAfterall();
  const auto& Wplus_decay1 = parton_level.getWPlusDecay1Before();
  const auto& Wplus_decay1_after1st = parton_level.getWPlusDecay1After1st();
  const auto& Wplus_decay1_afterall = parton_level.getWPlusDecay1Afterall();
  const auto& Wplus_decay2 = parton_level.getWPlusDecay2Before();
  const auto& Wplus_decay2_after1st = parton_level.getWPlusDecay2After1st();
  const auto& Wplus_decay2_afterall = parton_level.getWPlusDecay2Afterall();
  const auto& Wminus_decay1 = parton_level.getWMinusDecay1Before();
  const auto& Wminus_decay1_after1st = parton_level.getWMinusDecay1After1st();
  const auto& Wminus_decay1_afterall = parton_level.getWMinusDecay1Afterall();
  const auto& Wminus_decay2 = parton_level.getWMinusDecay2Before();
  const auto& Wminus_decay2_after1st = parton_level.getWMinusDecay2After1st();
  const auto& Wminus_decay2_afterall = parton_level.getWMinusDecay2Afterall();
  const auto& bottom = parton_level.getBottomQuarkBefore();
  const auto& bottom_after1st = parton_level.getBottomQuarkAfter1st();
  const auto& antibottom = parton_level.getAntibottomQuarkBefore();
  const auto& antibottom_after1st = parton_level.getAntibottomQuarkAfter1st();

  // -----------------------------------------------
  // Create parton jets.
  // -----------------------------------------------

  // 1. Find the b-quarks right before hadronization.
  const auto& bottom_rbf_had = parton_level.getBottomQuarkRbfHad();
  const auto& antibottom_rbf_had = parton_level.getAntibottomQuarkRbfHad();
  TLorentzVector b_rbf_had;
  TLorentzVector bbar_rbf_had;
  if (bottom_rbf_had) {
    b_rbf_had.SetPtEtaPhiM(bottom_rbf_had->pt(), bottom_rbf_had->eta(), bottom_rbf_had->phi(), bottom_rbf_had->m());
    m_MC_tty_bottom_pt_rbf_had = bottom_rbf_had->pt();
    m_MC_tty_bottom_eta_rbf_had = bottom_rbf_had->eta();
    m_MC_tty_bottom_phi_rbf_had = bottom_rbf_had->phi();
    m_MC_tty_bottom_m_rbf_had = bottom_rbf_had->m();
  }
  if (antibottom_rbf_had) {
    bbar_rbf_had.SetPtEtaPhiM(antibottom_rbf_had->pt(), antibottom_rbf_had->eta(), antibottom_rbf_had->phi(), antibottom_rbf_had->m());
    m_MC_tty_antibottom_pt_rbf_had = antibottom_rbf_had->pt();
    m_MC_tty_antibottom_eta_rbf_had = antibottom_rbf_had->eta();
    m_MC_tty_antibottom_phi_rbf_had = antibottom_rbf_had->phi();
    m_MC_tty_antibottom_m_rbf_had = antibottom_rbf_had->m();
  }

  // 2. Find the partons emitted by the b-quarks (including
  // itself) and incoming partons right before hadronization.
  ParticleSet descendants_rbf_had_bottom = parton_level.getBDescendants();
  ParticleSet descendants_rbf_had_antibottom = parton_level.getAntiBDescendants();
  ParticleSet descendants_rbf_had_isr = parton_level.getISRDescendants();
  ParticleSet descendants_rbf_had_bbbar;
  std::set_union(descendants_rbf_had_bottom.begin(), descendants_rbf_had_bottom.end(),
                 descendants_rbf_had_antibottom.begin(), descendants_rbf_had_antibottom.end(),
                 std::inserter(descendants_rbf_had_bbbar, descendants_rbf_had_bbbar.begin()));
  ParticleSet descendants_rbf_had;
  std::set_union(descendants_rbf_had_bbbar.begin(), descendants_rbf_had_bbbar.end(),
                 descendants_rbf_had_isr.begin(), descendants_rbf_had_isr.end(),
                 std::inserter(descendants_rbf_had, descendants_rbf_had.begin()));

  // 3. Form pseudojets. Use the user_index to identify b quark,
  // simultaneously do Frixione isolation (here sampling four
  // descrete points 0.1/0.2/0.3/0.4).
  //
  // https://arxiv.org/pdf/hep-ph/9801442.pdf
  std::vector<fastjet::PseudoJet> particles;
  TLorentzVector tlvph;
  if (photon_afterall) tlvph.SetPtEtaPhiM(photon_afterall->pt(), photon_afterall->eta(), photon_afterall->phi(), 0);
  std::vector<double> parton_photon_dR;
  std::vector<double> parton_Et;
  m_MC_tty_partonjet_pdgIds.clear();
  m_MC_tty_partonjet_status.clear();
  m_MC_tty_partonjet_avpt = 0;

  for (const auto& p : descendants_rbf_had) {
    m_MC_tty_partonjet_pdgIds.push_back(p->pdgId());
    m_MC_tty_partonjet_status.push_back(p->status());
    m_MC_tty_partonjet_avpt += p->pt();

    fastjet::PseudoJet jet(p->px(), p->py(), p->pz(), p->e());

    // Apply b-tagging using parton information.
    if (bottom_rbf_had && p->barcode() == bottom_rbf_had->barcode()) {
      jet.set_user_index(5);
    } else if (antibottom_rbf_had && p->barcode() == antibottom_rbf_had->barcode()) {
      jet.set_user_index(-5);
    } else {
      jet.set_user_index(0);
    }
    particles.push_back(jet);

    TLorentzVector tlvjet;
    tlvjet.SetPxPyPzE(p->px(), p->py(), p->pz(), p->e());
    double dR = tlvjet.DeltaR(tlvph);
    if (dR > 0.4) continue;
    parton_photon_dR.push_back(dR);
    parton_Et.push_back(tlvjet.Pt());
  }

  if (m_MC_tty_partonjet_pdgIds.size() != 0) {
    m_MC_tty_partonjet_avpt /= m_MC_tty_partonjet_pdgIds.size();
  }
  bool photoniso = true;
  double et_max_0 = tlvph.Pt() / (1 - std::cos(0.4));

  for (size_t i = 0; i < parton_Et.size(); i++) {
    // This first loop just loops for a set of dR threshold, each
    // dR will be used to perform an isolation selection.
    double conesize = parton_photon_dR.at(i);
    double et_max = et_max_0 * (1 - cos(conesize));
    double et = 0;
    for (size_t j = 0; j < parton_Et.size(); j++) {
      // This second loop tries to sum all pT with dR smaller
      // than the reference dR.
      double conesize2 = parton_photon_dR.at(j);
      if (conesize2 <= conesize) {
        et += parton_Et.at(j);
      }
    }
    if (et > et_max) {
      photoniso = false;
      break;
    }
  }

  m_MC_tty_ph_isIso_afterall = static_cast<char>(photoniso);

  // 4. Create a fastjet::ClusterSequence object and form the jets.
  double R= 0.4;
  fastjet::JetDefinition jet_def(fastjet::antikt_algorithm, R);
  std::shared_ptr<fastjet::ClusterSequence> clustersequence = std::shared_ptr<fastjet::ClusterSequence>(new fastjet::ClusterSequence(particles, jet_def));
  std::vector<fastjet::PseudoJet> jets = fastjet::sorted_by_pt(clustersequence->inclusive_jets());

  // 5. Tag the two b-jets.
  for (size_t i = 0; i < jets.size(); i++) {
    std::vector<fastjet::PseudoJet> constituents = jets[i].constituents();
    bool isb = false;
    bool isbbar = false;
    for (size_t j = 0; j < constituents.size(); j++) {
      if (constituents[j].user_index() == 5) isb = true;
      if (constituents[j].user_index() == -5) isbbar = true;
    }

    TLorentzVector j;
    j.SetPtEtaPhiM(jets[i].pt(), jets[i].eta(), jets[i].phi(), jets[i].m());
    if (isb && isbbar) {
      // The case where both b-quarks are clustered into a single jet.
      if (j.DeltaR(b_rbf_had) < j.DeltaR(bbar_rbf_had)) {
        m_MC_tty_bottom_pt_partonjet = jets[i].perp();
        m_MC_tty_bottom_eta_partonjet = jets[i].eta();
        m_MC_tty_bottom_phi_partonjet = jets[i].phi();
        m_MC_tty_bottom_m_partonjet = jets[i].m();
      } else {
        m_MC_tty_antibottom_pt_partonjet = jets[i].perp();
        m_MC_tty_antibottom_eta_partonjet = jets[i].eta();
        m_MC_tty_antibottom_phi_partonjet = jets[i].phi();
        m_MC_tty_antibottom_m_partonjet = jets[i].m();
      }
    } else if (isb && !isbbar) {
      m_MC_tty_bottom_pt_partonjet = jets[i].perp();
      m_MC_tty_bottom_eta_partonjet = jets[i].eta();
      m_MC_tty_bottom_phi_partonjet = jets[i].phi();
      m_MC_tty_bottom_m_partonjet = jets[i].m();
    } else if (!isb && isbbar) {
      m_MC_tty_antibottom_pt_partonjet = jets[i].perp();
      m_MC_tty_antibottom_eta_partonjet = jets[i].eta();
      m_MC_tty_antibottom_phi_partonjet = jets[i].phi();
      m_MC_tty_antibottom_m_partonjet = jets[i].m();
    }
  }  // Loop over all jets.

  m_MC_tty_is_dilepton = parton_level.isDilepton();
  m_MC_tty_is_emu_channel = parton_level.isEMuChannel();

  if (photon) {
    m_MC_tty_ph_pt_before               = photon->pt();
    m_MC_tty_ph_eta_before              = photon->eta();
    m_MC_tty_ph_phi_before              = photon->phi();
  }
  if (Wplus_decay1) {
    m_MC_tty_Wplus_decay1_pt_before     = Wplus_decay1->pt();
    m_MC_tty_Wplus_decay1_eta_before    = Wplus_decay1->eta();
    m_MC_tty_Wplus_decay1_phi_before    = Wplus_decay1->phi();
    m_MC_tty_Wplus_decay1_pdgId         = Wplus_decay1->pdgId();
  }
  if (Wplus_decay2) {
    m_MC_tty_Wplus_decay2_pt_before     = Wplus_decay2->pt();
    m_MC_tty_Wplus_decay2_eta_before    = Wplus_decay2->eta();
    m_MC_tty_Wplus_decay2_phi_before    = Wplus_decay2->phi();
    m_MC_tty_Wplus_decay2_pdgId         = Wplus_decay2->pdgId();
  }
  if (Wminus_decay1) {
    m_MC_tty_Wminus_decay1_pt_before    = Wminus_decay1->pt();
    m_MC_tty_Wminus_decay1_eta_before   = Wminus_decay1->eta();
    m_MC_tty_Wminus_decay1_phi_before   = Wminus_decay1->phi();
    m_MC_tty_Wminus_decay1_pdgId        = Wminus_decay1->pdgId();
  }
  if (Wminus_decay2) {
    m_MC_tty_Wminus_decay2_pt_before    = Wminus_decay2->pt();
    m_MC_tty_Wminus_decay2_eta_before   = Wminus_decay2->eta();
    m_MC_tty_Wminus_decay2_phi_before   = Wminus_decay2->phi();
    m_MC_tty_Wminus_decay2_pdgId        = Wminus_decay2->pdgId();
  }
  if (bottom) {
    m_MC_tty_bottom_pt_before           = bottom->pt();
    m_MC_tty_bottom_eta_before          = bottom->eta();
    m_MC_tty_bottom_phi_before          = bottom->phi();
  }
  if (antibottom) {
    m_MC_tty_antibottom_pt_before       = antibottom->pt();
    m_MC_tty_antibottom_eta_before      = antibottom->eta();
    m_MC_tty_antibottom_phi_before      = antibottom->phi();
  }

  if (photon_after1st) {
    m_MC_tty_ph_pt_after1st               = photon_after1st->pt();
    m_MC_tty_ph_eta_after1st              = photon_after1st->eta();
    m_MC_tty_ph_phi_after1st              = photon_after1st->phi();
  }
  if (photon_afterall) {
    m_MC_tty_ph_pt_afterall               = photon_afterall->pt();
    m_MC_tty_ph_eta_afterall              = photon_afterall->eta();
    m_MC_tty_ph_phi_afterall              = photon_afterall->phi();
  }
  if (Wplus_decay1_after1st) {
    m_MC_tty_Wplus_decay1_pt_after1st     = Wplus_decay1_after1st->pt();
    m_MC_tty_Wplus_decay1_eta_after1st    = Wplus_decay1_after1st->eta();
    m_MC_tty_Wplus_decay1_phi_after1st    = Wplus_decay1_after1st->phi();
  }
  if (Wplus_decay2_after1st) {
    m_MC_tty_Wplus_decay2_pt_after1st     = Wplus_decay2_after1st->pt();
    m_MC_tty_Wplus_decay2_eta_after1st    = Wplus_decay2_after1st->eta();
    m_MC_tty_Wplus_decay2_phi_after1st    = Wplus_decay2_after1st->phi();
  }
  if (Wminus_decay1_after1st) {
    m_MC_tty_Wminus_decay1_pt_after1st    = Wminus_decay1_after1st->pt();
    m_MC_tty_Wminus_decay1_eta_after1st   = Wminus_decay1_after1st->eta();
    m_MC_tty_Wminus_decay1_phi_after1st   = Wminus_decay1_after1st->phi();
  }
  if (Wminus_decay2_after1st) {
    m_MC_tty_Wminus_decay2_pt_after1st    = Wminus_decay2_after1st->pt();
    m_MC_tty_Wminus_decay2_eta_after1st   = Wminus_decay2_after1st->eta();
    m_MC_tty_Wminus_decay2_phi_after1st   = Wminus_decay2_after1st->phi();
  }

  if (Wplus_decay1_afterall) {
    TLorentzVector decay;
    // Set the coordinates, ignore the mass effect.
    decay.SetPtEtaPhiM(Wplus_decay1_afterall->pt(),
                       Wplus_decay1_afterall->eta(),
                       Wplus_decay1_afterall->phi(),
                       0);

    if (abs(m_MC_tty_Wplus_decay1_pdgId) == 11 || abs(m_MC_tty_Wplus_decay1_pdgId) == 13) {
      // Do photon dressing for direct e/mu final states.
      for (const auto& p : photons_afterall) {
        // Ignore the matrix-element photon.
        if (photon_afterall != nullptr && p->barcode() == photon_afterall->barcode()) continue;

        TLorentzVector ph;
        ph.SetPtEtaPhiM(p->pt(), p->eta(), p->phi(), 0);
        if (decay.DeltaR(ph) < 0.1) decay += ph;
      }
    }

    m_MC_tty_Wplus_decay1_pt_afterall     = decay.Pt();
    m_MC_tty_Wplus_decay1_eta_afterall    = decay.Eta();
    m_MC_tty_Wplus_decay1_phi_afterall    = decay.Phi();
  }

  if (Wplus_decay2_afterall) {
    TLorentzVector decay;
    // Set the coordinates, ignore the mass effect.
    decay.SetPtEtaPhiM(Wplus_decay2_afterall->pt(),
                       Wplus_decay2_afterall->eta(),
                       Wplus_decay2_afterall->phi(),
                       0);

    if (abs(m_MC_tty_Wplus_decay2_pdgId) == 11 || abs(m_MC_tty_Wplus_decay2_pdgId) == 13) {
      // Do photon dressing for direct e/mu final states.
      for (const auto& p : photons_afterall) {
        // Ignore the matrix-element photon.
        if (photon_afterall != nullptr && p->barcode() == photon_afterall->barcode()) continue; // ignore the ME photon

        TLorentzVector ph; ph.SetPtEtaPhiM(p->pt(), p->eta(), p->phi(), 0);
        if (decay.DeltaR(ph) < 0.1) decay += ph;
      }
    }

    m_MC_tty_Wplus_decay2_pt_afterall     = decay.Pt();
    m_MC_tty_Wplus_decay2_eta_afterall    = decay.Eta();
    m_MC_tty_Wplus_decay2_phi_afterall    = decay.Phi();
  }

  if (Wminus_decay1_afterall) {
    TLorentzVector decay;
    // Set the coordinates, ignore the mass effect.
    decay.SetPtEtaPhiM(Wminus_decay1_afterall->pt(),
                       Wminus_decay1_afterall->eta(),
                       Wminus_decay1_afterall->phi(),
                       0);

    if (abs(m_MC_tty_Wminus_decay1_pdgId) == 11 || abs(m_MC_tty_Wminus_decay1_pdgId) == 13) {
      // Do photon dressing for direct e/mu final states.
      for (const auto& p : photons_afterall) {
        // Ignore the matrix-element photon.
        if (photon_afterall != nullptr && p->barcode() == photon_afterall->barcode()) continue;

        TLorentzVector ph; ph.SetPtEtaPhiM(p->pt(), p->eta(), p->phi(), 0);
        if (decay.DeltaR(ph) < 0.1) decay += ph;
      }
    }

    m_MC_tty_Wminus_decay1_pt_afterall    = decay.Pt();
    m_MC_tty_Wminus_decay1_eta_afterall   = decay.Eta();
    m_MC_tty_Wminus_decay1_phi_afterall   = decay.Phi();
  }

  if (Wminus_decay2_afterall) {
    TLorentzVector decay;
    // Set the coordinates, ignore the mass effect.
    decay.SetPtEtaPhiM(Wminus_decay2_afterall->pt(),
                       Wminus_decay2_afterall->eta(),
                       Wminus_decay2_afterall->phi(),
                       0);

    if (abs(m_MC_tty_Wminus_decay2_pdgId) == 11 || abs(m_MC_tty_Wminus_decay2_pdgId) == 13) {
      // Do photon dressing for direct e/mu final states.
      for (const auto& p : photons_afterall) {
        // Ignore the matrix-element photon.
        if (photon_afterall != nullptr && p->barcode() == photon_afterall->barcode()) continue;
        TLorentzVector ph; ph.SetPtEtaPhiM(p->pt(), p->eta(), p->phi(), 0);
        if (decay.DeltaR(ph) < 0.1) decay += ph;
      }
    }

    m_MC_tty_Wminus_decay2_pt_afterall    = decay.Pt();
    m_MC_tty_Wminus_decay2_eta_afterall   = decay.Eta();
    m_MC_tty_Wminus_decay2_phi_afterall   = decay.Phi();
  }

  if (bottom_after1st) {
    m_MC_tty_bottom_pt_after1st           = bottom_after1st->pt();
    m_MC_tty_bottom_eta_after1st          = bottom_after1st->eta();
    m_MC_tty_bottom_phi_after1st          = bottom_after1st->phi();
  }
  if (antibottom_after1st) {
    m_MC_tty_antibottom_pt_after1st       = antibottom_after1st->pt();
    m_MC_tty_antibottom_eta_after1st      = antibottom_after1st->eta();
    m_MC_tty_antibottom_phi_after1st      = antibottom_after1st->phi();
  }

  // Let the base class take care of the event saving.
  top::EventSaverFlatNtuple::saveTruthEvent();
}

// -----------------------------------------------
void EventSaver::saveEvent(const top::Event& event) {
  // Resize all internal vectors to the correct size (depending
  // on how many photons the event has).
  resizePhotonVectors(event);

  // get PV z
  const xAOD::VertexContainer* primaryVertices(nullptr);
  top::check(evtStore()->retrieve(primaryVertices,top::EventSaverFlatNtuple::topConfig()->sgKeyPrimaryVertices()), "Failed to retrieve PrimaryVertices");
  m_event_PV_z = -999999;
  for (const auto & vertex: *primaryVertices) {
    if (vertex->vertexType() == 1) { // 1 means PV
      m_event_PV_z = vertex->z();
      break;
    }
  }

  for (auto ph_itr = event.m_photons.begin(); ph_itr != event.m_photons.end(); ++ph_itr) {
    const auto& i{ph_itr - event.m_photons.begin()};
    const auto& photon{*ph_itr};

    // Retrieve the photon index and decorate the photon with it.
    photon->auxdecor<int>("i") = i;

    m_ph_conversion_type[i] = photon->conversionType();

    // get photon z
    std::pair<float,float> ph_z = m_photonpointing_tool->getCaloPointing(photon);
    m_ph_z[i] = ph_z.first;
    m_ph_dz[i] = ph_z.second;
    m_ph_dz_ratio[i] = (ph_z.second == 0. ? 0. : (ph_z.first - m_event_PV_z) / ph_z.second);
  }

  TruthElectronExtractor electron_extractor{event};
  const auto& truth_electrons = electron_extractor.TruthElectrons();

  // Although everything could be done here, this is split into
  // smaller methods for better readability. The good-photon
  // indices should be stored first, as other methods need them.
  top::check(storeGoodPhotonIndices(event.m_photons), "top::ttgamma::EventSaver: \tCould not save good photon indices");
  top::check(storeOverlapRemovalInfo(event), "top::ttgamma::EventSaver: \tCould not save overlap removal info");
  top::check(storePhotonIdentificationVariables(event.m_photons), "top::ttgamma::EventSaver: \tCould not save photon identification variables");
  top::check(storePhotonIsolationVariables(event.m_photons), "top::ttgamma::EventSaver: \tCould not save photon isolation variables");
  top::check(storePhotonShowerShapes(event.m_photons), "top::ttgamma::EventSaver: \tCould not save photon shower shapes");
  top::check(storePhotonTruthInfo(event.m_photons, truth_electrons), "top::ttgamma::EventSaver: \tCould not save photon truh information");

  top::check(calculateBjetVariables(event.m_jets), "top::ttgamma::Eventsaver: \tError retrieving b-tagging variables");
  top::check(calculateJetMultiplicities(event.m_jets), "top::ttgamma::EventSaver: \tError retrieving jet multiplicities");
  top::check(calculateHT(event), "top::ttgamma::EventSaver: \tCould not calculate H_T variable");
  top::check(calculateST(event), "top::ttgamma::EventSaver: \tCould not calculate S_T variable");
  top::check(calculateLeptonBjetVariables(event.m_jets, event.m_electrons, event.m_muons), "top::ttgamma::Eventsaver: \tCould not calculate lepton-bjet variables");
  top::check(calculateDileptonVariables(event.m_electrons, event.m_muons), "top::ttgamma::EventSaver: \tCould not calculate dilepton variables");
  top::check(calculateMWT(event.m_electrons, event.m_muons, *event.m_met), "top::ttgamma::eventsaver: \tCould not calculate mwt variable");
  top::check(calculatePhotonBjetVariables(event.m_jets, event.m_photons), "top::ttgamma::Eventsaver: \tCould not calculate photon-bjet variables");
  top::check(calculatePhotonLeptonVariables(event), "top::ttgamma::eventsaver: \tCould not calculate photon lepton variables");

  // Let the base class take care of the event saving.
  top::EventSaverFlatNtuple::saveEvent(event);
}

// -----------------------------------------------
StatusCode EventSaver::calculateJetMultiplicities(const xAOD::JetContainer& jets) {
  m_event_njets = jets.size();
  m_event_nbjets70 = 0;
  m_event_nbjets77 = 0;
  m_event_nbjets85 = 0;

  for (const auto& jet : jets) {
    if (!jet->isAvailable<char>("isbtagged_MV2c10_FixedCutBEff_70") and !jet->isAvailable<char>("isbtagged_DL1r_FixedCutBEff_70")) return StatusCode::FAILURE;
    if (!jet->isAvailable<char>("isbtagged_MV2c10_FixedCutBEff_77") and !jet->isAvailable<char>("isbtagged_DL1r_FixedCutBEff_77")) return StatusCode::FAILURE;
    if (!jet->isAvailable<char>("isbtagged_MV2c10_FixedCutBEff_85") and !jet->isAvailable<char>("isbtagged_DL1r_FixedCutBEff_85")) return StatusCode::FAILURE;

    if(jet->isAvailable<char>("isbtagged_MV2c10_FixedCutBEff_70") or jet->isAvailable<char>("isbtagged_MV2c10_FixedCutBEff_77") or jet->isAvailable<char>("isbtagged_MV2c10_FixedCutBEff_85")){
      m_event_nbjets70 += jet->auxdataConst<char>("isbtagged_MV2c10_FixedCutBEff_70");
      m_event_nbjets77 += jet->auxdataConst<char>("isbtagged_MV2c10_FixedCutBEff_77");
      m_event_nbjets85 += jet->auxdataConst<char>("isbtagged_MV2c10_FixedCutBEff_85");
    }

    if(jet->isAvailable<char>("isbtagged_DL1r_FixedCutBEff_70") or jet->isAvailable<char>("isbtagged_DL1r_FixedCutBEff_77") or jet->isAvailable<char>("isbtagged_DL1r_FixedCutBEff_85")){
      m_event_nbjets70 += jet->auxdataConst<char>("isbtagged_DL1r_FixedCutBEff_70");
      m_event_nbjets77 += jet->auxdataConst<char>("isbtagged_DL1r_FixedCutBEff_77");
      m_event_nbjets85 += jet->auxdataConst<char>("isbtagged_DL1r_FixedCutBEff_85");
    }

  }
  return StatusCode::SUCCESS;
}

// -----------------------------------------------
StatusCode EventSaver::calculateHT(const top::Event& event) {
  m_event_HT = 0;
  for (const auto& jet : event.m_jets) {
    m_event_HT += jet->pt();
  }
  return StatusCode::SUCCESS;
}

// -----------------------------------------------
StatusCode EventSaver::calculateST(const top::Event& event) {
  m_event_ST = 0;
  for (const auto& el : event.m_electrons) {
    m_event_ST += el->pt();
  }
  for (const auto& mu : event.m_muons) {
    m_event_ST += mu->pt();
  }
  for (const auto& jet : event.m_jets) {
    m_event_ST += jet->pt();
  }
  for (const auto& ph : event.m_photons) {
    m_event_ST += ph->pt();
  }
  m_event_ST += event.m_met->met();

  return StatusCode::SUCCESS;
}

// -----------------------------------------------
StatusCode EventSaver::calculateBjetVariables(const xAOD::JetContainer& jet_cont) {
  std::vector<float> bjet_scores;

  for (const auto& jet : jet_cont) {
    const auto& btag = jet->btagging();
    double mvx = -999;
    if (btag) btag->MVx_discriminant("MV2c10", mvx);
    bjet_scores.emplace_back(mvx);
  }
  m_jet_mv2c10_ranked_index.resize(bjet_scores.size());
  std::iota(m_jet_mv2c10_ranked_index.begin(), m_jet_mv2c10_ranked_index.end(), 0);
  std::sort(m_jet_mv2c10_ranked_index.begin(), m_jet_mv2c10_ranked_index.end(), [&bjet_scores] (int i1, int i2) { return bjet_scores[i1] > bjet_scores[i2]; });
  return StatusCode::SUCCESS;
}

// -----------------------------------------------
StatusCode EventSaver::calculateLeptonBjetVariables(const xAOD::JetContainer& jet_cont, const xAOD::ElectronContainer& el_cont, const xAOD::MuonContainer& mu_cont) {
  // Store vectors of xAOD::Jet objects that pass a certain
  // b-tagging working point.
  std::vector<const xAOD::Jet*> jets70;
  std::vector<const xAOD::Jet*> jets77;
  std::vector<const xAOD::Jet*> jets85;

  for (const auto& jet : jet_cont) {
    if (!jet->isAvailable<char>("isbtagged_MV2c10_FixedCutBEff_70") and !jet->isAvailable<char>("isbtagged_DL1r_FixedCutBEff_70")) return StatusCode::FAILURE;
    if (!jet->isAvailable<char>("isbtagged_MV2c10_FixedCutBEff_77") and !jet->isAvailable<char>("isbtagged_DL1r_FixedCutBEff_77")) return StatusCode::FAILURE;
    if (!jet->isAvailable<char>("isbtagged_MV2c10_FixedCutBEff_85") and !jet->isAvailable<char>("isbtagged_DL1r_FixedCutBEff_85")) return StatusCode::FAILURE;

    if(jet->isAvailable<char>("isbtagged_MV2c10_FixedCutBEff_70") or jet->isAvailable<char>("isbtagged_MV2c10_FixedCutBEff_77") or jet->isAvailable<char>("isbtagged_MV2c10_FixedCutBEff_85")){
      if (jet->auxdataConst<char>("isbtagged_MV2c10_FixedCutBEff_70") == 1) jets70.emplace_back(jet);
      if (jet->auxdataConst<char>("isbtagged_MV2c10_FixedCutBEff_77") == 1) jets77.emplace_back(jet);
      if (jet->auxdataConst<char>("isbtagged_MV2c10_FixedCutBEff_85") == 1) jets85.emplace_back(jet);
    }

    if(jet->isAvailable<char>("isbtagged_DL1r_FixedCutBEff_70") or jet->isAvailable<char>("isbtagged_DL1r_FixedCutBEff_77") or jet->isAvailable<char>("isbtagged_DL1r_FixedCutBEff_85")){
      if (jet->auxdataConst<char>("isbtagged_DL1r_FixedCutBEff_70") == 1) jets70.emplace_back(jet);
      if (jet->auxdataConst<char>("isbtagged_DL1r_FixedCutBEff_77") == 1) jets77.emplace_back(jet);
      if (jet->auxdataConst<char>("isbtagged_DL1r_FixedCutBEff_85") == 1) jets85.emplace_back(jet);
    }

  }
  const auto& n_el = el_cont.size();
  m_el_drlb70.resize(n_el);
  m_el_drlb77.resize(n_el);
  m_el_drlb85.resize(n_el);
  m_el_mlb70.resize(n_el);
  m_el_mlb77.resize(n_el);
  m_el_mlb85.resize(n_el);

  const auto& n_mu = mu_cont.size();
  m_mu_drlb70.resize(n_mu);
  m_mu_drlb77.resize(n_mu);
  m_mu_drlb85.resize(n_mu);
  m_mu_mlb70.resize(n_mu);
  m_mu_mlb77.resize(n_mu);
  m_mu_mlb85.resize(n_mu);

  // Fill all variables with -1. If no b-tagged jets are
  // available for a given working point, this will be the
  // fall-back value.
  std::fill(m_el_drlb70.begin(), m_el_drlb70.end(), -1);
  std::fill(m_el_drlb77.begin(), m_el_drlb77.end(), -1);
  std::fill(m_el_drlb85.begin(), m_el_drlb85.end(), -1);
  std::fill(m_el_mlb70.begin(), m_el_mlb70.end(), -1);
  std::fill(m_el_mlb77.begin(), m_el_mlb77.end(), -1);
  std::fill(m_el_mlb85.begin(), m_el_mlb85.end(), -1);
  std::fill(m_mu_drlb70.begin(), m_mu_drlb70.end(), -1);
  std::fill(m_mu_drlb77.begin(), m_mu_drlb77.end(), -1);
  std::fill(m_mu_drlb85.begin(), m_mu_drlb85.end(), -1);
  std::fill(m_mu_mlb70.begin(), m_mu_mlb70.end(), -1);
  std::fill(m_mu_mlb77.begin(), m_mu_mlb77.end(), -1);
  std::fill(m_mu_mlb85.begin(), m_mu_mlb85.end(), -1);

  // Loop over all electrons and store the variables.
  for (auto el_itr = el_cont.begin(); el_itr != el_cont.end(); ++el_itr) {
    const auto& i{el_itr - el_cont.begin()};
    const auto& electron{*el_itr};

    // Sort the jet containers according to dR(el, jet).
    auto dr_sorted = [&electron] (const xAOD::Jet* jet1, const xAOD::Jet* jet2) {
      return (electron->p4().DeltaR(jet1->p4()) < electron->p4().DeltaR(jet2->p4()));
    };

    if (!jets70.empty()) {
      std::sort(jets70.begin(), jets70.end(), dr_sorted);
      m_el_drlb70[i] = electron->p4().DeltaR(jets70[0]->p4());
      m_el_mlb70[i] = top::invariantMass(*el_cont[i], *jets70[0]);
    }

    if (!jets77.empty()) {
      std::sort(jets77.begin(), jets77.end(), dr_sorted);
      m_el_drlb77[i] = electron->p4().DeltaR(jets77[0]->p4());
      m_el_mlb77[i] = top::invariantMass(*el_cont[i], *jets77[0]);
    }

    if (!jets85.empty()) {
      std::sort(jets85.begin(), jets85.end(), dr_sorted);
      m_el_drlb85[i] = electron->p4().DeltaR(jets85[0]->p4());
      m_el_mlb85[i] = top::invariantMass(*el_cont[i], *jets85[0]);
    }
  }

  // Loop over all muons and store the variables.
  for (auto mu_itr = mu_cont.begin(); mu_itr != mu_cont.end(); ++mu_itr){
    const auto& i{mu_itr - mu_cont.begin()};
    const auto& muon{*mu_itr};

    // Sort the jet containers according to dR(mu, jet).
    auto dr_sorted = [&muon] (const xAOD::Jet* jet1, const xAOD::Jet* jet2) {
      return (muon->p4().DeltaR(jet1->p4()) < muon->p4().DeltaR(jet2->p4()));
    };

    if (!jets70.empty()) {
      std::sort(jets70.begin(), jets70.end(), dr_sorted);
      m_mu_drlb70[i] = muon->p4().DeltaR(jets70[0]->p4());
      m_mu_mlb70[i] = top::invariantMass(*mu_cont[i], *jets70[0]);
    }

    if (!jets77.empty()) {
      std::sort(jets77.begin(), jets77.end(), dr_sorted);
      m_mu_drlb77[i] = muon->p4().DeltaR(jets77[0]->p4());
      m_mu_mlb77[i] = top::invariantMass(*mu_cont[i], *jets77[0]);
    }

    if (!jets85.empty()) {
      std::sort(jets85.begin(), jets85.end(), dr_sorted);
      m_mu_drlb85[i] = muon->p4().DeltaR(jets85[0]->p4());
      m_mu_mlb85[i] = top::invariantMass(*mu_cont[i], *jets85[0]);
    }
  }

  return StatusCode::SUCCESS;
}

// -----------------------------------------------
StatusCode EventSaver::calculateDileptonVariables(const xAOD::ElectronContainer& el_cont, const xAOD::MuonContainer& mu_cont) {
  const auto& n_mu = mu_cont.size();
  const auto& n_el = el_cont.size();

  // Return if we don't have enough leptons.
  if (n_mu + n_el != 2) {
    m_event_dphill = -1;
    m_event_detall = -1;
    m_event_drll = -1;
    m_event_mll = -1;
    return StatusCode::SUCCESS;
  }

  if (n_mu == 2) {
    m_event_dphill = std::abs(top::deltaPhi(*mu_cont[0], *mu_cont[1]));
    m_event_detall = std::abs(mu_cont[0]->eta() - mu_cont[1]->eta());
    m_event_drll = mu_cont[0]->p4().DeltaR(mu_cont[1]->p4());
    m_event_mll = top::invariantMass(*mu_cont[0], *mu_cont[1]);
  } else if (n_el == 2) {
    m_event_dphill = std::abs(top::deltaPhi(*el_cont[0], *el_cont[1]));
    m_event_detall = std::abs(el_cont[0]->eta() - el_cont[1]->eta());
    m_event_drll = el_cont[0]->p4().DeltaR(el_cont[1]->p4());
    m_event_mll = top::invariantMass(*el_cont[0], *el_cont[1]);
  } else if (n_mu == 1 && n_el == 1) {
    m_event_dphill = std::abs(top::deltaPhi(*mu_cont[0], *el_cont[0]));
    m_event_detall = std::abs(mu_cont[0]->eta() - el_cont[0]->eta());
    m_event_drll = mu_cont[0]->p4().DeltaR(el_cont[0]->p4());
    m_event_mll = top::invariantMass(*mu_cont[0], *el_cont[0]);
  }
  return StatusCode::SUCCESS;
}

// -----------------------------------------------
StatusCode EventSaver::calculateMWT(const xAOD::ElectronContainer& el_cont, const xAOD::MuonContainer& mu_cont, const xAOD::MissingET& met) {
  const auto& n_mu = mu_cont.size();
  const auto& n_el = el_cont.size();

  // Return if we don't have exactly one lepton.
  if (n_mu + n_el != 1) {
    m_event_mwt = -1;
    return StatusCode::SUCCESS;
  }

  if (n_mu == 1) {
    m_event_mwt = top::mwt(*mu_cont[0], met);
  } else {
    m_event_mwt = top::mwt(*el_cont[0], met);
  }
  return StatusCode::SUCCESS;
}

// -----------------------------------------------
StatusCode EventSaver::calculatePhotonBjetVariables(const xAOD::JetContainer& jet_cont, const xAOD::PhotonContainer& ph_cont) {
  // Store vectors of xAOD::Jet objects that pass a certain
  // b-tagging working point.
  std::vector<const xAOD::Jet*> jets70;
  std::vector<const xAOD::Jet*> jets77;
  std::vector<const xAOD::Jet*> jets85;

  for (const auto& jet : jet_cont) {
    if (!jet->isAvailable<char>("isbtagged_MV2c10_FixedCutBEff_70") and !jet->isAvailable<char>("isbtagged_DL1r_FixedCutBEff_70")) return StatusCode::FAILURE;
    if (!jet->isAvailable<char>("isbtagged_MV2c10_FixedCutBEff_77") and !jet->isAvailable<char>("isbtagged_DL1r_FixedCutBEff_77")) return StatusCode::FAILURE;
    if (!jet->isAvailable<char>("isbtagged_MV2c10_FixedCutBEff_85") and !jet->isAvailable<char>("isbtagged_DL1r_FixedCutBEff_85")) return StatusCode::FAILURE;
    
    if(jet->isAvailable<char>("isbtagged_MV2c10_FixedCutBEff_70") or jet->isAvailable<char>("isbtagged_MV2c10_FixedCutBEff_77") or jet->isAvailable<char>("isbtagged_MV2c10_FixedCutBEff_85")){
      if (jet->auxdataConst<char>("isbtagged_MV2c10_FixedCutBEff_70") == 1) jets70.emplace_back(jet);
      if (jet->auxdataConst<char>("isbtagged_MV2c10_FixedCutBEff_77") == 1) jets77.emplace_back(jet);
      if (jet->auxdataConst<char>("isbtagged_MV2c10_FixedCutBEff_85") == 1) jets85.emplace_back(jet);
    }

    if(jet->isAvailable<char>("isbtagged_DL1r_FixedCutBEff_70") or jet->isAvailable<char>("isbtagged_DL1r_FixedCutBEff_77") or jet->isAvailable<char>("isbtagged_DL1r_FixedCutBEff_85")){
      if (jet->auxdataConst<char>("isbtagged_DL1r_FixedCutBEff_70") == 1) jets70.emplace_back(jet);
      if (jet->auxdataConst<char>("isbtagged_DL1r_FixedCutBEff_77") == 1) jets77.emplace_back(jet);
      if (jet->auxdataConst<char>("isbtagged_DL1r_FixedCutBEff_85") == 1) jets85.emplace_back(jet);
    }

  }

  // Fill all variables with -1. If no b-tagged jets are
  // available for a given working point, this will be the
  // fall-back value.
  std::fill(m_ph_drphb70_leading_pt.begin(), m_ph_drphb70_leading_pt.end(), -1);
  std::fill(m_ph_drphb77_leading_pt.begin(), m_ph_drphb77_leading_pt.end(), -1);
  std::fill(m_ph_drphb85_leading_pt.begin(), m_ph_drphb85_leading_pt.end(), -1);
  std::fill(m_ph_drphb70_subleading_pt.begin(), m_ph_drphb70_subleading_pt.end(), -1);
  std::fill(m_ph_drphb77_subleading_pt.begin(), m_ph_drphb77_subleading_pt.end(), -1);
  std::fill(m_ph_drphb85_subleading_pt.begin(), m_ph_drphb85_subleading_pt.end(), -1);
  std::fill(m_ph_drphb70_leading_score.begin(), m_ph_drphb70_leading_score.end(), -1);
  std::fill(m_ph_drphb77_leading_score.begin(), m_ph_drphb77_leading_score.end(), -1);
  std::fill(m_ph_drphb85_leading_score.begin(), m_ph_drphb85_leading_score.end(), -1);
  std::fill(m_ph_drphb70_subleading_score.begin(), m_ph_drphb70_subleading_score.end(), -1);
  std::fill(m_ph_drphb77_subleading_score.begin(), m_ph_drphb77_subleading_score.end(), -1);
  std::fill(m_ph_drphb85_subleading_score.begin(), m_ph_drphb85_subleading_score.end(), -1);
  std::fill(m_ph_drphb70_closest.begin(), m_ph_drphb70_closest.end(), -1);
  std::fill(m_ph_drphb77_closest.begin(), m_ph_drphb77_closest.end(), -1);
  std::fill(m_ph_drphb85_closest.begin(), m_ph_drphb85_closest.end(), -1);
  std::fill(m_ph_mphb70_closest.begin(),  m_ph_mphb70_closest.end(),  -1);
  std::fill(m_ph_mphb77_closest.begin(),  m_ph_mphb77_closest.end(),  -1);
  std::fill(m_ph_mphb85_closest.begin(),  m_ph_mphb85_closest.end(),  -1);


  // Loop over all photons and store the variables.
  for (auto ph_itr = ph_cont.begin(); ph_itr != ph_cont.end(); ++ph_itr) {
    const auto& i{ph_itr - ph_cont.begin()};
    const auto& photon{*ph_itr};

    // Sort the jet containers according to mv2c10 score.
    auto score_sorted = [] (const xAOD::Jet* jet1, const xAOD::Jet* jet2) {
      const auto& btag1 = jet1->btagging();
      double score1 = -999;
      if (btag1) btag1->MVx_discriminant("MV2c10", score1);
      const auto& btag2 = jet2->btagging();
      double score2 = -999;
      if (btag2) btag2->MVx_discriminant("MV2c10", score2);
      return score1 > score2;
    };

    // Sort the jet containers according to dR(ph, jet).
    auto dr_sorted = [&photon] (const xAOD::Jet* jet1, const xAOD::Jet* jet2) {
      return (photon->p4().DeltaR(jet1->p4()) < photon->p4().DeltaR(jet2->p4()));
    };

    if (!jets70.empty()) {
      std::sort(jets70.begin(), jets70.end(), top::descendingPtSorter);
      m_ph_drphb70_leading_pt[i] = photon->p4().DeltaR(jets70[0]->p4());
      if (jets70.size() > 1) m_ph_drphb70_subleading_pt[i] = photon->p4().DeltaR(jets70[1]->p4());

      std::sort(jets70.begin(), jets70.end(), score_sorted);
      m_ph_drphb70_leading_score[i] = photon->p4().DeltaR(jets70[0]->p4());
      if (jets70.size() > 1) m_ph_drphb70_subleading_score[i] = photon->p4().DeltaR(jets70[1]->p4());

      std::sort(jets70.begin(), jets70.end(), dr_sorted);
      m_ph_drphb70_closest[i] = photon->p4().DeltaR(jets70[0]->p4());
      m_ph_mphb70_closest[i] = top::invariantMass(*ph_cont[i], *jets70[0]);
    }

    if (!jets77.empty()) {
      std::sort(jets77.begin(), jets77.end(), top::descendingPtSorter);
      m_ph_drphb77_leading_pt[i] = photon->p4().DeltaR(jets77[0]->p4());
      if (jets77.size() > 1) m_ph_drphb77_subleading_pt[i] = photon->p4().DeltaR(jets77[1]->p4());

      std::sort(jets77.begin(), jets77.end(), score_sorted);
      m_ph_drphb77_leading_score[i] = photon->p4().DeltaR(jets77[0]->p4());
      if (jets77.size() > 1) m_ph_drphb77_subleading_score[i] = photon->p4().DeltaR(jets77[1]->p4());

      std::sort(jets77.begin(), jets77.end(), dr_sorted);
      m_ph_drphb77_closest[i] = photon->p4().DeltaR(jets77[0]->p4());
      m_ph_mphb77_closest[i] = top::invariantMass(*ph_cont[i], *jets77[0]);
    }

    if (!jets85.empty()) {
      std::sort(jets85.begin(), jets85.end(), top::descendingPtSorter);
      m_ph_drphb85_leading_pt[i] = photon->p4().DeltaR(jets85[0]->p4());
      if (jets85.size() > 1) m_ph_drphb85_subleading_pt[i] = photon->p4().DeltaR(jets85[1]->p4());

      std::sort(jets85.begin(), jets85.end(), score_sorted);
      m_ph_drphb85_leading_score[i] = photon->p4().DeltaR(jets85[0]->p4());
      if (jets85.size() > 1) m_ph_drphb85_subleading_score[i] = photon->p4().DeltaR(jets85[1]->p4());

      std::sort(jets85.begin(), jets85.end(), dr_sorted);
      m_ph_drphb85_closest[i] = photon->p4().DeltaR(jets85[0]->p4());
      m_ph_mphb85_closest[i] = top::invariantMass(*ph_cont[i], *jets85[0]);
    }
  }

  return StatusCode::SUCCESS;
}

// -----------------------------------------------
StatusCode EventSaver::calculatePhotonLeptonVariables(const top::Event& event) {
  // Store all lepton pointers in a common container.
  std::vector<const xAOD::IParticle*> leptons;
  for (const auto& p : event.m_muons) {
    leptons.emplace_back(p);
  }
  for (const auto& p : event.m_electrons) {
    leptons.emplace_back(p);
  }

  // Loop over all photons and store the variables.
  for (auto ph_itr = event.m_photons.begin(); ph_itr != event.m_photons.end(); ++ph_itr) {
    const auto& i{ph_itr - event.m_photons.begin()};
    const auto& photon{*ph_itr};

    m_ph_drmuph[i] = -1;
    m_ph_drelph[i] = -1;
    m_ph_drlph_leading[i] = -1;
    m_ph_drlph_subleading[i] = -1;
    m_ph_drlph_closest[i] = -1;
    m_ph_mlph_closest[i] = -1;
    m_ph_mllph[i] = -1;

    if (leptons.empty()) return StatusCode::SUCCESS;

    if (event.m_muons.size() == 1) {
      m_ph_drmuph[i] = photon->p4().DeltaR(event.m_muons[0]->p4());
    }
    if (event.m_electrons.size() == 1) {
      m_ph_drelph[i] = photon->p4().DeltaR(event.m_electrons[0]->p4());
    }

    // Sort leptons according to pT and store those variables
    // first, then resort according to dR to the photon.
    std::sort(leptons.begin(), leptons.end(), top::descendingPtSorter);
    if (leptons.size() > 0) m_ph_drlph_leading[i] = photon->p4().DeltaR(leptons[0]->p4());
    if (leptons.size() > 1) m_ph_drlph_subleading[i] = photon->p4().DeltaR(leptons[1]->p4());

    // Sort the lepton container according to dR(lep, ph).
    auto dr_sorted = [&photon] (const xAOD::IParticle* p1, const xAOD::IParticle* p2) {
      return (photon->p4().DeltaR(p1->p4()) < photon->p4().DeltaR(p2->p4()));
    };
    std::sort(leptons.begin(), leptons.end(), dr_sorted);

    m_ph_drlph_closest[i] = photon->p4().DeltaR(leptons[0]->p4());
    m_ph_mlph_closest[i] = (photon->p4() + leptons[0]->p4()).M();

    if (leptons.size() == 2) {
      m_ph_mllph[i] = (photon->p4() + leptons[0]->p4() + leptons[1]->p4()).M();
    }
  }

  return StatusCode::SUCCESS;
}

// -----------------------------------------------
StatusCode EventSaver::storeGoodPhotonIndices(const xAOD::PhotonContainer& photons) {
  // Put all "good" photons in a list.
  std::vector<const xAOD::Photon*> good_photons;
  for (const auto& photon : photons) {
    if (qualifiesAsGood(*photon)) good_photons.emplace_back(photon);
  }

  std::sort(good_photons.begin(), good_photons.end(), top::descendingPtSorter);

  // Store the indices of the photons in the list.
  m_ph_good_index.resize(good_photons.size());
  for (size_t i = 0; i < m_ph_good_index.size(); ++i) {
    const auto& photon = good_photons[i];
    m_ph_good_index.at(i) = photon->auxdataConst<int>("i");
  }

  return StatusCode::SUCCESS;
}

// -----------------------------------------------
StatusCode EventSaver::storeLeptonPDGID(const asg::SgTEvent& store, std::vector<float>* var) {
  const xAOD::PartonHistory* parton_hist{nullptr};
  const xAOD::PartonHistoryContainer* ph_container{nullptr};
  if (store.contains<xAOD::PartonHistoryContainer>("TopPartonHistory")) {
    top::check(store.retrieve(ph_container, "TopPartonHistory"),
               "Failed to retrieve Top Parton History");
    if (ph_container->size() == 1) {
      parton_hist = ph_container->at(0);
    }
  }

  if (!parton_hist) return StatusCode::FAILURE;
  if (var->size() < 2) return StatusCode::FAILURE;

  if (parton_hist != nullptr) {
    if (parton_hist->isAvailable<int>("MC_Wdecay1_from_t_pdgId")) {
      if (std::abs(parton_hist->auxdataConst<int>("MC_Wdecay1_from_t_pdgId")) == 11) var->at(0) = 11;
      if (std::abs(parton_hist->auxdataConst<int>("MC_Wdecay1_from_t_pdgId")) == 13) var->at(0) = 13;
      if (std::abs(parton_hist->auxdataConst<int>("MC_Wdecay1_from_t_pdgId")) == 15) var->at(0) = 15;
    }
    if (parton_hist->isAvailable<int>("MC_Wdecay1_from_tbar_pdgId")) {
      if (std::abs(parton_hist->auxdataConst<int>("MC_Wdecay1_from_tbar_pdgId")) == 11) var->at(1) = 11;
      if (std::abs(parton_hist->auxdataConst<int>("MC_Wdecay1_from_tbar_pdgId")) == 13) var->at(1) = 13;
      if (std::abs(parton_hist->auxdataConst<int>("MC_Wdecay1_from_tbar_pdgId")) == 15) var->at(1) = 15;
    }
    if (parton_hist->isAvailable<int>("MC_Wdecay2_from_t_pdgId")) {
      if (std::abs(parton_hist->auxdataConst<int>("MC_Wdecay2_from_t_pdgId")) == 11) var->at(0) = 11;
      if (std::abs(parton_hist->auxdataConst<int>("MC_Wdecay2_from_t_pdgId")) == 13) var->at(0) = 13;
      if (std::abs(parton_hist->auxdataConst<int>("MC_Wdecay2_from_t_pdgId")) == 15) var->at(0) = 15;
    }
    if (parton_hist->isAvailable<int>("MC_Wdecay2_from_tbar_pdgId")) {
      if (std::abs(parton_hist->auxdataConst<int>("MC_Wdecay2_from_tbar_pdgId")) == 11) var->at(1) = 11;
      if (std::abs(parton_hist->auxdataConst<int>("MC_Wdecay2_from_tbar_pdgId")) == 13) var->at(1) = 13;
      if (std::abs(parton_hist->auxdataConst<int>("MC_Wdecay2_from_tbar_pdgId")) == 15) var->at(1) = 15;
    }
  }

  return StatusCode::SUCCESS;
}

// -----------------------------------------------
StatusCode EventSaver::storeOverlapRemovalInfo(const top::Event& event) {
  bool in_overlap;
  if (topConfig()->isMC()) {
    top::check(m_vgamma_tool->inOverlap(in_overlap), "Could not execute VGammaORTool::inOverlap(bool)");
  }
  m_event_in_overlap = static_cast<char>(in_overlap);

  SampleOverlapRemover or_remover(event);
  m_event_in_overlap_TTH = static_cast<char>(or_remover.inOverlap_TTH(15000., 0.2));

  // The rel. 20.7 method relies on a reconstructed "good"
  // photon, i.e. we need to make sure we have one before
  // attempting to store anything.
  if (!m_ph_good_index.empty()) {
    const auto& good_photon = event.m_photons.at(m_ph_good_index.at(0));
    m_event_in_overlap_REL20 = static_cast<char>(or_remover.inOverlap_REL20(good_photon));
  } else if (!event.m_photons.empty()) {
    const auto& photon = event.m_photons.at(0);
    m_event_in_overlap_REL20 = static_cast<char>(or_remover.inOverlap_REL20(photon));
  } else {
    m_event_in_overlap_REL20 = static_cast<char>(false);
  }

  return StatusCode::SUCCESS;
}

// -----------------------------------------------
StatusCode EventSaver::storePhotonIdentificationVariables(const xAOD::PhotonContainer& photons) {
  // To define a slightly relaxed tight photon ID menu, we don't
  // require the photon to pass _all_ calo-strips criteria. For
  // that, define a subset of calo-strips cuts (based on the
  // definition in egammaPID::CALOSTRIPS_PHOTONTIGHT).
  const unsigned int CALOSTRIPS_PHOTONTIGHT_relaxed =
    0x1 << egammaPID::ClusterStripsEratio_Photon     |
    0x1 << egammaPID::ClusterStripsDeltaEmax2_Photon |
    0x1 << egammaPID::ClusterStripsWtot_Photon       ;

  // Then, based on that, define the relaxed tight ID.
  const unsigned int PhotonTight_relaxed =
    egammaPID::HADLEAKETA_PHOTON |
    egammaPID::CALOMIDDLE_PHOTON |
    CALOSTRIPS_PHOTONTIGHT_relaxed;

  // And pick up all tight ID criteria that do _not_ fall in the
  // relaxed-tight ID defined above (with bitweise XOR).
  const unsigned int PhotonTight_not_in_relaxed = egammaPID::PhotonTight ^ PhotonTight_relaxed;

  for (auto ph_itr = photons.begin(); ph_itr != photons.end(); ++ph_itr) {
    const auto& i{ph_itr - photons.begin()};
    const auto& photon{*ph_itr};

    if (!photon->isAvailable<char>("DFCommonPhotonsIsEMLoose")) return StatusCode::FAILURE;
    if (!photon->isAvailable<char>("DFCommonPhotonsIsEMTight")) return StatusCode::FAILURE;
    if (!photon->isAvailable<unsigned int>("DFCommonPhotonsIsEMTightIsEMValue")) return StatusCode::FAILURE;

    m_ph_id_loose[i] = photon->auxdataConst<char>("DFCommonPhotonsIsEMLoose");
    m_ph_id_tight[i] = photon->auxdataConst<char>("DFCommonPhotonsIsEMTight");

    // Retrieve the bit word from the tight selector.
    const auto& bit_word = photon->auxdataConst<unsigned int>("DFCommonPhotonsIsEMTightIsEMValue");

    // Test whether the photon passed the relaxed tight criteria
    // and whether it failed on ore multiple calo-strips cuts.
    m_ph_id_tight_relaxed[i] = m_ph_id_loose[i] && (!(bit_word & PhotonTight_relaxed));
    m_ph_id_fail_deltaE[i] = static_cast<bool>(bit_word & (0x1 << egammaPID::ClusterStripsDeltaE_Photon));
    m_ph_id_fail_fside[i] = static_cast<bool>(bit_word & (0x1 << egammaPID::ClusterStripsFracm_Photon));
    m_ph_id_fail_ws3[i] = static_cast<bool>(bit_word & (0x1 << egammaPID::ClusterStripsWeta1c_Photon));
    m_ph_id_fail_Eratio[i] = static_cast<bool>(bit_word & (0x1 << egammaPID::ClusterStripsDEmaxs1_Photon));
    m_ph_id_fail_count[i] = std::bitset<32>(bit_word & PhotonTight_not_in_relaxed).count();
  }

  return StatusCode::SUCCESS;
}

// -----------------------------------------------
StatusCode EventSaver::storePhotonIsolationVariables(const xAOD::PhotonContainer& photons) {
  for (auto ph_itr = photons.begin(); ph_itr != photons.end(); ++ph_itr) {
    const auto& i{ph_itr - photons.begin()};
    const auto& photon{*ph_itr};

    // Deactivated as of version > 2.0, to save disk space. These
    // variables are not stored in TOPQ1 derivations anyways.
    // Reactivate if needed.
    // photon->isolationValue(m_ph_iso_ptcone30[i],     xAOD::Iso::ptcone30 );
    // photon->isolationValue(m_ph_iso_ptcone40[i],     xAOD::Iso::ptcone40 );
    // photon->isolationValue(m_ph_iso_ptvarcone20[i],  xAOD::Iso::ptvarcone20 );
    // photon->isolationValue(m_ph_iso_ptvarcone30[i],  xAOD::Iso::ptvarcone30 );
    // photon->isolationValue(m_ph_iso_ptvarcone40[i],  xAOD::Iso::ptvarcone40 );
    // photon->isolationValue(m_ph_iso_topoetcone30[i], xAOD::Iso::topoetcone30 );

    photon->isolationValue(m_ph_iso_ptcone20[i],     xAOD::Iso::ptcone20 );
    photon->isolationValue(m_ph_iso_topoetcone20[i], xAOD::Iso::topoetcone20);
    photon->isolationValue(m_ph_iso_topoetcone40[i], xAOD::Iso::topoetcone40 );
    m_ph_iso_FCL[i] =   photon->auxdataConst<char>("AnalysisTop_Isol_FixedCutLoose");
    m_ph_iso_FCT[i] =   photon->auxdataConst<char>("AnalysisTop_Isol_FixedCutTight");
    m_ph_iso_FCTCO[i] = photon->auxdataConst<char>("AnalysisTop_Isol_FixedCutTightCaloOnly");
  }

  return StatusCode::SUCCESS;
}

// -----------------------------------------------
StatusCode EventSaver::storePhotonShowerShapes(const xAOD::PhotonContainer& photons) {
  for (auto ph_itr = photons.begin(); ph_itr != photons.end(); ++ph_itr) {
    const auto& i{ph_itr - photons.begin()};
    const auto& photon{*ph_itr};
    photon->showerShapeValue(m_ph_ss_deltaE[i], xAOD::EgammaParameters::DeltaE);
    photon->showerShapeValue(m_ph_ss_e277[i],   xAOD::EgammaParameters::e277);
    photon->showerShapeValue(m_ph_ss_emaxs1[i], xAOD::EgammaParameters::emaxs1);
    photon->showerShapeValue(m_ph_ss_eratio[i], xAOD::EgammaParameters::Eratio);
    photon->showerShapeValue(m_ph_ss_f1[i],     xAOD::EgammaParameters::f1);
    photon->showerShapeValue(m_ph_ss_fracs1[i], xAOD::EgammaParameters::fracs1);
    photon->showerShapeValue(m_ph_ss_reta[i],   xAOD::EgammaParameters::Reta);
    photon->showerShapeValue(m_ph_ss_rhad1[i],  xAOD::EgammaParameters::Rhad1);
    photon->showerShapeValue(m_ph_ss_rhad[i],   xAOD::EgammaParameters::Rhad);
    photon->showerShapeValue(m_ph_ss_rphi[i],   xAOD::EgammaParameters::Rphi);
    photon->showerShapeValue(m_ph_ss_weta1[i],  xAOD::EgammaParameters::weta1);
    photon->showerShapeValue(m_ph_ss_weta2[i],  xAOD::EgammaParameters::weta2);
    photon->showerShapeValue(m_ph_ss_wtots1[i], xAOD::EgammaParameters::wtots1);
  }

  return StatusCode::SUCCESS;
}

// -----------------------------------------------
StatusCode EventSaver::storePhotonTruthInfo(const xAOD::PhotonContainer& photons, const std::vector<const xAOD::TruthParticle*>& truth_electrons) {
  for (auto ph_itr = photons.begin(); ph_itr != photons.end(); ++ph_itr) {
    const auto& i{ph_itr - photons.begin()};
    const auto& photon{*ph_itr};

    PhotonTruthInfo info{*photon, truth_electrons};
    m_ph_true_topparent[i] = info.topparent;
    m_ph_true_parent[i]    = info.parent;
    m_ph_true_barcode[i]   = info.barcode;
    m_ph_true_pdgid[i]     = info.pdgid;
    m_ph_true_eta[i]       = info.four_mom.Eta();
    m_ph_true_phi[i]       = info.four_mom.Phi();
    m_ph_true_pt[i]        = info.four_mom.Pt();
    m_ph_true_origin[i]    = info.origin;
    m_ph_true_type[i]      = info.type;
    m_ph_true_category[i]  = info.category;
  }

  return StatusCode::SUCCESS;
}
}  // namespace ttgamma
}  // namespace top
