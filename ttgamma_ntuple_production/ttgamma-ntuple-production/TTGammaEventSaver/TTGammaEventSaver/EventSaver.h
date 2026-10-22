#ifndef TTGAMMAEVENTSAVER_EVENTSAVER_H_
#define TTGAMMAEVENTSAVER_EVENTSAVER_H_

#include <string>
#include <vector>

#include <AsgTools/AnaToolHandle.h>             ///< To define the ASG analysis tool handle.
#include "GammaORTools/IVGammaORTool.h"         ///< For the sample stitching
#include "TopAnalysis/EventSaverFlatNtuple.h"   ///< The base class of this class.
#include "TTGammaEventSaver/PhotonTruthInfo.h"  ///< To retrieve truth info about photons.
#include "xAODMissingET/MissingET.h"            ///< To work with missing ET.
#include "PhotonVertexSelection/IPhotonPointingTool.h"
#include "PhotonVertexSelection/PhotonPointingTool.h"

namespace top {
class Event;
class TopConfig;

namespace ttgamma {
  /**
   * Custom event saver class which adds ttgamma-specific
   * variables. The class is based on EventSaverFlatNtuple() and
   * also calls its functionalities to do the main work.
  */
class EventSaver : public top::EventSaverFlatNtuple {
 public:
  /// Default class constructor. Use initialiser list to init
  /// some of the member variables.
  EventSaver();

  /// Defaulted destructor.
  ~EventSaver();

  /// Explicitly deleted copy constructors.
  EventSaver(EventSaver&& svr) = delete;
  EventSaver(const EventSaver& svr) = delete;

  /// Explicitly deleted assignment operator.
  EventSaver& operator=(const EventSaver& svr) = delete;

  /**
   * Initialise the event saving. This adds the additional
   * variables to all trees.
   * @param config Shared pointer to the config object holding
   * the data read in from the steering file.
   * @param file The output ROOT file.
   * @param extraBranches A vector of strings for extra branches.
   * This gets forwarded to the main event saver and is not used
   * here.
   */
  void initialize(std::shared_ptr<top::TopConfig> config, TFile* file, const std::vector<std::string>& extraBranches) override;

  /// Dummy initialise() function, as this is an ASG tool.
  StatusCode initialize() override { return StatusCode::SUCCESS; }

  /**
   * Function that tests whether an xAOD::Photon object qualifies
   * as a "good" photon. Those criteria include:
   *   1) Minimum p_T of 20 GeV
   *   2) Passing the tight ID requirements
   *
   * @param ph The reference to the photon object.
   * @return A boolean whether the photon is good
   */
  bool qualifiesAsGood(const xAOD::Photon& photon);

  /**
   * Function to resize all internal vectors for photon variables
   * to the correct length. This is necessary because the number
   * of photons varies per event.
   * @param event The reference to the event object.
   */
  void resizePhotonVectors(const top::Event& event);

  /**
   * The main function executed for every event. Here are all the
   * modifications for this custom event saver.
   * EventSaverFlatNtuple::saveEvent() gets called at the very
   * end from this method.
   * @param event The reference to the event object.
   */
  void saveEvent(const top::Event& event) override;

  /**
   * The main function executed for every truth event. Here are
   * all the modifications for the truth tree.
   */
  virtual void saveTruthEvent() override;

  /// \name Helper functions to slim down the saveEvent() function
  /// @{

  /**
   * Calculate the jet multiplicities of the event. This
   * calculates the number of jets, and the number of b-tagged
   * jets for three working points (70%, 77%, 85%).
   * @param jets Constant reference to the jet container.
   * @param njets Pointer to the njets variable to be filed.
   * @param nbjets_70 Pointer to the nbjets variable to be filed.
   * @param nbjets_77 Pointer to the nbjets variable to be filed.
   * @param nbjets_85 Pointer to the nbjets variable to be filed.
   * @return A status code.
   */
  StatusCode calculateJetMultiplicities(const xAOD::JetContainer& jets);

  /**
   * Calculate the H_T of the event (scalar sum of all hadronic
   * transverse momenta, i.e. momenta of all jets).
   * @param event Constant reference to the event.
   * @return A status code.
   */
  StatusCode calculateHT(const top::Event& event);

  /**
   * Calculate the S_T of the event (scalar sum over *all*
   * transverse momenta, that is: transverse momenta of leptons,
   * jets, photons, plus MET).
   * @param event Constant reference to the event.
   * @return A status code.
   */
  StatusCode calculateST(const top::Event& event);

  /**
   * Calculate dilepton variables, such as the invariant mass of
   * the two leptons of the event (if the event has more than one
   * lepton).
   * @param el_cont Constant reference to the electron container.
   * @param mu_cont Constant reference to the muon container.
   * @param mll Pointer to the mll variable to be filled.
   * @return A status code.
   */
  StatusCode calculateDileptonVariables(const xAOD::ElectronContainer& el_cont, const xAOD::MuonContainer& mu_cont);

  /**
   * Calculate variables between the photon(s) and their closest
   * bjet. This currently includes dR and invariant masses.
   * @param jet_cont Constant reference to the jet container.
   * @param ph_cont Constant reference to the photon container.
   * @return A status code.
   */
  StatusCode calculatePhotonBjetVariables(const xAOD::JetContainer& jet_cont, const xAOD::PhotonContainer& ph_cont);

  /**
   * Calculate variables between the photons and the lepton(s).
   * This includes dR and invariant masses.
   * @param event Constant reference to the event.
   * @return A status code.
   */
  StatusCode calculatePhotonLeptonVariables(const top::Event& event);

  /**
   * Calculate variables between the lepton(s) and their closest
   * bjet. This currently includes dR and invariant masses.
   * @param jet_cont Constant reference to the jet container.
   * @param el_cont Constant reference to the electron container
   * @param mu_cont Constant reference to the muon container
   * @return A status code.
   */
  StatusCode calculateLeptonBjetVariables(const xAOD::JetContainer& jet_cont, const xAOD::ElectronContainer& el_cont, const xAOD::MuonContainer& mu_cont);

  /**
   * Calculate a few b-tagging related variables, such as a vector of jet
   * indices, where the jets are ranked according to their mv2c10 score.
   * @param jet_cont Constant reference to the jet container.
   * @return A status code.
   */
  StatusCode calculateBjetVariables(const xAOD::JetContainer& jet_cont);

  /**
   * Calculate the transverse mass of the W boson in the
   * single-lepton channels.
   * @param el_cont Constant reference to the electron container.
   * @param mu_cont Constant reference to the muon container.
   * @param met Constant reference to the Missing ET object.
   * @param mll Pointer to the mll variable to be filled.
   * @return A status code.
   */
  StatusCode calculateMWT(const xAOD::ElectronContainer& el_cont, const xAOD::MuonContainer& mu_cont, const xAOD::MissingET& met);

  /**
   * Categorize photon into one of the categories in the
   * PhotonCategory enum.
   * @param photon_index Index of the photon.
   * @return The event category
   */
  PhotonCategory categorizePhoton(size_t photon_index);

  /**
   * Store the indices of all "good" photons, that is, photon
   * passing the qualifiesAsGood() method.
   * @param photons Reference to the photon container
   * @return A status code
   */
  StatusCode storeGoodPhotonIndices(const xAOD::PhotonContainer& photons);

  /**
   * Check whether the W decay particles are leptonic, and store
   * their PDG IDs into a given vector (if leptonic). This
   * function is not being used as of version > 2.0 to save
   * ntuple disk space. Without this function, we can entirely
   * deactivate the TopPartonHistory package (which saves us a
   * lot of space). Reactivate if needed.
   * @param store Constant reference to the event store (for
   *    retrieving the parton history object).
   * @param var Pointer to vector in which variables are stored.
   * @return A status code.
   */
  StatusCode storeLeptonPDGID(const asg::SgTEvent& store, std::vector<float>* var);

  /**
   * Store information about the sample overlap removal procedures.
   * @param event Const reference to the event
   * @return A status code
   */
  StatusCode storeOverlapRemovalInfo(const top::Event& event);

  /**
   * Store identification variables of the photons.
   * @param photons Reference to the photon container
   * @return A status code
   */
  StatusCode storePhotonIdentificationVariables(const xAOD::PhotonContainer& photons);

  /**
   * Store isolation variables of the photons.
   * @param photons Reference to the photon container
   * @return A status code
   */
  StatusCode storePhotonIsolationVariables(const xAOD::PhotonContainer& photons);

  /**
   * Store shower shape variables of the photons.
   * @param photons Reference to the photon container
   * @return A status code
   */
  StatusCode storePhotonShowerShapes(const xAOD::PhotonContainer& photons);

  /**
   * Store truth information about the photons.
   * @param photons Reference to the photon container
   * @return A status code
   */
  StatusCode storePhotonTruthInfo(const xAOD::PhotonContainer& photons, const std::vector<const xAOD::TruthParticle*>& truth_electrons);

  /// @}

 private:
  /// Analysis tool handle for the sample stitching/overlap removal.
  asg::AnaToolHandle<IVGammaORTool> m_vgamma_tool;

  /// Analysis tool handle for the PhotonPointTool (to get its origin).
  asg::AnaToolHandle<CP::IPhotonPointingTool> m_photonpointing_tool;

  /// Variables stored in the truth record (parton-level info).
  char m_MC_tty_event_in_overlap;      ///< Boolean whether event is in overlap region (using the VGammaORTool).
  std::vector<float> m_MC_tty_photon_pts_overlap;    ///< pT of photons outside sample-overlap region

  float m_MC_tty_ph_pt_before;
  float m_MC_tty_ph_pt_after1st;
  float m_MC_tty_ph_pt_afterall;
  float m_MC_tty_ph_eta_before;
  float m_MC_tty_ph_eta_after1st;
  float m_MC_tty_ph_eta_afterall;
  float m_MC_tty_ph_phi_before;
  float m_MC_tty_ph_phi_after1st;
  float m_MC_tty_ph_phi_afterall;
  char m_MC_tty_ph_isIso_afterall;
  float m_MC_tty_Wplus_decay1_pt_before;
  float m_MC_tty_Wplus_decay1_pt_after1st;
  float m_MC_tty_Wplus_decay1_pt_afterall;
  float m_MC_tty_Wplus_decay1_eta_before;
  float m_MC_tty_Wplus_decay1_eta_after1st;
  float m_MC_tty_Wplus_decay1_eta_afterall;
  float m_MC_tty_Wplus_decay1_phi_before;
  float m_MC_tty_Wplus_decay1_phi_after1st;
  float m_MC_tty_Wplus_decay1_phi_afterall;
  float m_MC_tty_Wplus_decay2_pt_before;
  float m_MC_tty_Wplus_decay2_pt_after1st;
  float m_MC_tty_Wplus_decay2_pt_afterall;
  float m_MC_tty_Wplus_decay2_eta_before;
  float m_MC_tty_Wplus_decay2_eta_after1st;
  float m_MC_tty_Wplus_decay2_eta_afterall;
  float m_MC_tty_Wplus_decay2_phi_before;
  float m_MC_tty_Wplus_decay2_phi_after1st;
  float m_MC_tty_Wplus_decay2_phi_afterall;
  float m_MC_tty_Wminus_decay1_pt_before;
  float m_MC_tty_Wminus_decay1_pt_after1st;
  float m_MC_tty_Wminus_decay1_pt_afterall;
  float m_MC_tty_Wminus_decay1_eta_before;
  float m_MC_tty_Wminus_decay1_eta_after1st;
  float m_MC_tty_Wminus_decay1_eta_afterall;
  float m_MC_tty_Wminus_decay1_phi_before;
  float m_MC_tty_Wminus_decay1_phi_after1st;
  float m_MC_tty_Wminus_decay1_phi_afterall;
  float m_MC_tty_Wminus_decay2_pt_before;
  float m_MC_tty_Wminus_decay2_pt_after1st;
  float m_MC_tty_Wminus_decay2_pt_afterall;
  float m_MC_tty_Wminus_decay2_eta_before;
  float m_MC_tty_Wminus_decay2_eta_after1st;
  float m_MC_tty_Wminus_decay2_eta_afterall;
  float m_MC_tty_Wminus_decay2_phi_before;
  float m_MC_tty_Wminus_decay2_phi_after1st;
  float m_MC_tty_Wminus_decay2_phi_afterall;
  float m_MC_tty_bottom_pt_before;
  float m_MC_tty_bottom_pt_after1st;
  float m_MC_tty_bottom_pt_partonjet;
  float m_MC_tty_bottom_pt_rbf_had;
  float m_MC_tty_bottom_eta_before;
  float m_MC_tty_bottom_eta_after1st;
  float m_MC_tty_bottom_eta_partonjet;
  float m_MC_tty_bottom_eta_rbf_had;
  float m_MC_tty_bottom_phi_before;
  float m_MC_tty_bottom_phi_after1st;
  float m_MC_tty_bottom_phi_partonjet;
  float m_MC_tty_bottom_phi_rbf_had;
  float m_MC_tty_bottom_m_partonjet;
  float m_MC_tty_bottom_m_rbf_had;
  float m_MC_tty_antibottom_pt_before;
  float m_MC_tty_antibottom_pt_after1st;
  float m_MC_tty_antibottom_pt_partonjet;
  float m_MC_tty_antibottom_pt_rbf_had;
  float m_MC_tty_antibottom_eta_before;
  float m_MC_tty_antibottom_eta_after1st;
  float m_MC_tty_antibottom_eta_partonjet;
  float m_MC_tty_antibottom_eta_rbf_had;
  float m_MC_tty_antibottom_phi_before;
  float m_MC_tty_antibottom_phi_after1st;
  float m_MC_tty_antibottom_phi_partonjet;
  float m_MC_tty_antibottom_phi_rbf_had;
  float m_MC_tty_antibottom_m_partonjet;
  float m_MC_tty_antibottom_m_rbf_had;
  std::vector<int> m_MC_tty_partonjet_pdgIds;
  std::vector<int> m_MC_tty_partonjet_status;
  float m_MC_tty_partonjet_avpt;
  int m_MC_tty_Wplus_decay1_pdgId;
  int m_MC_tty_Wplus_decay2_pdgId;
  int m_MC_tty_Wminus_decay1_pdgId;
  int m_MC_tty_Wminus_decay2_pdgId;

  char m_MC_tty_is_dilepton;            ///< Boolean whether we're looking at a dilepton event on parton level.
  char m_MC_tty_is_emu_channel;         ///< Boolean whether we're looking at e+mu channel on parton level.

  /// General event information variables.
  float m_event_dphill;                 ///< Modulus of dPhi of the two leptons between [0, pi] (if available).
  float m_event_detall;                 ///< Modulus of dEta of the two leptons (if available).
  float m_event_drll;                   ///< dR of the two leptons (if available).
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
  std::vector<int> m_ph_conversion_type;     ///< Defined in xAODEgamma/EgammaEnums.h.
  std::vector<float> m_ph_drmuph;            ///< dR between photon and muon (if available).
  std::vector<float> m_ph_drelph;            ///< dR between photon and electron (if available).
  std::vector<float> m_ph_drlph_leading;     ///< dR between photon and leading lepton.
  std::vector<float> m_ph_drlph_subleading;  ///< dR between photon and sub-leading lepton (if available).
  std::vector<float> m_ph_drlph_closest;     ///< dR between photon and closest lepton.
  std::vector<float> m_ph_mlph_closest;      ///< Invariant mass of photon and closest lepton.
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
  //std::vector<float> m_ph_iso_ptcone40;
  //std::vector<float> m_ph_iso_ptvarcone20;
  //std::vector<float> m_ph_iso_ptvarcone40;
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
  std::vector<float> m_ph_drphb70_leading_pt;        ///< dR between photon and leading bjet (in pT) (70% WP).
  std::vector<float> m_ph_drphb77_leading_pt;        ///< dR between photon and leading bjet (in pT) (77% WP).
  std::vector<float> m_ph_drphb85_leading_pt;        ///< dR between photon and leading bjet (in pT) (85% WP).
  std::vector<float> m_ph_drphb70_subleading_pt;     ///< dR between photon and subleading bjet (in pT) (70% WP).
  std::vector<float> m_ph_drphb77_subleading_pt;     ///< dR between photon and subleading bjet (in pT) (77% WP).
  std::vector<float> m_ph_drphb85_subleading_pt;     ///< dR between photon and subleading bjet (in pT) (85% WP).
  std::vector<float> m_ph_drphb70_leading_score;     ///< dR between photon and leading bjet (in score) (70% WP).
  std::vector<float> m_ph_drphb77_leading_score;     ///< dR between photon and leading bjet (in score) (77% WP).
  std::vector<float> m_ph_drphb85_leading_score;     ///< dR between photon and leading bjet (in score) (85% WP).
  std::vector<float> m_ph_drphb70_subleading_score;  ///< dR between photon and subleading bjet (in score) (70% WP).
  std::vector<float> m_ph_drphb77_subleading_score;  ///< dR between photon and subleading bjet (in score) (77% WP).
  std::vector<float> m_ph_drphb85_subleading_score;  ///< dR between photon and subleading bjet (in score) (85% WP).
  std::vector<float> m_ph_drphb70_closest;      ///< dR between photon and closest bjet (70% WP).
  std::vector<float> m_ph_drphb77_closest;      ///< dR between photon and closest bjet (77% WP).
  std::vector<float> m_ph_drphb85_closest;      ///< dR between photon and closest bjet (85% WP).
  std::vector<float> m_ph_mphb70_closest;       ///< Invariant mass between photon and closest bjet (70% WP).
  std::vector<float> m_ph_mphb77_closest;       ///< Invariant mass between photon and closest bjet (77% WP).
  std::vector<float> m_ph_mphb85_closest;       ///< Invariant mass between photon and closest bjet (85% WP).

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

  // Variables to check for pile-up photons.
  float m_event_PV_z;
  std::vector<float> m_ph_z;
  std::vector<float> m_ph_dz;
  std::vector<float> m_ph_dz_ratio;

  // Boolean whether we're looking at a Wty (5FS) sample.
  bool m_is_tWy{false};
};
}  // namespace ttgamma
}  // namespace top

#endif  // TTGAMMAEVENTSAVER_EVENTSAVER_H_
