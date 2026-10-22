#include <fstream>
#include <iostream>

#include <boost/program_options.hpp>

#include "TTGammaNtupleSkimmer/SkimmingOptions.h"

namespace TTGammaNtupleSkimmer {
// ===============================================
SkimmingOptions get_skim_opts(int argc, char* argv[]) {
  namespace po = boost::program_options;

  std::vector<std::string> slimlist;
  std::vector<std::string> slim_exclude;
  bool flag_process_truth{false};
  bool flag_remove_overlap{false};
  bool flag_skip_systematics{false};
  bool flag_debug{false};

  // Options allowed in config file and command line.
  po::options_description config_file_opts("Options to be provided via command line or config file");
  config_file_opts.add_options()
    ("input,i", po::value<std::vector<std::string> >()->composing(), "Input file(s) to be skimmed/slimmed. This option can be provided multiple times.")
    ("output,o", po::value<std::string>()->default_value("output.root"), "Output file into which the skimmed/slimmed trees are saved.")
    ("cuts,c", po::value<std::string>()->default_value(""), "Cut string for event selection, e.g. \"event_njets >= 4 && event_nbjets77 >=1\".")
    ("kfactor", po::value<float>()->default_value(1.), "K-factor to be used for the process (instead of AMI value).")
    ("luminosity", po::value<float>()->default_value(79888.36), "Luminosity to be injected into the trees (in pb).")
    ("slim-branch", po::value<std::vector<std::string> >(&slimlist), "Branch(es) to be slimmed (supports wildcards), e.g. \"el_true_*\".")
    ("slim-exclude", po::value<std::vector<std::string> >(&slim_exclude), "Branch(es) vetoed during slimming (supports wildcards), e.g. \"el_true_pt\".")
    ("remove-overlap", po::bool_switch(&flag_remove_overlap), "Remove events from the sample overlap region.")
    ("process-truth", po::bool_switch(&flag_process_truth), "Process the truth tree in the skimming.")
    ("skip-systematics", po::bool_switch(&flag_skip_systematics), "Skip systematic trees in the skimming.")
    ("debug", po::bool_switch(&flag_debug), " print debug messages.")
    ;

  // Options only used in the command line.
  po::options_description cmd_line_opts("Command line options");
  cmd_line_opts.add_options()
    ("config-file", po::value<std::string>(), "configuration file")
    ("help,h", "produce help message")
    ;

  po::options_description all_opts("Allowed options");
  all_opts.add(config_file_opts).add(cmd_line_opts);

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(all_opts).run(), vm);
  po::notify(vm);

  // If config file provided via command line, read that as well.
  if (vm.count("config-file")) {
    std::ifstream ifs(vm["config-file"].as<std::string>().c_str());
    if (!ifs) {
      std::cerr << "Could not open config file. Exiting ..." << std::endl;
      exit(1);
    } else {
      po::store(po::parse_config_file(ifs, config_file_opts), vm);
      notify(vm);
    }
  }

  if (vm.count("help")) {
    std::cout << all_opts << std::endl;
    exit(1);
  }

  if (!vm.count("input")) {
    std::cerr << "No input files provided. Exiting ..." << std::endl;
    std::cout << all_opts << std::endl;
    exit(1);
  }

  SkimmingOptions opts;
  opts.in = vm["input"].as<std::vector<std::string> >();
  opts.out = vm["output"].as<std::string>();
  opts.cuts = vm["cuts"].as<std::string>();
  opts.kfactor = vm["kfactor"].as<float>();
  opts.lumi = vm["luminosity"].as<float>();
  opts.slimlist = slimlist;
  opts.slim_exclude= slim_exclude;
  opts.remove_overlap = flag_remove_overlap;
  opts.process_truth = flag_process_truth;
  opts.skip_systematics = flag_skip_systematics;
  opts.debug= flag_debug;

  return opts;
}

// ===============================================
void print_config(const SkimmingOptions& opts) {
  std::cout << " ========================= TTGammaNtupleSkimmer =========================" << std::endl;
  std::cout << "Using the following options:" << std::endl;
  std::cout << "  - k-factor: \"" << opts.kfactor << "\"" << std::endl;
  std::cout << "  - luminosity: \"" << opts.lumi << "\"" << std::endl;
  std::cout << "  - cut string: \"" << opts.cuts << "\"" << std::endl;
  std::cout << "  - remove overlap: \"" << opts.remove_overlap << "\"" << std::endl;
  std::cout << "  - slimmed branches:" << std::endl;
  for (const auto& branch : opts.slimlist) {
    std::cout << "      - \"" << branch << "\"" << std::endl;
  }
  std::cout << "  - branches vetoed during slimming:" << std::endl;
  for (const auto& branch : opts.slim_exclude) {
    std::cout << "      - \"" << branch << "\"" << std::endl;
  }
  std::cout << "  - output file: \"" << opts.out << "\"" << std::endl;
  std::cout << "  - list of input files:" << std::endl;
  for (const auto& file : opts.in) {
    std::cout << "      - \"" << file << "\"" << std::endl;
  }
  std::cout << " ========================= TTGammaNtupleSkimmer =========================" << std::endl;
  std::cout << std::endl << std::endl;
}
}  // namespace TTGammaNtupleSkimmer
