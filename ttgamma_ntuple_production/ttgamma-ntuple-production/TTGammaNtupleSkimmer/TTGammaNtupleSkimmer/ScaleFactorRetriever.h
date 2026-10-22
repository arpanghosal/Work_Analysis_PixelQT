#ifndef TTGAMMA_SKIMMING_PACKAGE_SF_CALCULATOR 

#define TTGAMMA_SKIMMING_PACKAGE_SF_CALCULATOR 

#include "TFile.h"

#include <iostream>
#include <vector>

namespace TTGammaNtupleSkimmer{
struct EfakePhoton{
	double pt_min= 0;
	double pt_max= 0;
	double eta_min= 0;
	double eta_max= 0;

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
	EFakeScaleFactorRetriever(const std::string& file);
	~EFakeScaleFactorRetriever();
	void clear();

	//return scale factor corresponding to eta and pt bins
	std::tuple<double, double> getEfakeSF(double eta, double pt);
	// print scale factor values
	void printEfakeSF();

private:
	std::vector<EfakePhoton> *m_efakePhoton; // storing pt, eta, SF info in a vector
	TFile* fileName{nullptr};

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

}

#endif // TTGAMMA_SKIMMING_PACKAGE_SF_CALCULATOR 

