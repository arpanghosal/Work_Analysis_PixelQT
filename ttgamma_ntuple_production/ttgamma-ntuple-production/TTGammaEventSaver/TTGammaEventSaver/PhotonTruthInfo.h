#ifndef TTGAMMAEVENTSAVER_PHOTON_TRUTH_INFO_H_
#define TTGAMMAEVENTSAVER_PHOTON_TRUTH_INFO_H_

#include "xAODEgamma/Photon.h"
#include "xAODTruth/TruthParticle.h"
#include "TLorentzVector.h"

namespace top {
namespace ttgamma {
enum class PhotonCategory;

/**
 * Class to retrieve and generate photon truth info. A large
 * bunch of the stored variables is just retrieved from the
 * corresponding truth photon directly, but some also need
 * internal calculations/categorizations etc.
 */
class PhotonTruthInfo {
 public:
  /// Main constructor to retrieve truth info for a photon. Also
  /// takes vector of truth electrons to check overlap.
  PhotonTruthInfo(const xAOD::Photon& photon, const std::vector<const xAOD::TruthParticle*>& truth_electrons);

  /// Defaulted destructor.
  ~PhotonTruthInfo();

  /// Explicitly deleted copy constructors.
  PhotonTruthInfo(PhotonTruthInfo&& e) = delete;
  PhotonTruthInfo(const PhotonTruthInfo& e) = delete;

  /// Explicitly deleted assignment operator.
  PhotonTruthInfo& operator=(const PhotonTruthInfo& e) = delete;

  /// Perform photon categorization based on truth info.
  PhotonCategory categorizePhoton();

  /// Category of the photon, according to TTGamma::PhotonCategory.
  int category{0};
  /// Ancestry of the xAOD::TruthParticle leads to a top.
  char topparent{0};
  /// PDG ID of the parent of the xAOD::TruthParticle.
  int parent{0};
  /// barcode of the corresponding xAOD::TruthParticle.
  int barcode{0};
  /// Defined in MCTruthClassifier/MCTruthClassifierDefs.h.
  int origin{0};
  /// PDG ID of the corresponding xAOD::TruthParticle.
  int pdgid{0};
  /// Defined in MCTruthClassifier/MCTruthClassifierDefs.h.
  int type{0};
  /// Four-momentum of the photon.
  TLorentzVector four_mom{};

private:
  /// Vector of constant pointers to the xAOD::TruthParticle electrons.
  std::vector<const xAOD::TruthParticle*> m_truth_electrons;
};
}  // namespace ttgamma
}  // namespace top

#endif  // TTGAMMAEVENTSAVER_PHOTON_TRUTH_INFO_H_
