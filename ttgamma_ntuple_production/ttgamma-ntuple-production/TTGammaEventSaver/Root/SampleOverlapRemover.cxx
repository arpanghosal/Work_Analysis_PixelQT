#include "TTGammaEventSaver/SampleOverlapRemover.h"
#include "MCTruthClassifier/MCTruthClassifier.h"
#include "xAODTruth/xAODTruthHelpers.h"

namespace {
/**
 * Get the parent of a particle.
 */
const xAOD::TruthParticle* get_parent(const xAOD::TruthParticle* particle) {
  for (size_t i = 0; i < particle->nParents(); ++i) {
    const auto& parent = particle->parent(i);
    if (!parent) continue;

    if (parent->pdgId() == particle->pdgId()) {
      return get_parent(parent);
    } else {
      return parent;
    }
  }
  return nullptr;
}

/**
 * Boolean whether we look at a particle before its decay.
 */
bool is_before_decay(const xAOD::TruthParticle* particle) {
  bool has_children = false;
  for (size_t i = 0; i < particle->nChildren(); ++i) {
    if (!particle->child(i)) continue;
    has_children = true;
    // We're not before decay if the child is identical.
    if (particle->child(i)->pdgId() == particle->pdgId()) return false;
  }
  return has_children;
}

/**
 * Small function to shorten the std::find function for sets.
 * This returns a boolean whether an entry can be found.
 */
template<typename T>
bool isInSet(const std::set<T>& coll, T value) {
  return (std::find(coll.begin(), coll.end(), value) != coll.end());
}
}  // namespace


namespace top {
namespace ttgamma {
// -----------------------------------------------
SampleOverlapRemover::SampleOverlapRemover(const top::Event& event) {
  MCTruthClassifier classifier{"MCTruthClassifier"};
  auto truth_particles = event.m_truth;

  // Return gracefully here and not let this class do anything
  // (e.g. when running over data).
  if (!truth_particles) return;

  // Compile a list of photon candidates (TTH).
  for (const auto& particle : *truth_particles) {
    if (!particle || particle->pdgId() != 22) continue;
    if (particle->pt() < m_min_pt) continue;
    if (particle->status() != 1) continue;
    if (particle->barcode() > 2e5) continue;
    m_cand_photons_TTH.push_back(particle);
  }

  // Compile a list of photon candidates (VGAMMA).
  for (const auto& particle : *truth_particles) {
    if (!particle || particle->pdgId() != 22) continue;
    if (particle->pt() < m_min_pt) continue;
    if (particle->status() != 1) continue;
    if (particle->barcode() > 1e5) continue;
    auto ph_origin = classifier.particleTruthClassifier(particle).second;
    if (isInSet<int>(m_vetoed_ph_origins, ph_origin)) continue;
    m_cand_photons_VGAMMA.push_back(particle);
  }

  // Then compile a list of lepton candidates.
  for (const auto& particle : *truth_particles) {
    if (!particle) continue;
    if (!isInSet<int>(m_lepton_pdgids, particle->pdgId())) continue;
    if (particle->pt() < m_min_pt) continue;
    if (particle->status() != 1 && std::abs(particle->pdgId()) != 15) continue;
    if (particle->barcode() > 1e5) continue;
    auto lep_origin = classifier.particleTruthClassifier(particle).second;
    if (isInSet<int>(m_vetoed_lep_origins, lep_origin)) continue;
    if (std::abs(particle->pdgId()) == 15 && !is_before_decay(particle)) continue;
    if (get_parent(particle) && std::abs(get_parent(particle)->pdgId()) == 15) continue;
    m_cand_leptons_VGAMMA.push_back(particle);
  }
}

// -----------------------------------------------
bool SampleOverlapRemover::inOverlap_TTH(float pt_cut, float dr_cut) {
  // Call the overloaded method for all photon candidates and
  // return 'true' as soon as one of them is in the overlap
  // region.
  for (const auto& ph : m_cand_photons_TTH) {
    if (inOverlap_TTH(ph, pt_cut, dr_cut)) return true;
  }

  return false;
}

// -----------------------------------------------
bool SampleOverlapRemover::inOverlap_TTH(const xAOD::TruthParticle* photon, float pt_cut, float dr_cut) {
  // Ignore anything that's not a photon.
  if (!photon || photon->pdgId() != 22) return false;

  // Make sure the parent pointer makes sense.
  const auto& parent = get_parent(photon);
  if (!parent) return false;

  // Only look at photon from parents generated on ME level.
  if (std::abs(parent->pdgId()) > 99) return false;

  // Only look at photons from parents with non-zero momentum.
  if (!(parent->pt() > 0)) return false;

  // Only pick photons above a certain pT threshold.
  if (photon->pt() < pt_cut) return false;

  // Calculate the dR between photon and parent. Exclude
  // photons under dR threshold coming from a lepton.
  const auto& dR = photon->p4().DeltaR(parent->p4());
  bool is_from_lepton = isInSet<int>(m_lepton_pdgids, parent->pdgId());
  if (is_from_lepton && (dR < dr_cut)) return false;

  // If we have a photon that doesn't overlap with an lepton, we
  // are in the overlap region.
  return true;
}

// -----------------------------------------------
bool SampleOverlapRemover::inOverlap_VGAMMA(float pt_cut, float dr_cut) {
  // Call the overloaded method for all photon candidates and
  // return 'true' as soon as one of them is in the overlap
  // region.
  for (const auto& ph : m_cand_photons_VGAMMA) {
    if (inOverlap_VGAMMA(ph, pt_cut, dr_cut)) return true;
  }

  return false;
}

// -----------------------------------------------
bool SampleOverlapRemover::inOverlap_VGAMMA(const xAOD::TruthParticle* photon, float pt_cut, float dr_cut) {
  // Make sure the photon origin is not in the veto list.

  // Make sure the candidate is above the pT threshold.
  if (photon->pt() < pt_cut) return false;

  // Make sure it doesn't overlap with any of the lepton candidates.
  for (const auto& lepton : m_cand_leptons_VGAMMA) {
    if (lepton->p4().DeltaR(photon->p4()) < dr_cut) return false;
  }

  return true;
}

// -----------------------------------------------
bool SampleOverlapRemover::inOverlap_REL20(const xAOD::Photon* reco_photon) {
  const auto& truth_photon{xAOD::TruthHelpers::getTruthParticle(*reco_photon)};
  if (!truth_photon) {
    // throw std::invalid_argument("top::ttgamma::SampleOverlapRemover: could not retrieve truth photon");
    return false;
  }

  // Exclude photons faked by electrons:
  //  - check truth PDG ID and whether it's an electron
  //  - check whether dR(photon,lepton) < 0.2
  if (std::abs(truth_photon->pdgId()) == 11) return false;
  for (const auto& lep : m_cand_leptons_VGAMMA) {
    if (std::abs(lep->pdgId()) != 11) continue;
    if (truth_photon->p4().DeltaR(lep->p4()) < 0.2) return false;
  }

  // Exclude photons of hadronic origin.
  MCTruthClassifier classifier{"MCTruthClassifier"};
  auto ph_origin = classifier.particleTruthClassifier(truth_photon).second;
  auto ph_true_type = classifier.particleTruthClassifier(truth_photon).first;
  if (isInSet<int>(m_vetoed_ph_origins, ph_origin) && ph_true_type == 16) return false;

  // Otherwise we are looking at a prompt photon.
  return true;
}
}  // namespace ttgamma
}  // namespace top
