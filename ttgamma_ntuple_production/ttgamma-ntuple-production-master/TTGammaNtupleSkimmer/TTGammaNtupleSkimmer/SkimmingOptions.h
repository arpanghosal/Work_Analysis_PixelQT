#ifndef TTGAMMA_NTUPLE_SKIMMER_SKIMMING_OPTIONS_H_
#define TTGAMMA_NTUPLE_SKIMMER_SKIMMING_OPTIONS_H_

#include <string>
#include <vector>

namespace TTGammaNtupleSkimmer {
/// Structure to store options of the skimmer.
struct SkimmingOptions {
  bool remove_overlap;
  bool process_truth;
  bool skip_systematics;
  float kfactor;
  float lumi;
  std::string cuts;
  std::vector<std::string> in;
  std::string out;
  std::vector<std::string> slimlist;
  std::vector<std::string> slim_exclude;
  bool debug;
};

/// Parse the command line arguments.
SkimmingOptions get_skim_opts(int argc, char* argv[]);

/// Print the configuration of the skimmer.
void print_config(const SkimmingOptions& opts);
}  // namesapce TTGammaNtupleSkimmer

#endif   // TTGAMMA_NTUPLE_SKIMMER_SKIMMING_OPTIONS_H_
