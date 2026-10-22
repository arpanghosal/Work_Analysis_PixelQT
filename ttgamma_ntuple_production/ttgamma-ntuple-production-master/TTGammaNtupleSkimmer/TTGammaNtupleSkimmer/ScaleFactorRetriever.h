#ifndef TTGAMMA_SKIMMING_PACKAGE_SF_CALCULATOR 

#define TTGAMMA_SKIMMING_PACKAGE_SF_CALCULATOR 

#include "TFile.h"
#include "TH1.h"
#include "TH2.h"

#include <iostream>
#include <vector>
#include <TH1F.h>

namespace TTGammaNtupleSkimmer{
struct EfakePhoton{
	double pt_min= 0;
	double pt_max= 0;
	double eta_min= 0;
	double eta_max= 0;

	int conversion_type = 0;

	double scale_factor = 1.0;
	double scale_factor_error = 0.0;

};
struct HfakePhoton{
	double pt_min= 0;
	double pt_max= 0;
	double eta_min= 0;
	double eta_max= 0;

	int conversion_type = 0;

	double scale_factor = 1.0;
	double scale_factor_error = 0.0;

};
/// A class which helps to calculate efake scale factor
/// from a root file
class EFakeScaleFactorRetriever{
public:
	EFakeScaleFactorRetriever(const TString& fileUnconv, const TString& fileConv);
	~EFakeScaleFactorRetriever();
	void clear();

	//return scale factor corresponding to eta and pt bins
	std::tuple<double, double> getEfakeSF(double eta, double pt, int conversion_type);
	// print scale factor values
	void printEfakeSF();

private:
	std::vector<EfakePhoton> *m_efakePhotonConverted; // storing pt, eta, SF info in a vector
	std::vector<EfakePhoton> *m_efakePhotonUnconverted; // storing pt, eta, SF info in a vector
	TFile* fileNameConverted{nullptr};
	TFile* fileNameUnconverted{nullptr};

};

/// A class which helps to calculate hfake scale factor
/// from a root file
class HFakeScaleFactorRetriever{
public:
	HFakeScaleFactorRetriever(const std::string& file);
	~HFakeScaleFactorRetriever();
	void clear();

	//return scale factor corresponding to eta and pt bins
	std::tuple<double, double> getHfakeSF(double eta, double pt, int conversion_type);
	// print scale factor values
	void printHfakeSF();

private:
	std::vector<HfakePhoton> *m_hfakePhotonUnconverted; // storing pt, eta, SF info in a vector
	std::vector<HfakePhoton> *m_hfakePhotonConverted;
	TFile* fileName{nullptr};

};

/**
 * @brief class for retrieving and applying lepton fake efficiencies on loose data
 *
 */
 class LeptonFakeEfficiencyRetriever{
 public:
  LeptonFakeEfficiencyRetriever(const std::string& realEffRootFileElelctron,
                                const std::string& realEffRootFileMuon,
                                const std::string& fakeEffRootFileElectron,
                                const std::string& fakeEffRootFileMuon );
  ~LeptonFakeEfficiencyRetriever();

  /**
   * @brief calculate event weight from the efficiency
   *
   */
  Double_t getEventWeight(bool isElectron, bool isMuon, bool isTight, float eta, float pt );

  /**
   * @brief set obj names in root file
   */
  TString m_realHistName = "real_pt_eta_2D_Tight";
  TString m_fakeHistName = "lfake_pt_eta_2D_Tight";
   /**
    * @brief setter for m_realHistName and m_fakeHistName
    */
  void setRealHistName(TString histName){this->m_realHistName = histName;}
  void setFakeHistName(TString histName){this->m_fakeHistName = histName;}

  /**
   * @brief connect the m_realEffi... to root histo objects
   */
  void getHistos();

 private:
  TH1F* m_realEfficiency2D_el = nullptr;
  TH1F* m_realEfficiency2D_mu = nullptr;

  TH1F* m_fakeEfficiency2D_el = nullptr;
  TH1F* m_fakeEfficiency2D_mu = nullptr;

  // file ptrs
  TFile* realEffRootFileEl = nullptr;
  TFile* realEffRootFileMu = nullptr;
  TFile* fakeEffRootFileEl = nullptr;
  TFile* fakeEffRootFileMu = nullptr;
 };
}

#endif // TTGAMMA_SKIMMING_PACKAGE_SF_CALCULATOR 

