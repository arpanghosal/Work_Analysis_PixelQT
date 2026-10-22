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

double mt(double lep_pt, double met_met, double lep_phi, double met_phi){
  return sqrt(2*lep_pt*met_met*(1-cos(lep_phi-met_phi)))/1000.;
}

double dPhi(double mt, double lep_pt, double met){
  return acos(1.-pow(mt,2)/(2*lep_pt*met));
}

bool isMC(const std::vector<std::string>& file_list){
  TFile* file = TFile::Open(file_list.at(0).c_str(), "READ");
  TTree* treeNominal = (TTree*)file->Get("nominal");
  UInt_t mcChannelNumber;
  treeNominal->SetBranchAddress("mcChannelNumber", &mcChannelNumber);
  treeNominal->GetEntry(0);
  if(mcChannelNumber == 0)
  {
    return false;
  }
  else{
    return true;
  }
}
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
  bool is_mc = isMC(options.in);

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
  //path to Lepton fake efficiency maps
  std::string effRealElPath_nominal = "/eos/user/b/bmondal/tty/lfakeEff/pt_eta_2D_el_70.root";
  std::string effRealMuPath_nominal = "/eos/user/b/bmondal/tty/lfakeEff/pt_eta_2D_mu_70.root";
  std::string effFakeElPath_nominal = "/eos/user/b/bmondal/tty/lfakeEff/pt_eta_2D_el_70.root";
  std::string effFakeMuPath_nominal = "/eos/user/b/bmondal/tty/lfakeEff/pt_eta_2D_mu_70.root";
  // pt_mt var
  std::string effRealElPath_syst1 = "/eos/user/b/bmondal/tty/lfakeEff/pt_mt_2D_el_70.root";
  std::string effRealMuPath_syst1 = "/eos/user/b/bmondal/tty/lfakeEff/pt_mt_2D_mu_70.root";
  std::string effFakeElPath_syst1 = "/eos/user/b/bmondal/tty/lfakeEff/pt_mt_2D_el_70.root";
  std::string effFakeMuPath_syst1 = "/eos/user/b/bmondal/tty/lfakeEff/pt_mt_2D_mu_70.root";
  // pt_phi var
  std::string effRealElPath_syst2 = "/eos/user/b/bmondal/tty/lfakeEff/pt_phi_2D_el_70.root";
  std::string effRealMuPath_syst2 = "/eos/user/b/bmondal/tty/lfakeEff/pt_phi_2D_mu_70.root";
  std::string effFakeElPath_syst2 = "/eos/user/b/bmondal/tty/lfakeEff/pt_phi_2D_el_70.root";
  std::string effFakeMuPath_syst2 = "/eos/user/b/bmondal/tty/lfakeEff/pt_phi_2D_mu_70.root";

  LeptonFakeEfficiencyRetriever lepFakeEffRetr_nominal(effRealElPath_nominal, effRealMuPath_nominal,effFakeElPath_nominal, effFakeMuPath_nominal);
  lepFakeEffRetr_nominal.setRealHistName("real_pt_eta_2D_Tight");
  lepFakeEffRetr_nominal.setFakeHistName("lfake_pt_eta_2D_Tight");
  lepFakeEffRetr_nominal.getHistos();

  LeptonFakeEfficiencyRetriever lepFakeEffRetr_syst1(effRealElPath_syst1, effRealMuPath_syst1,effFakeElPath_syst1, effFakeMuPath_syst1);
  lepFakeEffRetr_syst1.setRealHistName("real_pt_mt_2D_Tight");
  lepFakeEffRetr_syst1.setFakeHistName("lfake_pt_mt_2D_Tight");
  lepFakeEffRetr_syst1.getHistos();

  LeptonFakeEfficiencyRetriever lepFakeEffRetr_syst2(effRealElPath_syst2, effRealMuPath_syst2,effFakeElPath_syst2, effFakeMuPath_syst2);
  lepFakeEffRetr_syst2.setRealHistName("real_pt_phi_2D_Tight");
  lepFakeEffRetr_syst2.setFakeHistName("lfake_pt_phi_2D_Tight");
  lepFakeEffRetr_syst2.getHistos();

  // Retrieve scale factors info.
  TString efakeScaleFactorPathUnconv = "/eos/user/b/bmondal/tty/efakeSF/unconverted_finalSF.root"; // path to the efake SF root file
  TString efakeScaleFactorPathConv = "/eos/user/b/bmondal/tty/efakeSF/converted_finalSF.root";// path to the efake SF root file
  std::string hfakeScaleFactorPath = "/eos/user/b/bmondal/tty/hfakeSF/HFakeScaleFactors_Btag70.root"; // path to the hfake SF root file
  EFakeScaleFactorRetriever efake_scalefactor{efakeScaleFactorPathUnconv, efakeScaleFactorPathConv};
  if(options.debug) efake_scalefactor.printEfakeSF();
  HFakeScaleFactorRetriever hfake_scalefactor{hfakeScaleFactorPath.c_str()};
  if(options.debug) hfake_scalefactor.printHfakeSF();

  // These branches will be added to the output as well.
  std::vector<int> ph_medium_index{};
  double efakeSF;
	double efakeSF_Up;
	double efakeSF_Down;
  double hfakeSF;
	double hfakeSF_Up;
	double hfakeSF_Down;
  Double_t weight_leptonFake_nominal;
  Double_t weight_leptonFake_Up;
  Double_t weight_leptonFake_Down;

  // These branches need to be connected to check duplicate events.
  unsigned int run_number;
  unsigned long long event_number;

  // These branches need to be connected for calculations.
  std::vector<char>* ph_id_tight_relaxed{};
  std::vector<float>* ph_pt{};
  std::vector<float>* ph_eta{};
  std::vector<float>* el_pt{};
  std::vector<float>* el_eta{};
  std::vector<float>* el_phi{};
  std::vector<char>* el_isTight{};
  std::vector<float>* mu_pt{};
  std::vector<float>* mu_eta{};
  std::vector<float>* mu_phi{};
  std::vector<char>* mu_isTight{};
  std::vector<float>* ph_drlph_closest{};
  float event_detall{0};
  float event_dphill{0};
  std::vector<float>* ph_true_pt{};
  std::vector<int>* ph_true_category{};
  std::vector<int>* ph_conversion_type{};
  float event_ST{};
  float met_met{};
  float met_phi{};
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
    chain.SetBranchAddress("el_pt", &el_pt);
    chain.SetBranchAddress("el_eta", &el_eta);
    chain.SetBranchAddress("el_phi", &el_phi);
    if(entry == "nominal_Loose") chain.SetBranchAddress("el_isTight", &el_isTight);
    chain.SetBranchAddress("mu_pt", &mu_pt);
    chain.SetBranchAddress("mu_eta", &mu_eta);
    chain.SetBranchAddress("mu_phi", &mu_phi);
    if(entry == "nominal_Loose") chain.SetBranchAddress("mu_isTight", &mu_isTight);
    chain.SetBranchAddress("ph_drlph_closest", &ph_drlph_closest);
    chain.SetBranchAddress("event_detall", &event_detall);
    chain.SetBranchAddress("event_dphill", &event_dphill);
    chain.SetBranchAddress("ph_true_pt", &ph_true_pt);
    chain.SetBranchAddress("ph_true_category", &ph_true_category);
    chain.SetBranchAddress("ph_conversion_type", &ph_conversion_type);
    chain.SetBranchAddress("event_ST", &event_ST);
    chain.SetBranchAddress("met_met", &met_met);
    chain.SetBranchAddress("met_phi", &met_phi);
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
    skimmed_tree->Branch("efakeSF_Up", &efakeSF_Up);
    skimmed_tree->Branch("efakeSF_Down", &efakeSF_Down);
    skimmed_tree->Branch("hfakeSF", &hfakeSF);
    skimmed_tree->Branch("hfakeSF_Up", &hfakeSF_Up);
    skimmed_tree->Branch("hfakeSF_Down", &hfakeSF_Down);
    skimmed_tree->Branch("weight_leptonFake_nominal", &weight_leptonFake_nominal);
    skimmed_tree->Branch("weight_leptonFake_Up", &weight_leptonFake_Up);
    skimmed_tree->Branch("weight_leptonFake_Down", &weight_leptonFake_Down);
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
      double tmp_efakeSF = 1;
      double tmp_efakeSFError = 0;
      double tmp_hfakeSF = 1;
      double tmp_hfakeSFError = 0;

      for (size_t i = 0; i < ph_eta->size(); ++i) {
				//only apply efake and hfake sf for mc samples
      	// efake sf
      	if( ph_true_category->at(i) == int(top::ttgamma::PhotonCategory::Efake) || (ph_true_category->at(i)) == int(top::ttgamma::PhotonCategory::EfakeDeltaR))
      	{
      	  std::tuple<double, double> efake_sf = efake_scalefactor.getEfakeSF(std::abs(ph_eta->at(i)), ph_pt->at(i)/1000, ph_conversion_type->at(i)); // 1000 in the argument to convert to GeV
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
      
				efakeSF = tmp_efakeSF;
				efakeSF_Up = tmp_efakeSF + tmp_efakeSFError;
				efakeSF_Down = tmp_efakeSF - tmp_efakeSFError;
				hfakeSF = tmp_hfakeSF;
				hfakeSF_Up = tmp_hfakeSF + tmp_hfakeSFError;
				hfakeSF_Down = tmp_hfakeSF - tmp_hfakeSFError;
      }
      if(chain.FindBranch("jet_truthflav")!=0){
       nTruthBJets=0;
       nTruthCJets=0;
       for( int i =0; i<event_njets; i++){
         if(jet_truthflav->at(i)==5){nTruthBJets++;}
         if(jet_truthflav->at(i)==4){nTruthCJets++;}
       }
      }
      // lepton fake event weight only for data and nominal_Loose branch
      if(!is_mc && (entry == "nominal_Loose")){
        // check whether the evnet contains el or mu
        /**
         * note:  you might see some abnormal naming between the lepFakeEffRetr obj and the weights.
         * this is just a quick way of renaming the systematics depending on the fake lepton contribution
         * coming from different parametrization. its fine.
         */
        if(el_pt->size() != 0){
          weight_leptonFake_Down = lepFakeEffRetr_nominal.getEventWeight(true, false, static_cast<bool>(el_isTight->at(0)), std::abs(el_eta->at(0)), el_pt->at(0)/1000);
        }
        if(mu_pt->size() != 0){
          weight_leptonFake_nominal = lepFakeEffRetr_nominal.getEventWeight(false, true, static_cast<bool>(mu_isTight->at(0)), std::abs(mu_eta->at(0)), mu_pt->at(0)/1000);
        }
	// syst1 (pt, mt)
        double mt_el = 0;
        if(el_pt->size() != 0) mt_el = mt(el_pt->at(0), met_met, el_phi->at(0), met_phi);
        double mt_mu = 0;
        if(mu_pt->size() != 0) mt_mu =  mt(mu_pt->at(0), met_met, mu_phi->at(0), met_phi);
	if(el_pt->size() != 0){
          weight_leptonFake_nominal = lepFakeEffRetr_syst1.getEventWeight(true, false, static_cast<bool>(el_isTight->at(0)), mt_el, el_pt->at(0)/1000);
        }
        if(mu_pt->size() != 0){
          weight_leptonFake_Up = lepFakeEffRetr_syst1.getEventWeight(false, true, static_cast<bool>(mu_isTight->at(0)), mt_mu, mu_pt->at(0)/1000);
        }
	// syst2 (pt, phi)
	if(el_pt->size() != 0){
          double dphi = dPhi(mt_el, el_pt->at(0)/1000, met_met/1000);
          weight_leptonFake_Up = lepFakeEffRetr_syst2.getEventWeight(true, false, static_cast<bool>(el_isTight->at(0)), dphi, el_pt->at(0)/1000);
        }
        if(mu_pt->size() != 0){
          double dphi = dPhi(mt_el, mu_pt->at(0)/1000, met_met/1000);
          weight_leptonFake_Down = lepFakeEffRetr_syst2.getEventWeight(false, true, static_cast<bool>(mu_isTight->at(0)), dphi, mu_pt->at(0)/1000);

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
