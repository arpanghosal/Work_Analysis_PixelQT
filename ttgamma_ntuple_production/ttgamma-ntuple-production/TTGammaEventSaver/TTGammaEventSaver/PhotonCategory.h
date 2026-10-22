

namespace top {
namespace ttgamma {
/// Event category to describe the origin of a photon. In order
/// to match any of the categories, the photon candidate needs
/// to pass the "good" photon criteria.
enum class PhotonCategory {
  NotDefined = 0,   ///< Event was not classified
  ISR = 1,          ///< Prompt photon from production radiation
  FromTop = 2,      ///< Prompt photon radiated from top
  FromW = 3,        ///< Prompt photon radiated from W
  FromB = 4,        ///< Prompt photon radiated from B
  FromLepton = 5,   ///< Prompt photon radiated from lepton
  Other = 6,        ///< Prompt photon from other sources
  Hfake = 10,       ///< Hadronic fake photon
  Efake = 20,       ///< Electronic fake photon
  EfakeDeltaR = 21, ///< Electronic fake photon (with dR overlap)
};
}
}
