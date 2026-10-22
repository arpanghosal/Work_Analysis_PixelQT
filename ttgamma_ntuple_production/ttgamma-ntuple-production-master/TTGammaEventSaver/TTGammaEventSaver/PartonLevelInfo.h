#ifndef TTGAMMAEVENTSAVER_PARTON_LEVEL_INFO_H_
#define TTGAMMAEVENTSAVER_PARTON_LEVEL_INFO_H_

#include "xAODTruth/TruthParticleContainer.h"

#include <set>       ///< For std::set class.
#include <utility>   ///< For std::pair class.

namespace top {
namespace ttgamma {
using ParticlePair = std::pair<const xAOD::TruthParticle*, const xAOD::TruthParticle*>;
using ParticleSet = std::set<const xAOD::TruthParticle*>;

/**
 * Class to retrieve and generate parton level information for an
 * event. This class is written in such a way that it can handle
 * ttbar, tty, Wt, and Wty events.
 */
class PartonLevelInfo {
 public:
  /// Main constructor to retrieve parton level info.
  explicit PartonLevelInfo(const xAOD::TruthParticleContainer* truth, bool is_tWy);

  /** \name Main functions to build parton-level info */
  /* @{ */

  // Find a bottom quark in the event. Depending on the given PDG
  // ID, this looks either at the top decay vertex or the antitop
  // decay vertex to find a b quark. Of none is found, move one
  // level up to the production vertex and identify it.
  const xAOD::TruthParticle* findBottomQuark(const int pdg_id = 5);

  // Find the photon of the event. Depending on the particle, by
  // which the photon was emitted, we need to consider different
  // vertices: the production vertex of the event, the
  // top/antitop quark decay vertices, the W decay vertices.
  const xAOD::TruthParticle* findPhoton();

  // Find all the photons of the event for lepton dressing
  ParticleSet findPhotonsAfterall();

  /// Try to find a W+b vertex. This goes through the truth
  /// record, and tries to find a b quark that has a W sibling.
  /// Because the W boson might be off-shell, also vertices with
  /// a quark pair or a lepton pair as sibling are considered.
  /// Return the set of outgoing particles.
  ParticleSet findWbVertex();

  // Find the decay products of a W boson with PDG ID = +/- 24.
  // This first looks at the top/antitop decay vertex if a real
  // (on-shell) W boson exists. If not, look at the top/antitop
  // decay vertex for the W decay products directly. If not
  // found, move one level up to the production vertex and
  // identify the W decay products.
  ParticlePair findWDecayParticles(const int pdg_id = 24);

  // get descendant particles of the 2 b quarks, in their status right before hadronization start
  ParticleSet getBDescendants() {return m_descendants_rbf_had_bottom;};
  ParticleSet getAntiBDescendants() {return m_descendants_rbf_had_antibottom;};
  ParticleSet getISRDescendants() {return m_isr_rbf_had;};

  /* @} */
  /** \name Getters for the actual output */
  /* @{ */

  /// Boolean whether event is dilepton or not.
  bool isDilepton() { return m_is_dilepton; }

  /// Boolean whether event is e+mu channel or not.
  bool isEMuChannel() { return m_is_emu_channel; }

  const xAOD::TruthParticle* getBottomQuarkBefore() { return m_bottom; }
  const xAOD::TruthParticle* getAntibottomQuarkBefore() { return m_antibottom; }
  const xAOD::TruthParticle* getPhotonBefore() { return m_photon; }
  const xAOD::TruthParticle* getWPlusDecay1Before() { return m_W_plus_decay.first; }
  const xAOD::TruthParticle* getWPlusDecay2Before() { return m_W_plus_decay.second; }
  const xAOD::TruthParticle* getWMinusDecay1Before() { return m_W_minus_decay.first; }
  const xAOD::TruthParticle* getWMinusDecay2Before() { return m_W_minus_decay.second; }

  const xAOD::TruthParticle* getBottomQuarkAfter1st() { return m_bottom_after1st; }
  const xAOD::TruthParticle* getAntibottomQuarkAfter1st() { return m_antibottom_after1st; }
  const xAOD::TruthParticle* getPhotonAfter1st() { return m_photon_after1st; }
  const xAOD::TruthParticle* getWPlusDecay1After1st() { return m_W_plus_decay_after1st.first; }
  const xAOD::TruthParticle* getWPlusDecay2After1st() { return m_W_plus_decay_after1st.second; }
  const xAOD::TruthParticle* getWMinusDecay1After1st() { return m_W_minus_decay_after1st.first; }
  const xAOD::TruthParticle* getWMinusDecay2After1st() { return m_W_minus_decay_after1st.second; }
  const xAOD::TruthParticle* getWPlusDecay1Afterall() { return m_W_plus_decay_afterall.first; }
  const xAOD::TruthParticle* getWPlusDecay2Afterall() { return m_W_plus_decay_afterall.second; }
  const xAOD::TruthParticle* getWMinusDecay1Afterall() { return m_W_minus_decay_afterall.first; }
  const xAOD::TruthParticle* getWMinusDecay2Afterall() { return m_W_minus_decay_afterall.second; }

  const xAOD::TruthParticle* getBottomQuarkRbfHad() { return m_bottom_rbf_had; }
  const xAOD::TruthParticle* getAntibottomQuarkRbfHad() { return m_antibottom_rbf_had; }
  const xAOD::TruthParticle* getPhotonAfterall() { return m_photon_afterall; }

  ParticleSet getPhotonsAfterall() { return m_photons_afterall; }

  /* @} */

 private:
  /// Boolean to output debugging information.
  bool m_debug{false};

  /// Const pointer to the Monte-Carlo truth record.
  const xAOD::TruthParticleContainer* m_truth_record{nullptr};

  /// Const pointer to the top and anti-top quarks.
  const xAOD::TruthParticle* m_top_initial{nullptr};
  const xAOD::TruthParticle* m_antitop_initial{nullptr};
  const xAOD::TruthParticle* m_bottom{nullptr};
  const xAOD::TruthParticle* m_antibottom{nullptr};
  const xAOD::TruthParticle* m_photon{nullptr};
  ParticlePair m_W_plus_decay{nullptr, nullptr};
  ParticlePair m_W_minus_decay{nullptr, nullptr};

  /// Const pointer to the particles after 1st radiation.
  const xAOD::TruthParticle* m_bottom_after1st{nullptr};
  const xAOD::TruthParticle* m_antibottom_after1st{nullptr};
  const xAOD::TruthParticle* m_photon_after1st{nullptr};
  ParticlePair m_W_plus_decay_after1st{nullptr, nullptr};
  ParticlePair m_W_minus_decay_after1st{nullptr, nullptr};
  ParticlePair m_W_plus_decay_afterall{nullptr, nullptr};
  ParticlePair m_W_minus_decay_afterall{nullptr, nullptr};

  /// Const pointer to the particles right before hadronisation.
  const xAOD::TruthParticle* m_bottom_rbf_had{nullptr};
  const xAOD::TruthParticle* m_antibottom_rbf_had{nullptr};
  const xAOD::TruthParticle* m_photon_afterall{nullptr};
  const xAOD::TruthParticle* m_ancestor_incoming1{nullptr};
  const xAOD::TruthParticle* m_ancestor_incoming2{nullptr};

  /// Particle sets to contain vertex information.
  ParticleSet m_production_vtx{};
  ParticleSet m_top_decay_vtx{};
  ParticleSet m_antitop_decay_vtx{};
  ParticleSet m_W_plus_decay_vtx{};
  ParticleSet m_W_minus_decay_vtx{};

  /// Particle sets to construct parton jets. These are meant to correspond to b-jets from top-quark decays.
  ParticleSet m_descendants_rbf_had_bottom{};
  ParticleSet m_descendants_rbf_had_antibottom{};
  ParticleSet m_isr_rbf_had{};

  // Set of photons stored for lepton dressing.
  ParticleSet m_photons_afterall{};

  /// Misc event information variables.
  bool m_is_dilepton{false};
  bool m_is_emu_channel{false};

  // Boolean whether we're looking at a Wty (5FS) sample.
  bool m_is_tWy{false};
};
}  // namespace ttgamma
}  // namespace top

#endif  // TTGAMMAEVENTSAVER_PARTON_LEVEL_INFO_H_
