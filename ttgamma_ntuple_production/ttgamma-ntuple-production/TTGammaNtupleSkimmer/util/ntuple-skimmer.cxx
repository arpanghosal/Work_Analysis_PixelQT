#include "TTGammaNtupleSkimmer/SampleMetaData.h"
#include "TTGammaNtupleSkimmer/SkimmingOptions.h"
#include "TTGammaNtupleSkimmer/ScaleFactorRetriever.h"
#include "EventDuplicateChecker/EventDuplicateCheckerTool.h"
#include "PathResolver/PathResolver.h"
#include "TTGammaEventSaver/PhotonCategory.h"

#include <iostream>
#include <iomanip>
#include<tuple>

#include "TChain.h"
#include "TEntryList.h"
#include "TError.h"
#include "TFile.h"
#include "TH1D.h"

namespace {
/// Open the first file from a given file list, and retrieve
/// the list of all trees in it.
std::vector<std::string> get_list_of_trees(const std::vector<std::string>& file_list) {
  const auto& file = TFile::Open(file_list.at(0).c_str(), "READ");
  std::vector<std::string> trees;
  TIter nextkey(file->GetListOfKeys());
  TKey *key = nullptr;
  while ((key = static_cast<TKey*>(nextkey()))) {
    TObject *obj = key->ReadObj();
    if (!obj->IsA()->InheritsFrom(TTree::Class())) continue;
    trees.push_back(obj->GetName());
  }
  file->Close();
  return trees;
}

/// Prepend the file protocol and the eos domain to make sure
/// we're using the correct way to access ROOT files. If not
/// prepended, we run into file limits through the eos fuse (i.e.
/// freezes when too many files are opened simultaneously).
void prepend_eos_domain(std::vector<std::string>& file_list) {
  auto replace_first_occurence = [] (std::string& s, const std::string& to_replace, const std::string& replace_with) {
    auto pos = s.find(to_replace);
    if (pos == std::string::npos) return s;
    return s.replace(pos, to_replace.length(), replace_with);
  };

  for (auto& file : file_list) {
    file = replace_first_occurence(file, "/eos/atlas/", "root://eosatlas//eos/atlas/");
    file = replace_first_occurence(file, "/eos/user/", "root://eosuser//eos/user/");
  }
}
}  // namespace




int main(int argc, char* argv[]) {
  using namespace TTGammaNtupleSkimmer;

  // Set the ignore level to not spit out any warnings.
  gErrorIgnoreLevel = 2001;

  // Parse the command line options and print them.
  SkimmingOptions options = get_skim_opts(argc, argv);
  print_config(options);

  // Make sure we add the eos domain in front of all file names.
  prepend_eos_domain(options.in);

  const auto& tree_list = get_list_of_trees(options.in);

  // Store the luminosity (given via cmd options).
  auto event_lumi = options.lumi;

  // Retrieve sample meta info.
  SampleMetaData meta{options.in};
  auto event_norm = meta.getXSec() * options.kfactor;

  std::vector<float> event_norm_vector{};
  event_norm_vector.resize(meta.getTotalEventsWeightedVector()->size());
  for (size_t i = 0; i < event_norm_vector.size(); ++i) {
    event_norm_vector.at(i) = event_norm / meta.getTotalEventsWeightedVector()->at(i);
  }
  event_norm /= meta.getTotalEventsWeighted();

  // Retrieve scale factors info.
  std::string efakeScaleFactorPath = "/eos/user/b/bmondal/tty/efakeSF/finalSF.root";
  std::string hfakeScaleFactorPath = "/eos/user/b/bmondal/tty/hfakeSF/HFakeScaleFactors_Btag70.root";
  EFakeScaleFactorRetriever efake_scalefactor{efakeScaleFactorPath.c_str()};
  if(options.debug) efake_scalefactor.printEfakeSF();
  HFakeScaleFactorRetriever hfake_scalefactor{hfakeScaleFactorPath.c_str()};
  if(options.debug) hfake_scalefactor.printHfakeSF();


  // These branches will be added to the output as well.
  std::vector<int> ph_medium_index{};
  std::vector<double> efakeSF;
  std::vector<double> efakeSFError;
  std::vector<double> hfakeSF;
  std::vector<double> hfakeSFError;


  // These branches need to be connected to check duplicate events.
  unsigned int run_number;
  unsigned long long event_number;

  // These branches need to be connected for calculations.
  std::vector<char>* ph_id_tight_relaxed{};
  std::vector<float>* ph_pt{};
  std::vector<float>* ph_eta{};
  std::vector<float>* ph_drlph_closest{};
  float event_detall{0};
  float event_dphill{0};
  std::vector<float>* ph_true_pt{};
  std::vector<int>* ph_true_category{};
  std::vector<int>* ph_conversion_type{};
  float event_ST{};
  std::vector<int>* jet_truthflav{};
  int event_njets{};
  int nTruthBJets{};
  int nTruthCJets{};

  TEntryList *elist = nullptr;
  auto output_file = TFile::Open(options.out.c_str(), "RECREATE");

  for (const auto& entry : tree_list) {
    // Skip all trees that contain non-reconstruction info.
    bool is_reco_tree{true};
    bool is_truth_tree{false};
    if (entry == "AnalysisTracking") is_reco_tree = false;
    if (entry == "sumWeights") is_reco_tree = false;
    if (options.skip_systematics && entry != "nominal") is_reco_tree = false;
    if (entry == "truth") {
      is_reco_tree = false;
      is_truth_tree = true;
    }

    // Skip all trees that are neither reco nor truth.
    if (!is_reco_tree && !is_truth_tree) {
      std::cout << "Skipping tree \"" << entry << "\" ..." << std::endl;
      continue;
    }

    // Skip the truth tree as well if not requested explicitly.
    if (!options.process_truth && is_truth_tree) {
      std::cout << "Skipping tree \"" << entry << "\", because option [--process-truth] was not set ..." << std::endl;
      continue;
    }

    TChain chain{entry.c_str()};
    for (auto& file : options.in) {
      chain.Add(file.c_str());
    }

    if (options.process_truth && entry == "truth") {
      elist->Reset();
      chain.Draw(">>elist", "event_is_dilepton = 1", "entrylist");
      elist = static_cast<TEntryList*>(gDirectory->Get("elist"));
      chain.SetEntryList(elist);
      auto skimmed_tree = chain.CloneTree(0);
      skimmed_tree->SetDirectory(output_file);
      skimmed_tree->Branch("event_norm", &event_norm);
      skimmed_tree->Branch("event_norm_vector", &event_norm_vector);
      skimmed_tree->SetAutoFlush(500);
      const auto& n_selected = elist->GetN();
      for (Long64_t event = 0; event < n_selected; ++event) {
        chain.GetEntry(chain.GetEntryNumber(event));
        skimmed_tree->Fill();
      }

      // Delete the current file to not get into ownership conflicts.
      delete chain.GetCurrentFile();

      // Delete entry list, otherwise it goes into the output.
      elist->Reset();

      // Now continue to the next entry in the files.
      continue;
    }

    // Deactivate all branches in the slim list.
    for (const auto& branch : options.slimlist) {
      chain.SetBranchStatus(branch.c_str(), 0);
    }
    // Reactivate those from the veto list.
    for (const auto& branch : options.slim_exclude) {
      chain.SetBranchStatus(branch.c_str(), 1);
    }

    // If flag "--remove-overlap" is given, remove all events
    // that fall into the overlap region.
    if (options.remove_overlap) {
      options.cuts = options.cuts + " && event_in_overlap == 0";
    }

    // Apply event selection to the TChain and store list of
    // events in a TEntryList object.
    chain.Draw(">>elist", options.cuts.c_str(), "entrylist");
    elist = static_cast<TEntryList*>(gDirectory->Get("elist"));
    chain.SetEntryList(elist);

    chain.SetBranchAddress("runNumber", &run_number);
    chain.SetBranchAddress("eventNumber", &event_number);
    chain.SetBranchAddress("ph_id_tight_relaxed", &ph_id_tight_relaxed);
    chain.SetBranchAddress("ph_pt", &ph_pt);
    chain.SetBranchAddress("ph_eta", &ph_eta);
    chain.SetBranchAddress("ph_drlph_closest", &ph_drlph_closest);
    chain.SetBranchAddress("event_detall", &event_detall);
    chain.SetBranchAddress("event_dphill", &event_dphill);
    chain.SetBranchAddress("ph_true_pt", &ph_true_pt);
    chain.SetBranchAddress("ph_true_category", &ph_true_category);
    chain.SetBranchAddress("ph_conversion_type", &ph_conversion_type);
    chain.SetBranchAddress("event_ST", &event_ST);
    chain.SetBranchAddress("event_njets", &event_njets);
    if(chain.FindBranch("jet_truthflav")!=0){
      chain.SetBranchAddress("jet_truthflav", &jet_truthflav);
    }
    // Output some info about the event selection.
    const auto& n_total = chain.GetEntriesFast();
    if (n_total == 0) continue;
    const auto& n_selected = elist->GetN();
    std::cout << entry << ": Selecting " << n_selected;
    std::cout << " out of " << n_total << " raw MC events (";
    std::cout << std::setprecision(3);
    std::cout << 100. * n_selected / n_total << "%)" << std::endl;

    // Create a clone of the tree (with only meta data copied for
    // now). Below we loop over all entries of the TEntryList and
    // actually fill this clone.
    auto skimmed_tree = chain.CloneTree(0);
    skimmed_tree->SetDirectory(output_file);

    // Flush the tree buffer into the file automatically.
    skimmed_tree->SetAutoFlush(500);

    // Add these variables to the skimmed tree.
    skimmed_tree->Branch("event_lumi", &event_lumi, "event_lumi/F");
    skimmed_tree->Branch("event_norm", &event_norm, "event_norm/F");
    skimmed_tree->Branch("ph_medium_index", &ph_medium_index);
    //adding branches for scale factors
    skimmed_tree->Branch("efakeSF", &efakeSF);
    skimmed_tree->Branch("efakeSFError", &efakeSFError);
    skimmed_tree->Branch("hfakeSF", &hfakeSF);
    skimmed_tree->Branch("hfakeSFError", &hfakeSFError);
    //adding branches for truth jet flavour classification
    if(chain.FindBranch("jet_truthflav")!=0){
      skimmed_tree->Branch("nTruthBJets", &nTruthBJets); 
      skimmed_tree->Branch("nTruthCJets", &nTruthCJets);
    }



    // If the tree is 'nominal' and the event normalisation
    // values for shower and/or scale variation are non-zero,
    // add them to the tree. They are only extracted for certain
    // samples (see in SampleMetaData class for details).
    if (entry == "nominal" && !event_norm_vector.empty()) {
      skimmed_tree->Branch("event_norm_vector", &event_norm_vector);
    }

    // Implement ASG's event duplicate checker and skip any event
    // that is stored multiple times in the same tree.
    asg::EventDuplicateCheckerTool duplicate_checker;
    duplicate_checker.initialize().ignore();
    unsigned int duplicate_counter{0};

    // Go through all selected events and fill the new tree.
    for (Long64_t event = 0; event < n_selected; ++event) {
      chain.GetEntry(chain.GetEntryNumber(event));

      // Skip if duplicate (i.e. event we have looked at before).
      if (duplicate_checker.isDuplicate(run_number, event_number)) {
        duplicate_counter++;
        continue;
      }

      ph_medium_index.resize(0);
      for (size_t i = 0; i < ph_pt->size(); ++i) {
        // If the photon passes the tight-relaxed ID and has p_T
        // > 20 GeV, add its index to the ph_medium_index
        // variable (i.e. passed medium ID criteria).
        if (static_cast<bool>(ph_id_tight_relaxed->at(i)) && ph_pt->at(i) > 20e3) {
          ph_medium_index.emplace_back(i);
        }
      }

      //Add efkae Scale Factor
      efakeSF.resize(0);
      efakeSFError.resize(0);
      hfakeSF.resize(0);
      hfakeSFError.resize(0);

      double tmp_efakeSF = 1;
      double tmp_efakeSFError = 0;
      double tmp_hfakeSF = 1;
      double tmp_hfakeSFError = 0;

      for (size_t i = 0; i < ph_eta->size(); ++i) {
				//only apply efake and hfake sf for mc samples
      	// efake sf
      	if( ph_true_category->at(i) == int(top::ttgamma::PhotonCategory::Efake) || (ph_true_category->at(i)) == int(top::ttgamma::PhotonCategory::EfakeDeltaR))
      	{
      	  std::tuple<double, double> efake_sf = efake_scalefactor.getEfakeSF(std::abs(ph_eta->at(i)), ph_pt->at(i)/1000); // 1000 in the argument to convert to GeV
      	  tmp_efakeSF = std::get<0>(efake_sf);
      	  tmp_efakeSFError = std::get<1>(efake_sf);
      	}
      
      	// hfake sf
      	if( ph_true_category->at(i) == int(top::ttgamma::PhotonCategory::Hfake))
      	{
      	  std::tuple<double, double> hfake_sf = hfake_scalefactor.getHfakeSF(std::abs(ph_eta->at(i)), ph_pt->at(i)/1000, ph_conversion_type->at(i)); // 1000 in the argument to convert to GeV
      	  tmp_hfakeSF = std::get<0>(hfake_sf);
      	  tmp_hfakeSFError = std::get<1>(hfake_sf);
      	}
      
      	efakeSF.emplace_back(tmp_efakeSF);
      	efakeSFError.emplace_back(tmp_efakeSFError);
      	hfakeSF.emplace_back(tmp_hfakeSF);
      	hfakeSFError.emplace_back(tmp_hfakeSFError);
      }
      if(chain.FindBranch("jet_truthflav")!=0){
       nTruthBJets=0;
       nTruthCJets=0;
       for( int i =0; i<event_njets; i++){
         if(jet_truthflav->at(i)==5){nTruthBJets++;}
         if(jet_truthflav->at(i)==4){nTruthCJets++;}
       }
      }
      skimmed_tree->Fill();
    }

    std::cout << "Found duplicates: " << duplicate_counter << std::endl;

    // Delete the current file to not get into ownership conflicts.
    delete chain.GetCurrentFile();

    // Reset entry list, it resets the number of blocks to 0.
    elist->Reset();
  }
  // delete entry_list otherwise it will go to output
  delete elist;
  output_file->Write();
  output_file->Close();

  return 0;
}
