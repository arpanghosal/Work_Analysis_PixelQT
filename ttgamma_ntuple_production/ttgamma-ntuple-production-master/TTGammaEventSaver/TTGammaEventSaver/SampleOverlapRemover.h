#ifndef TTGAMMAEVENTSAVER_SAMPLE_OVERLAP_REMOVER_H_
#define TTGAMMAEVENTSAVER_SAMPLE_OVERLAP_REMOVER_H_

#include "xAODTruth/TruthParticle.h"
#include "TopEvent/Event.h"

namespace top {
namespace ttgamma {
class SampleOverlapRemover {
 public:
  /**
   * Main constructor which generates the list of photon and
   * lepton candidates used for the different sample overlap
   * removal methods. To generate the lists, the following
   * criteria are applied:
   *
   * - PDG ID must be 22 for photons, or within the set of
   *   m_lepton_pdgids for leptons.
   * - Transverse momentum must be pT > m_min_pt.
   * - Particle status must be == 1.
   * - Particle barcode must be below m_max_barcode.
   *
   * @param event Const reference to the event.
   */
  SampleOverlapRemover(const top::Event& event);

  /**
   * Check whether the event is in the sample overlap region
   * (according to the TTH multilepton method). This effectively
   * calls the overloaded inOverlap_TTH(photon, pt_cut, dr_cut)
   * method for all photon candidates in m_cand_photons and
   * returns the inclusive "or".
   *
   * @param pt_cut The minimal pT cut applied to photons
   * @param dr_cut The minimal dR(photon,lepton) required
   * @return Boolean whether event is in sample overlap region.
   */
  bool inOverlap_TTH(float pt_cut, float dr_cut);

  /**
   * Boolean whether the photon in question is in the sample
   * overlap region (according to the TTH multilepton method).
   * This method only accepts photon candidates from the ME
   * calculation by requiring barcode < m_max_barcode, and the
   * photon parent PDG ID to be below 100. Then, the transverse
   * momentum is required to be pT >= pt_cut. The sample overlap
   * region is then defined as: the photon comes from a lepton,
   * and dR(photon,lepton) >= dr_cut.
   *
   * @param photon The photon truth particle
   * @param pt_cut The minimal pT cut applied to photons
   * @param dr_cut The minimal dR(photon,lepton) required
   * @return Boolean whether event is in sample overlap region.
   */
  bool inOverlap_TTH(const xAOD::TruthParticle* photon, float pt_cut, float dr_cut);

  /**
   * Check whether the event is in the sample overlap region
   * (according to the Vgamma group method). This effectively
   * calls the overloaded inOverlap_VGAMMA(photon, pt_cut,
   * dr_cut) method for all photon candidates in m_cand_photons
   * and returns the inclusive "or".
   *
   * @param pt_cut The minimal pT cut applied to photons
   * @param dr_cut The minimal dR(photon,lepton) required
   * @return Boolean whether event is in sample overlap region.
   */
  bool inOverlap_VGAMMA(float pt_cut, float dr_cut);

  /**
   * Boolean whether the event in question is in the sample
   * overlap removal region (according to the Vgamma group
   * method). This method accepts photon candidates from the ME
   * calculation by requiring the photon origin to _not_ be in
   * m_vetoed_ph_origins. Then, the transverse momentum is
   * required to be pt >= pt_cut. The sample overlap region is
   * then defined as: the photon does not overlap with any of the
   * lepton candidates in m_cand_leptons, i.e. dR(photon,lepton)
   * >= dr_cut.
   *
   * @param photon The photon truth particle
   * @param pt_cut The minimal pT cut applied to photons
   * @param dr_cut The minimal dR(photon,lepton) required
   * @return Boolean whether event is in sample overlap region.
   */
  bool inOverlap_VGAMMA(const xAOD::TruthParticle* photon, float pt_cut, float dr_cut);

  /**
   * Boolean whether the event in question is in the sample
   * overlap removal region (according to the rel. 20.7 method).
   * This method uses a reconstructed photon, looks at its truth
   * origin value, and then accepts prompt photons from the
   * dedicated ttbar+gamma (and V+gamma) samples, and non-prompt
   * photons from all others.
   *
   * CAUTION: a positive return value means that the photon
   * passed all prompt criteria, i.e. these events should be
   * accepted from dedicated X+gamma samples, whereas negative
   * return values mean that the photon failed the prompt
   * criteria (such events should be taken from the non-dedicated
   * samples).
   *
   * @param reco_photon The reconstructed photon, on which the
   *        the event categorisation is based.
   * @return Boolean whether event is in sample overlap region.
   */
  bool inOverlap_REL20(const xAOD::Photon* reco_photon);

 private:
  /// Collection of photon candidates for TTH technique.
  std::vector<const xAOD::TruthParticle*> m_cand_photons_TTH;

  /// Collection of photon candidates for VGAMMA technique.
  std::vector<const xAOD::TruthParticle*> m_cand_photons_VGAMMA;

  /// Collection of lepton candidates for VGAMMA technique.
  std::vector<const xAOD::TruthParticle*> m_cand_leptons_VGAMMA;

  /// Minimal pT to consider for ME particles.
  const float m_min_pt{3e3};

  /// Collection of lepton PDG IDs.
  const std::set<int> m_lepton_pdgids = {11, -11, 13, -13, 15, -15};

  /// Collection of photon origins to veto (not from ME).
  const std::set<int> m_vetoed_ph_origins = {9, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 42};

  /// Collection of lepton origins to veto (not from ME).
  const std::set<int> m_vetoed_lep_origins = {3, 5, 6, 7, 8, 9, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42};
};
}  // namespace ttgamma
}  // namespace top

#endif  // TTGAMMAEVENTSAVER_SAMPLE_OVERLAP_REMOVER_H_
