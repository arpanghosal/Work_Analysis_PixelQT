#ifndef TTGAMMAEVENTSAVER_TRUTHELECTRONEXTRACTOR_H_
#define TTGAMMAEVENTSAVER_TRUTHELECTRONEXTRACTOR_H_

#include "xAODTruth/TruthParticle.h"
#include <vector>

namespace top {
class Event;

namespace ttgamma {
/**
 * A class to extract a container of truth electrons from a given
 * event. This takes the event, loops over all particles in the
 * truth record, and extracts all electrons meeting certain
 * selection criteria. These include:
 *   - abs(PDG ID) == 11
 *   - pT > 10 GeV
 *   - |eta| < 3.0
 *   - barcode <= 1e5
 */
class TruthElectronExtractor {
 public:
  /// Default class constructor. Takes an event and extract all
  /// truth electrons from it.
  explicit TruthElectronExtractor(const top::Event& event);

  /// Defaulted destructor.
  ~TruthElectronExtractor();

  /// Explicitly deleted copy constructors.
  TruthElectronExtractor(TruthElectronExtractor&& e) = delete;
  TruthElectronExtractor(const TruthElectronExtractor& e) = delete;

  /// Explicitly deleted assignment operator.
  TruthElectronExtractor& operator=(const TruthElectronExtractor& e) = delete;

  /// Main function to get the truth electrons.
  const std::vector<const xAOD::TruthParticle*>& TruthElectrons();

  std::vector<const xAOD::TruthParticle*> m_electrons;
};
}  // namespace ttgamma
}  // namespace top

#endif  // TTGAMMAEVENTSAVER_TRUTHELECTRONEXTRACTOR_H_
