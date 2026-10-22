#include "TTGammaEventSaver/PartonLevelInfo.h"

#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthVertex.h"
#include "TruthUtils/PIDHelpers.h"

#include <algorithm>
#include <iterator>
#include <sstream>

// -----------------------------------------------
// -----------------------------------------------

namespace {
// Forward declaration, implemented below.
top::ttgamma::ParticleSet get_siblings(const xAOD::TruthParticle* p);


// Find particle with requested PDG ID in a ParticleSet. If
// found, remove it and return the pointer to that particle.
const xAOD::TruthParticle* find_remove_return(top::ttgamma::ParticleSet& c, int pdg_id) {
  const xAOD::TruthParticle* part{nullptr};
  for (auto i = c.begin(), last = c.end(); i != last; ) {
    if ((*i)->pdgId() == pdg_id) {
      part = *i;
      i = c.erase(i);
      break;
    } else {
      ++i;
    }
  }
  return part;
}

// Find a pair of leptons within a given ParticleSet. Make sure
// that we have one charged lepton and one neutrino. The 'charge'
// parameter requires a specific charge of the pair. If found,
// return the pair, otherwise two nullptrs. In the returned pair,
// store the one with positive PDG ID first.
top::ttgamma::ParticlePair find_lepton_pair(const top::ttgamma::ParticleSet& container, int charge = 1) {
  for (const auto& p : container) {
    if (!p || !p->isChLepton()) continue;
    for (const auto& c : container) {
      if (!c || !c->isNeutrino()) continue;
      if (c == p) continue;
      if (p->threeCharge() + c->threeCharge() != 3*charge) continue;
      if (p->pdgId() > 0) {
        return top::ttgamma::ParticlePair{p, c};
      } else {
        return top::ttgamma::ParticlePair{c, p};
      }
    }
  }
  return top::ttgamma::ParticlePair{nullptr, nullptr};
}

// Find a pair of quarks within a given ParticleSet. Make sure
// that (1) both particles are quarks, (2) their PDG IDs are
// even/odd (i.e. one up-type, one down-type), (3) their PDG IDs
// are negative/positive (i.e. one quark, one antiquark). The
// 'charge' parameter requires a specific charge of the pair. If
// found, return the pair, otherwise two nullptrs. In the
// returned pair, store the one with positive PDG ID first.
top::ttgamma::ParticlePair find_quark_pair(const top::ttgamma::ParticleSet& container, const int charge = 1) {
  for (const auto& p : container) {
    if (!p || !p->isQuark()) continue;
    for (const auto& c : container) {
      if (!c || !c->isQuark()) continue;
      if (c == p) continue;
      const auto& pdg1 = p->pdgId();
      const auto& pdg2 = c->pdgId();
      if (std::abs(pdg1 + pdg2) % 2 != 1) continue;
      if ((pdg1 < 0 && pdg2 < 0) || (pdg1 > 0 && pdg2 > 0)) continue;
      if (p->threeCharge() + c->threeCharge() != 3*charge) continue;
      if (pdg1 > 0) {
        return top::ttgamma::ParticlePair{p, c};
      } else {
        return top::ttgamma::ParticlePair{c, p};
      }
    }
  }
  return top::ttgamma::ParticlePair{nullptr, nullptr};
}

// Take a TruthParticle and proceed to its state just before
// decay. That is, walk down the self-child chain to the end.
const xAOD::TruthParticle* go_to_decay(const xAOD::TruthParticle* part) {
  if (!part) return part;

  while (true) {
    size_t children_counter{0};
    for (size_t i = 0; i < part->nChildren(); ++i) {
      if (!part->child(i)) continue;
      if (part->child(i)->pdgId() == part->pdgId()) {
        part = part->child(i);
        ++children_counter;
        break;
      }
    }
    if (children_counter == 0) break;
  }
  return part;
}

// Function to go to the first emission of a parton. When the
// emission is found, return the particle right _after_ emission.
// The additional dRcut boolean controls whether to skip
// radiations smaller than dR = 0.4 for this check.
const xAOD::TruthParticle* go_to_1st_emission(const xAOD::TruthParticle* part, bool dRcut = false) {
  if (!part) return part;

  bool found_1st_emission{false};
  for (size_t i = 0; i < part->nChildren(); ++i) {
    if (!part->child(i)) continue;

    // We only want to look at radiation, not at decays, so find
    // the child of the particle with identical PDG ID.
    if (part->child(i)->pdgId() != part->pdgId()) continue;

    const auto& child = part->child(i);
    auto siblings = get_siblings(child);
    for (const auto& s : siblings) {
      if (!s) continue;
      if (std::abs(s->pdgId()) == 21) {
        if (dRcut && part->p4().DeltaR(child->p4()) < 0.4) continue;
        found_1st_emission = true;
        part = child;
        break;
      }
    }
    if (found_1st_emission) break;
  }  // for (children)

  return part;
}

// Find the decay vertex of a given particle, and return a
// ParticleSet object with all outgoing particles. If this fails
// for some reason, return an empty set.
top::ttgamma::ParticleSet get_decay_particles(const xAOD::TruthParticle* p) {
  top::ttgamma::ParticleSet particles{};
  const auto& last = go_to_decay(p);
  if (!last || !last->hasDecayVtx()) return particles;
  const auto& vtx = last->decayVtx();
  for (size_t i = 0; i < vtx->nOutgoingParticles(); ++i) {
    if (!vtx->outgoingParticle(i)) continue;
    particles.insert(vtx->outgoingParticle(i));
  }
  return particles;
}

// Get descendant particles of a given parton, until a hadron or
// an internal MC particle is found. Returns 'false' as long as
// these conditions are not fulfilled.
bool get_descendants_rbf_had(const xAOD::TruthParticle* p, top::ttgamma::ParticleSet& particles, const xAOD::TruthVertex* stopvtx = nullptr) {
  if (!p) return false;

  // When the child particle is a hadron or an internal MC
  // particle (ID between 81 and 100, e.g. for Herwig7, id 81
  // means cluster), return true, so that the mother particle
  // will be saved. This also ends the loop over its sibling.
  if (MC::PID::isHadron(abs(p->pdgId())) || (p->pdgId() >= 81 && p->pdgId() <= 100)) {
    return true;
  }

  const auto& vtx = p->decayVtx();
  // When the decay vertex does not exist, it is a non-hadron
  // final state particle (checked in the log, almost all of them
  // are super soft gluons). Prefer to store it although we don't
  // know why the decay vtx is missing ... Return 'false' so that
  // the next sibling will be checked.
  if (!vtx) {
    particles.insert(p);
    return false;
  } else if (vtx == stopvtx) {
    particles.insert(p);
    return false;
  }

  for (size_t i = 0; i < vtx->nOutgoingParticles(); ++i) {
    const xAOD::TruthParticle* p2 = vtx->outgoingParticle(i);
    // Record the mother particle when one of its children is hadron.
    bool record = get_descendants_rbf_had(p2, particles, stopvtx);
    if (record) {
      particles.insert(p);
      break;
    }
  }

  // When non of the 'return' conditions above are met, this
  // check for particle 'p' is finished. Continue with p's next
  // sibling. 'false' is returned so that the loop over p's
  // sibling is not interrupted.
  return false;
}

// Get the ancestor for an initial parton.
const xAOD::TruthParticle* get_ancestor_initialparton(const xAOD::TruthParticle* p) {
  if (!p) return p;

  while (true) {
    size_t parent_counter{0};
    const xAOD::TruthParticle* p_parent{nullptr};
    for (size_t i = 0; i < p->nParents(); ++i) {
      if (!p->parent(i)) continue;
      p_parent = p->parent(i);
      if (p->parent(i)->pdgId() == p->pdgId()) {
        p = p->parent(i);
        ++parent_counter;
        break;
      }
    }
    if (parent_counter == 0) {
      if (p_parent != nullptr) p = p_parent;
      break;
    }
  }
  return p;
}

// Print all ancestors until one of them is a hadron.
bool print_history(const xAOD::TruthParticle* p, bool ancestor, const xAOD::TruthVertex* stopvtx = nullptr, int layer = 0, int index = 0) {
  if (!p) return false;

  if (MC::PID::isHadron(abs(p->pdgId()))) {
    for (int i = 0; i < layer; i++) std::cout << "   ";
    std::cout << "has hadron, stop" << std::endl;
    return true;
  }

  for (int i = 0; i < layer; i++) std::cout << "   ";
  std::cout << "layer:" << (ancestor ? -1*layer : layer) << " index:" << index << " id:" << p->pdgId() << " status:" << p->status() << " barcode:" << p->barcode() << " px:" << p->px() << " py:" << p->py() << " pz:" << p->pz() << std::endl;

  const auto& prodvtx = p->prodVtx();
  const auto& decvtx = p->decayVtx();
  if (ancestor) {
    if (!prodvtx) {
      for (int i = 0; i < layer; i++) std::cout << "   ";
      std::cout << "no production vtx, stop" << std::endl;
      return false;
    } else if (prodvtx == stopvtx) {
      for (int i = 0; i < layer; i++) std::cout << "   ";
      std::cout << "production vtx is the stop vtx, stop" << std::endl;
      return false;
    }
  } else {
    if (!decvtx) {
      for (int i = 0; i < layer; i++) std::cout << "   ";
      std::cout << "no decay vtx, stop" << std::endl;
      return false;
    } else if (decvtx == stopvtx) {
      for (int i = 0; i < layer; i++) std::cout << "   ";
      std::cout << "decay vtx is the stop vtx, stop" << std::endl;
      return false;
    }
  }

  if (prodvtx) {
    for (size_t i = 0; i < prodvtx->nIncomingParticles(); ++i) {
      const xAOD::TruthParticle* p2 = prodvtx->incomingParticle(i);
      if (!p2) continue;
      for (int j = 0; j < layer; j++) std::cout << "   ";
      std::cout << "parent:" << i << " id:" << p2->pdgId() << " status:" << p2->status() << " barcode:" << p2->barcode() << " px:" << p2->px() << " py:" << p2->py() << " pz:" << p2->pz() << std::endl;
    }
    for (size_t i = 0; i < prodvtx->nOutgoingParticles(); ++i) {
      const xAOD::TruthParticle* p2 = prodvtx->outgoingParticle(i);
      if (!p2) continue;
      if (p2->barcode() == p->barcode()) continue;
      for (int j = 0; j < layer; j++) std::cout << "   ";
      std::cout << "sibling:" << i << " id:" << p2->pdgId() << " status:" << p2->status() << " barcode:" << p2->barcode() << " px:" << p2->px() << " py:" << p2->py() << " pz:" << p2->pz() << std::endl;
    }
  }
  if (decvtx) {
    for (size_t i = 0; i < decvtx->nIncomingParticles(); ++i) {
      const xAOD::TruthParticle* p2 = decvtx->incomingParticle(i);
      if (!p2) continue;
      if (p2->barcode() == p->barcode()) continue;
      for (int j = 0; j < layer; j++) std::cout << "   ";
      std::cout << "friend:" << i << " id:" << p2->pdgId() << " status:" << p2->status() << " barcode:" << p2->barcode() << " px:" << p2->px() << " py:" << p2->py() << " pz:" << p2->pz() << std::endl;
    }
    for (size_t i = 0; i < decvtx->nOutgoingParticles(); ++i) {
      const xAOD::TruthParticle* p2 = decvtx->outgoingParticle(i);
      if (!p2) continue;
      for (int j = 0; j < layer; j++) std::cout << "   ";
      std::cout << "children:" << i << " id:" << p2->pdgId() << " status:" << p2->status() << " barcode:" << p2->barcode() << " px:" << p2->px() << " py:" << p2->py() << " pz:" << p2->pz() << std::endl;
    }
  }

  if (ancestor) {
    for (size_t i = 0; i < prodvtx->nIncomingParticles(); ++i) {
      const xAOD::TruthParticle* p2 = prodvtx->incomingParticle(i);
      bool tobreak = print_history(p2, ancestor, stopvtx, layer+1, i);
      if (tobreak) break;
    }
  } else {
    for (size_t i = 0; i < decvtx->nOutgoingParticles(); ++i) {
      const xAOD::TruthParticle* p2 = decvtx->outgoingParticle(i);
      bool tobreak = print_history(p2, ancestor, stopvtx, layer+1, i);
      if (tobreak) break;
    }
  }

  return false;
}

// Find the production vertex of a particle and return a
// ParticleSet object with all its siblings (excluding the
// particle itself).
top::ttgamma::ParticleSet get_siblings(const xAOD::TruthParticle* p) {
  top::ttgamma::ParticleSet particles{};
  if (!p || !p->hasProdVtx()) return particles;
  const auto& vtx = p->prodVtx();
  for (size_t i = 0; i < vtx->nOutgoingParticles(); ++i) {
    if (!vtx->outgoingParticle(i)) continue;
    if (vtx->outgoingParticle(i) == p) continue;
    particles.insert(vtx->outgoingParticle(i));
  }
  return particles;
}

// Print the content of a given ParticleSet object.
void print_particle_set(const top::ttgamma::ParticleSet& c, const std::string& s) {
  std::stringstream ss;
  for (const auto& p : c) ss << p->pdgId() << " (" << p << ") ";
  std::cout << "PartonLevelInfo:\t" << s << " with the following particles: " << ss.str() << std::endl;
}

// Check whether the given ParticleSet contains pointer(s) to the
// same TruthParticle object as the one given. If yes, remove
// them from the set.
void remove_from_set(top::ttgamma::ParticleSet& c, const xAOD::TruthParticle* part) {
  if (!part) return;
  for (auto i = c.begin(), last = c.end(); i != last; ) {
    if (*i == part) {
      i = c.erase(i);
    } else {
      ++i;
    }
  }
}
}  // namespace


// -----------------------------------------------
// -----------------------------------------------

namespace top {
namespace ttgamma {
// -----------------------------------------------
PartonLevelInfo::PartonLevelInfo(const xAOD::TruthParticleContainer* truth, bool is_tWy)
  : m_debug(false)
  , m_truth_record(truth)
  , m_is_tWy(is_tWy) {
  if (!m_truth_record) return;

  // This constructor sequentially goes through all steps
  // necessary to build the parton-level information. The biggest
  // problem when reconstruting tty and Wty is that there might
  // be various scenarios for how truth particles are linked. In
  // an ideal event, the procedure is to:
  //  1) Go through the truth record and find top/antitop.
  //  2) Identify W bosons and b quarks through the decay
  //     vertices of the top/antitop.
  //  3) Identify decay products of the W bosons through the
  //     decay vertices of the W bosons.
  //  4) Find the photon. This can be either (a) at the top or
  //     antitop decay vertex, (b) at one of the two W boson
  //     decay vertices, (c) a sibling of the top/antitop pair.
  //
  // Now things can get difficult when:
  //  1) One or both of the top quarks is off-shell, because then
  //     their decay products might be directly attached to the
  //     production vertex and the truth record does _not_ contain
  //     any top/antitop quark. We need to account for that.
  //  2) One or both of the two W bosons is off-shell, which leads
  //     to a similar scenario. The W boson decay products are then
  //     attached to the W production vertex.

  // First step: make an attempt to identify top/antitop quarks
  // in the truth record. If found, they are on-shell.
  for (const auto& particle : *m_truth_record) {
    if (!particle) continue;
    if (particle->pdgId() == 6) {
      m_top_initial = particle;
      break;
    }
  }
  for (const auto& particle : *m_truth_record) {
    if (!particle) continue;
    if (particle->pdgId() == -6) {
      m_antitop_initial = particle;
      break;
    }
  }

  // Next step: try to find the production vertex. If at least
  // one of the top quarks exist, take its production vertex (or
  // the vertices of both top/antitop) as production vertex. If
  // no tops exist, find a vertex with W+b.
  if (m_top_initial || m_antitop_initial) {
    auto top_siblings = get_siblings(m_top_initial);
    auto antitop_siblings = get_siblings(m_antitop_initial);
    std::set_union(top_siblings.begin(), top_siblings.end(),
                   antitop_siblings.begin(), antitop_siblings.end(),
                   std::inserter(m_production_vtx, m_production_vtx.begin()));
    remove_from_set(m_production_vtx, m_antitop_initial);
    remove_from_set(m_production_vtx, m_top_initial);
  } else {
    m_production_vtx = findWbVertex();
  }

  if (m_debug) print_particle_set(m_production_vtx, "Production vertex");

  // Next step: in case the top/antitop exist, find their decay
  // vertices and get the sets of decay particles.
  m_top_decay_vtx = get_decay_particles(m_top_initial);
  m_antitop_decay_vtx = get_decay_particles(m_antitop_initial);

  if (m_debug) print_particle_set(m_top_decay_vtx, "Top decay vertex");
  if (m_debug) print_particle_set(m_antitop_decay_vtx, "Antitop decay vertex");

  // Next step: find the bottom and antibottom quarks. This
  // considers the top/antitop quark decay vertices and, if no b
  // quark is found there, moves to the production vertex.
  m_bottom = findBottomQuark(5);
  m_antibottom = findBottomQuark(-5);

  // Next step: find the W bosons. This considers the top/antitop
  // quark decay vertices and, if no W boson is found there,
  // tries to identify W decay products on that vertex. If not
  // found, move to the production vertex.
  m_W_plus_decay = findWDecayParticles(24);
  m_W_minus_decay = findWDecayParticles(-24);

  if (m_debug) print_particle_set(m_W_plus_decay_vtx, "W decay vertex");
  if (m_debug) print_particle_set(m_W_minus_decay_vtx, "W decay vertex");

  // Next step: find the photon. This considers the production
  // vertex, the top/antitop quark decay vertices and the W decay
  // vertices.
  m_photon = findPhoton();
  if (m_photon) m_photon_afterall = go_to_decay(m_photon);
  m_photons_afterall = findPhotonsAfterall();

  // Store the particles after first radiation if that first
  // radiation is hard (i.e. dR > 0.4 to the original particle).
  // Otherwise, we stick to the particle before that emission.
  if (m_photon) m_photon_after1st = go_to_1st_emission(m_photon, true);
  if (m_W_plus_decay.first) m_W_plus_decay_after1st.first = go_to_1st_emission(m_W_plus_decay.first, true);
  if (m_W_plus_decay.second) m_W_plus_decay_after1st.second = go_to_1st_emission(m_W_plus_decay.second, true);
  if (m_W_minus_decay.first) m_W_minus_decay_after1st.first = go_to_1st_emission(m_W_minus_decay.first, true);
  if (m_W_minus_decay.second) m_W_minus_decay_after1st.second = go_to_1st_emission(m_W_minus_decay.second, true);
  if (m_W_plus_decay.first) m_W_plus_decay_afterall.first = go_to_decay(m_W_plus_decay.first);
  if (m_W_plus_decay.second) m_W_plus_decay_afterall.second = go_to_decay(m_W_plus_decay.second);
  if (m_W_minus_decay.first) m_W_minus_decay_afterall.first = go_to_decay(m_W_minus_decay.first);
  if (m_W_minus_decay.second) m_W_minus_decay_afterall.second = go_to_decay(m_W_minus_decay.second);
  if (m_bottom) m_bottom_after1st = go_to_1st_emission(m_bottom, true);
  if (m_antibottom) m_antibottom_after1st = go_to_1st_emission(m_antibottom, true);

  // Store ancestors of incoming particles.
  const xAOD::TruthVertex* prodvtx = nullptr;
  m_ancestor_incoming1 = nullptr;
  m_ancestor_incoming2 = nullptr;
  m_isr_rbf_had.clear();
  if (prodvtx == nullptr && m_top_initial != nullptr) prodvtx = m_top_initial->prodVtx();
  if (prodvtx == nullptr && m_antitop_initial != nullptr) prodvtx = m_antitop_initial->prodVtx();
  if (prodvtx && prodvtx->nIncomingParticles() == 2) {
    const xAOD::TruthParticle* p1 = prodvtx->incomingParticle(0);
    const xAOD::TruthParticle* p2 = prodvtx->incomingParticle(1);
    m_ancestor_incoming1 = get_ancestor_initialparton(p1);
    m_ancestor_incoming2 = get_ancestor_initialparton(p2);
    get_descendants_rbf_had(m_ancestor_incoming1, m_isr_rbf_had, prodvtx);
    get_descendants_rbf_had(m_ancestor_incoming2, m_isr_rbf_had, prodvtx);
    remove_from_set(m_isr_rbf_had, p1);
    remove_from_set(m_isr_rbf_had, p2);
  }

  // Find the b-parton jets.
  m_descendants_rbf_had_bottom.clear();
  m_descendants_rbf_had_antibottom.clear();
  get_descendants_rbf_had(m_bottom, m_descendants_rbf_had_bottom);
  get_descendants_rbf_had(m_antibottom, m_descendants_rbf_had_antibottom);

  // Tag b-parton jets
  m_bottom_rbf_had = nullptr;
  m_antibottom_rbf_had = nullptr;

  if (!m_is_tWy) {
    // This is the easier case in which we expect two b-partons
    // to exist (i.e. not the 5-flavour Wty sample).
    for (const auto& p : m_descendants_rbf_had_bottom) {
      if (p->pdgId() == 5) {
        m_bottom_rbf_had = p;
        break;
      }
    }
    for (const auto& p : m_descendants_rbf_had_antibottom) {
      if (p->pdgId() == -5) {
        m_antibottom_rbf_had = p;
        break;
      }
    }
  } else {
    // More difficult case (5-flavour scheme Wty sample), where
    // we only expect to have one b-parton.
    if (m_bottom && !m_antibottom) {
      // Only bottom, but not antibottom exist. Then try to find
      // the antibottom from the incoming particles.
      for (const auto& p : m_descendants_rbf_had_bottom) {
        if (p->pdgId() == 5) {
          m_bottom_rbf_had = p;
          break;
        }
      }
      for (const auto& p : m_isr_rbf_had) {
        if (p->pdgId() == -5) {
          m_antibottom_rbf_had = p;
          break;
        }
      }
    } else if (m_antibottom && !m_bottom) {
      // Only antibottom, but not bottom exist. Then try to find
      // the bottom from the incoming particles.
      for (const auto& p : m_descendants_rbf_had_antibottom) {
        if (p->pdgId() == -5) {
          m_antibottom_rbf_had = p;
          break;
        }
      }
      for (const auto& p : m_isr_rbf_had) {
        if (p->pdgId() == 5) {
          m_bottom_rbf_had = p;
          break;
        }
      }
    } else if (m_antibottom && m_bottom) {
      // Both b-partons exist.
      for (const auto& p : m_descendants_rbf_had_bottom) {
        if (p->pdgId() == 5) {
          m_bottom_rbf_had = p;
          break;
        }
      }
      for (const auto& p : m_descendants_rbf_had_antibottom) {
        if (p->pdgId() == -5) {
          m_antibottom_rbf_had = p;
          break;
        }
      }
    }
  }

  // Store flags for dilepton events and e+mu channel.
  if (m_W_plus_decay.first && m_W_minus_decay.first) {
    const auto& p1 = m_W_plus_decay.second;   // charged anti-lepton from W plus
    const auto& p2 = m_W_minus_decay.first;   // charged lepton from W minus

    auto is_e = [] (const xAOD::TruthParticle* p) {
      return (std::abs(p->pdgId()) == 11) ? true : false;
    };

    auto is_mu = [] (const xAOD::TruthParticle* p) {
      return (std::abs(p->pdgId()) == 13) ? true : false;
    };

    if (p1->isChLepton() && p2->isChLepton()) m_is_dilepton = true;
    if (is_e(p1) && is_mu(p2)) m_is_emu_channel = true;
    if (is_e(p2) && is_mu(p1)) m_is_emu_channel = true;
  }

  if (m_debug) {
    if (!m_photon) std::cout << "PartonLevelInfo:	" << "Did _NOT_ find a photon in the event" << std::endl;
    if (!m_bottom) std::cout << "PartonLevelInfo:	" << "Did _NOT_ find a bottom quark in the event" << std::endl;
    if (!m_antibottom) std::cout << "PartonLevelInfo:	" << "Did _NOT_ find an antibottom quark in the event" << std::endl;
    if (!m_W_plus_decay.first) std::cout << "PartonLevelInfo:	" << "Did _NOT_ find both decay products of W_plus" << std::endl;
    if (!m_W_minus_decay.first) std::cout << "PartonLevelInfo:	" << "Did _NOT_ find both decay products of W_minus" << std::endl;
    std::cout << "PartonLevelInfo:	" << "=== END OF EVENT ===" << std::endl << std::endl;
  }
}

// -----------------------------------------------
const xAOD::TruthParticle* PartonLevelInfo::findBottomQuark(const int pdg_id) {
  if (std::abs(pdg_id) != 5) throw std::invalid_argument("");

  const xAOD::TruthParticle* p{nullptr};
  if (pdg_id == 5) {
    p = find_remove_return(m_top_decay_vtx, pdg_id);
  } else {
    p = find_remove_return(m_antitop_decay_vtx, pdg_id);
  }

  if (!p) {
    p = find_remove_return(m_production_vtx, pdg_id);
  }
  return p;
}

// -----------------------------------------------
const xAOD::TruthParticle* PartonLevelInfo::findPhoton() {
  const xAOD::TruthParticle* part{nullptr};
  part = find_remove_return(m_production_vtx, 22);
  if (part) return part;
  part = find_remove_return(m_top_decay_vtx, 22);
  if (part) return part;
  part = find_remove_return(m_antitop_decay_vtx, 22);
  if (part) return part;
  part = find_remove_return(m_W_plus_decay_vtx, 22);
  if (part) return part;
  part = find_remove_return(m_W_minus_decay_vtx, 22);
  if (part) return part;
  return nullptr;
}

// -----------------------------------------------
ParticleSet PartonLevelInfo::findPhotonsAfterall() {
  ParticleSet particles;
  for (const auto& particle : *m_truth_record) {
    if (particle->pdgId() == 22 && particle->status() == 1) {
      particles.insert(particle);
    }
  }
  return particles;
}

// -----------------------------------------------
ParticleSet PartonLevelInfo::findWbVertex() {
  for (const auto& particle : *m_truth_record) {
    if (!particle) continue;
    if (std::abs(particle->pdgId()) != 5) continue;

    // If |PDG ID| = 5, we have a b quark. Then consider two
    // cases: (1) the W boson is on-shell and exists (PDG ID =
    // 24), (2) no W boson is found, but the b quark as a quark
    // pair or a lepton pair as sibling (off-shell scenario).
    auto siblings = get_siblings(particle);
    if (siblings.empty()) continue;

    // If W is there, return the b quark + siblings.
    for (const auto& s : siblings) {
      if (!s) continue;
      if (std::abs(s->pdgId()) == 24) {
        siblings.insert(particle);
        return siblings;
      }
    }

    if (siblings.size() >= 2) {
      const auto& quark_pair = find_quark_pair(siblings);
      const auto& lepton_pair = find_lepton_pair(siblings);

      if (quark_pair.first || lepton_pair.first) {
        siblings.insert(particle);
        return siblings;
      }
    }
  }  // for loop over truth record

  return ParticleSet{};
}

// -----------------------------------------------
ParticlePair PartonLevelInfo::findWDecayParticles(const int pdg_id) {
  if (std::abs(pdg_id) != 24) throw std::invalid_argument("");

  ParticlePair pair{nullptr, nullptr};
  const int charge = (pdg_id > 0) ? 1 : -1;

  // Try to find a real (on-shell) W boson in the event.
  const xAOD::TruthParticle* w_boson{nullptr};
  if (pdg_id > 0 && !m_top_decay_vtx.empty()) {
    w_boson = find_remove_return(m_top_decay_vtx, pdg_id);
  } else if (pdg_id < 0 && !m_antitop_decay_vtx.empty()) {
    w_boson = find_remove_return(m_antitop_decay_vtx, pdg_id);
  } else {
    w_boson = find_remove_return(m_production_vtx, pdg_id);
  }

  if (w_boson && pdg_id > 0) {
    // First case: the W boson is on-shell, so we can just look
    // for a lepton pair or quark pair at its decay vertex.
    m_W_plus_decay_vtx = get_decay_particles(w_boson);
    pair = find_lepton_pair(m_W_plus_decay_vtx, charge);
    if (!pair.first) pair = find_quark_pair(m_W_plus_decay_vtx, charge);
  } else if (w_boson && pdg_id < 0) {
    // Do the same for W minus boson.
    m_W_minus_decay_vtx = get_decay_particles(w_boson);
    pair = find_lepton_pair(m_W_minus_decay_vtx, charge);
    if (!pair.first) pair = find_quark_pair(m_W_minus_decay_vtx, charge);
  } else if (pdg_id > 0 && !m_top_decay_vtx.empty()) {
    // Second case: we're looking for a W plus boson, but there
    // is none in the truth record. If the top decay vertex
    // exists, the W decay products must be found there.
    pair = find_lepton_pair(m_top_decay_vtx, charge);
    if (!pair.first) pair = find_quark_pair(m_top_decay_vtx, charge);
  } else if (pdg_id < 0 && !m_antitop_decay_vtx.empty()) {
    // Third case: we're looking for W minus boson, but there is
    // none in the thruth record. If the antitop decay vertex
    // exists, the W decay products must be found there.
    pair = find_lepton_pair(m_antitop_decay_vtx, charge);
    if (!pair.first) pair = find_quark_pair(m_antitop_decay_vtx, charge);
  } else {
    // Fourth case: neither W nor top/antitop exist in the truth
    // record. Then look for a lepton pair or quark pair at the
    // production vertex of the event.
    pair = find_lepton_pair(m_production_vtx, charge);
    if (!pair.first) pair = find_quark_pair(m_production_vtx, charge);
  }

  return pair;
}
}  // namespace ttgamma
}  // namespace top
