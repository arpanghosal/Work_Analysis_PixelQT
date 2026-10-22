#include "TTGammaEventSaver/PhotonTruthInfo.h"

#include "xAODTruth/xAODTruthHelpers.h"
#include "TTGammaEventSaver/PhotonCategory.h"

namespace {
/**
 * Get the parent of a particle. This goes back in the particle
 * chain step by step, until a particle with a different PDG ID
 * is found.
 * @param particle Reference to the particle in question.
 * @return A pointer to the parent.
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
 * Check whether the ancestry of a particle contains a top quark.
 * @param particle Reference to the particle in question.
 * @return A boolean
 */
bool ancestry_contains_top(const xAOD::TruthParticle* particle) {
  const auto& parent = get_parent(particle);
  if (!parent) return false;

  if (std::abs(parent->pdgId()) == 6) {
    return true;
  } else {
    return ancestry_contains_top(parent);
  }
}
}  // namespace



namespace top {
namespace ttgamma {
// -----------------------------------------------
PhotonTruthInfo::PhotonTruthInfo(const xAOD::Photon& photon, const std::vector<const xAOD::TruthParticle*>& truth_electrons)
  : m_truth_electrons(truth_electrons) {
  const auto& truth_photon{xAOD::TruthHelpers::getTruthParticle(photon)};
  if (truth_photon) {
    four_mom = truth_photon->p4();
    topparent = static_cast<char>(ancestry_contains_top(truth_photon));
    const auto& parent_part = get_parent(truth_photon);
    parent = parent_part ? parent_part->pdgId() : 0;
    barcode = truth_photon->barcode();
    pdgid = truth_photon->pdgId();
  }

  // Save photon true origin and type.
  SG::AuxElement::Accessor<int> originph("truthOrigin");
  origin = originph.isAvailable(photon) ? originph(photon) : 0;
  SG::AuxElement::Accessor<int> typeph("truthType");
  type = typeph.isAvailable(photon) ? typeph(photon) : 0;

  category = static_cast<typename std::underlying_type<PhotonCategory>::type>(categorizePhoton());
}

// -----------------------------------------------
PhotonTruthInfo::~PhotonTruthInfo() = default;

// -----------------------------------------------
PhotonCategory PhotonTruthInfo::categorizePhoton() {
  PhotonCategory cat{PhotonCategory::NotDefined};

  // Criteria to categorize photon as electronic fake.
  if (cat == PhotonCategory::NotDefined) {
    // true PDG ID is electron
    if (std::abs(pdgid) == 11) return PhotonCategory::Efake;

    // Check overlap with truth electrons. If dR is smaller than
    // 0.1, this is also an efake photon.
    for (const auto& electron : m_truth_electrons) {
      if (four_mom.DeltaR(electron->p4()) < 0.1) return PhotonCategory::EfakeDeltaR;
    }
  }

  // Criteria to categorize photon as hadronic fake.
  if (cat == PhotonCategory::NotDefined) {
    // origin: mesons/baryons, type: bkg photon.
    if (origin >= 23 && origin <= 35 && type == 16) return PhotonCategory::Hfake;

    // origin: pi zero, type: bkg photon.
    if (origin == 42 && type == 16) return PhotonCategory::Hfake;

    // type: hadronic energy deposition, misidentified as photon.
    if (type == 17) return PhotonCategory::Hfake;
  }

  // Criteria to categorize different types of prompt photons.
  // Whereas the previous categories entirely rely on
  // ph_true_origin and ph_true_type, this also uses ancestry
  // information to divide into sub-categories.
  if (cat == PhotonCategory::NotDefined) {
    // type "Unknown", origin "NonDefined"
    if (type == 13 && origin == 0) cat = PhotonCategory::ISR;

    // parent is gluon, and no top in ancestry
    if (!topparent && parent == 21) cat = PhotonCategory::ISR;

    // type "BkgPhoton", origin "UndrPhot"
    if (type == 16 && origin == 38) cat = PhotonCategory::FromTop;

    // parent is top
    if (std::abs(parent) == 6) cat = PhotonCategory::FromTop;

    // type "NonIsoPhoton", origin "FSRPhot"
    if (type == 15 && origin == 40) cat = PhotonCategory::FromW;

    // parent is W, and top in ancestry
    if (topparent && std::abs(parent) == 24) cat = PhotonCategory::FromW;

    // parent is leptonic, and top in ancestry
    if (topparent && std::abs(parent) > 10 && std::abs(parent) < 20) cat = PhotonCategory::FromLepton;

    // Does not fit any of the above categories
    if (cat == PhotonCategory::NotDefined) cat = PhotonCategory::Other;
  }
  return cat;
}
}  // namespace ttgamma
}  // namespace top
