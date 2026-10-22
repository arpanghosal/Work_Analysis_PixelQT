#include "TTGammaEventSaver/TruthElectronExtractor.h"

#include "TopEvent/Event.h"

namespace top {
namespace ttgamma {
// -----------------------------------------------
TruthElectronExtractor::TruthElectronExtractor(const top::Event& event) {
  if (event.m_truth) {
    for (const auto& particle : *event.m_truth) {
      if (!particle) continue;
      if (std::abs(particle->pdgId()) != 11) continue;
      if (particle->pt() < 10e3) continue;
      if (std::abs(particle->eta()) > 3.0) continue;
      if (particle->barcode() > 1e5) continue;
      m_electrons.push_back(particle);
    }
  }
}

// -----------------------------------------------
TruthElectronExtractor::~TruthElectronExtractor() = default;

// -----------------------------------------------
const std::vector<const xAOD::TruthParticle*>& TruthElectronExtractor::TruthElectrons() {
  return m_electrons;
}
}  // namespace ttgamma
}  // namespace top
