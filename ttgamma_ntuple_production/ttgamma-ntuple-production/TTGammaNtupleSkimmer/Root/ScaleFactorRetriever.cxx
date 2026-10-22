#include "TTGammaNtupleSkimmer/ScaleFactorRetriever.h"

#include <iostream>

#include "TFile.h"
#include "TH2.h"


namespace TTGammaNtupleSkimmer{

//Takes input file, extracts the sf values and assigns into arrays
EFakeScaleFactorRetriever::EFakeScaleFactorRetriever(const std::string& file)
	: m_efakePhoton(new std::vector<EfakePhoton>)
	{

	// Load the root file
	fileName = new TFile(file.c_str());
	if(!(fileName->IsOpen())){std::cout<<"root file containing efake SF is not found"<<std::endl;}
	TH2F *h2sf = (TH2F*)fileName->Get("finalSF");


	//pt and eta bin range
	double pt_min[] = {20, 35, 45, 60};
	double pt_max[] = {35, 45, 60, 1000};
	double eta_min[] = {0, 0.5, 1.0, 1.37, 1.52, 2.0};
	double eta_max[] = {0.5, 1.0, 1.37, 1.52, 2.0, 2.37};
	for(int eta = 0; eta < 3; eta++){
			for(int pt = 0; pt < 4; pt++){

				EfakePhoton ph;
				ph.pt_min = pt_min[pt];
				ph.pt_max = pt_max[pt];
				ph.eta_min = eta_min[eta];
				ph.eta_max = eta_max[eta];
				ph.scale_factor = h2sf->GetBinContent(eta+1, pt+1);
				ph.scale_factor_error = h2sf->GetBinError(eta+1, pt+1);

				m_efakePhoton->push_back(ph);
		}

	}
	//Avoiding crack region in the 2D scale factor histo

	for(int eta = 3; eta < 5; eta++){
			for(int pt= 0; pt < 4; pt++){

				EfakePhoton ph;
				ph.pt_min = pt_min[pt];
				ph.pt_max = pt_max[pt];
				ph.eta_min = eta_min[eta+1];
				ph.eta_max = eta_max[eta+1];
				ph.scale_factor = h2sf->GetBinContent(eta+2, pt+1);
				ph.scale_factor_error = h2sf->GetBinError(eta+2, pt+1);

				m_efakePhoton->push_back(ph);

		}

	}



}

EFakeScaleFactorRetriever::~EFakeScaleFactorRetriever(){
	this->clear();
	delete m_efakePhoton;
	delete fileName;

}

void EFakeScaleFactorRetriever::clear(){
	m_efakePhoton->clear();
}

void EFakeScaleFactorRetriever::printEfakeSF(){
	std::cout<< "size of the m_efakePhoton: "<<m_efakePhoton->size()<<std::endl;

	for( const EfakePhoton& m_efakePhoton_ptr : *m_efakePhoton)
	std::cout<<"pt_min: "<<m_efakePhoton_ptr.pt_min<<" pt_max: "<<m_efakePhoton_ptr.pt_max<<" et_min: "<<m_efakePhoton_ptr.eta_min<<" eta_max: "<<m_efakePhoton_ptr.eta_max<<" efakeSF: "<<m_efakePhoton_ptr.scale_factor<<" efakeSF error: "<<m_efakePhoton_ptr.scale_factor_error<<std::endl;
}

std::tuple<double, double> EFakeScaleFactorRetriever::getEfakeSF(double eta, double pt){

	std::tuple<double, double> tmp_sf(1.0, 0.0);
	// depending on pt and eta return the scale factor
	for(const EfakePhoton& ph_ptr : *m_efakePhoton)
	{
		if( (eta >= ph_ptr.eta_min) && (eta < ph_ptr.eta_max) && (pt >= ph_ptr.pt_min) && (pt < ph_ptr.pt_max) ){
			tmp_sf = std::make_tuple(ph_ptr.scale_factor, ph_ptr.scale_factor_error);

		}

	}
	return tmp_sf;

}

// hfake
HFakeScaleFactorRetriever::HFakeScaleFactorRetriever(const std::string& file)
	: m_hfakePhotonUnconverted(new std::vector<HfakePhoton>)
	, m_hfakePhotonConverted(new std::vector<HfakePhoton>)
	{

	// Load the root file
	fileName = new TFile(file.c_str());
	if(!(fileName->IsOpen())){std::cout<<"root file containing hfake SF is not found"<<std::endl;}

	TH2F *h2sf_unconverted = (TH2F*)fileName->Get("SF_finer_binning_unconverted");
	TH2F *h2sf_converted = (TH2F*)fileName->Get("SF_finer_binning_converted");


  	//pt and eta bin range
  	double pt_min[] = {20, 50};
  	double pt_max[] = {50, 500};
  	double eta_min[] = {0, 0.3, 0.6, 1.37, 1.52};
  	double eta_max[] = {0.3, 0.6, 1.37, 1.52, 2.37};
  	for(int eta = 0; eta < 3; eta++){
  			for(int pt = 0; pt < 2; pt++){

				HfakePhoton ph_unconv;
				HfakePhoton ph_conv;
				ph_unconv.pt_min = pt_min[pt];
				ph_unconv.pt_max = pt_max[pt];
				ph_unconv.eta_min = eta_min[eta];
				ph_unconv.eta_max = eta_max[eta];
				ph_unconv.scale_factor = h2sf_unconverted->GetBinContent(eta+1, pt+1);
				ph_unconv.scale_factor_error = h2sf_unconverted->GetBinError(eta+1, pt+1);
				ph_conv.pt_min = pt_min[pt];
				ph_conv.pt_max = pt_max[pt];
				ph_conv.eta_min = eta_min[eta];
				ph_conv.eta_max = eta_max[eta];
				ph_conv.scale_factor = h2sf_converted->GetBinContent(eta+1, pt+1);
				ph_conv.scale_factor_error = h2sf_converted->GetBinError(eta+1, pt+1);


				m_hfakePhotonUnconverted->push_back(ph_unconv);
				m_hfakePhotonConverted->push_back(ph_conv);
		}

	}
	//Avoiding crack region in the 2D scale factor histo

  	for(int eta = 3; eta < 4; eta++){
  			for(int pt= 0; pt < 2; pt++){

				HfakePhoton ph_unconv;
				HfakePhoton ph_conv;
				ph_unconv.pt_min = pt_min[pt];
				ph_unconv.pt_max = pt_max[pt];
				ph_unconv.eta_min = eta_min[eta+1];
				ph_unconv.eta_max = eta_max[eta+1];
				ph_unconv.scale_factor = h2sf_unconverted->GetBinContent(eta+2, pt+1);
				ph_unconv.scale_factor_error = h2sf_unconverted->GetBinError(eta+2, pt+1);
				ph_conv.pt_min = pt_min[pt];
				ph_conv.pt_max = pt_max[pt];
				ph_conv.eta_min = eta_min[eta+1];
				ph_conv.eta_max = eta_max[eta+1];
				ph_conv.scale_factor = h2sf_converted->GetBinContent(eta+2, pt+1);
				ph_conv.scale_factor_error = h2sf_converted->GetBinError(eta+2, pt+1);

				m_hfakePhotonUnconverted->push_back(ph_unconv);
				m_hfakePhotonConverted->push_back(ph_conv);

		}
	}
}

HFakeScaleFactorRetriever::~HFakeScaleFactorRetriever(){
	this->clear();
	delete m_hfakePhotonUnconverted;
	delete m_hfakePhotonConverted;
	delete fileName;

}

void HFakeScaleFactorRetriever::clear(){
	m_hfakePhotonUnconverted->clear();
	m_hfakePhotonConverted->clear();
}


void HFakeScaleFactorRetriever::printHfakeSF(){
	std::cout<< "size of the m_hfakePhotonUnconverted: "<<m_hfakePhotonUnconverted->size()<<std::endl;

	for( const HfakePhoton& m_hfakePhotonUnconv_ptr : *m_hfakePhotonUnconverted)
  	std::cout<<"pt [ "<<m_hfakePhotonUnconv_ptr.pt_min<<" , "<<m_hfakePhotonUnconv_ptr.pt_max<<" ], "<<" eta [ "<<m_hfakePhotonUnconv_ptr.eta_min<<" , "<<m_hfakePhotonUnconv_ptr.eta_max<<" ]: "<<" hfakeSF: "<<m_hfakePhotonUnconv_ptr.scale_factor<<" hfakeSF error: "<<m_hfakePhotonUnconv_ptr.scale_factor_error<<std::endl;


	std::cout<< "size of the m_hfakePhotonConverted: "<<m_hfakePhotonConverted->size()<<std::endl;
for( const HfakePhoton& m_hfakePhotonConv_ptr : *m_hfakePhotonConverted)
	std::cout<<"pt [ "<<m_hfakePhotonConv_ptr.pt_min<<" , "<<m_hfakePhotonConv_ptr.pt_max<<" ], "<<" eta [ "<<m_hfakePhotonConv_ptr.eta_min<<" , "<<m_hfakePhotonConv_ptr.eta_max<<" ]: "<<" hfakeSF: "<<m_hfakePhotonConv_ptr.scale_factor<<" hfakeSF error: "<<m_hfakePhotonConv_ptr.scale_factor_error<<std::endl;


}

std::tuple<double, double> HFakeScaleFactorRetriever::getHfakeSF(double eta, double pt, int conversion_type){

	std::tuple<double, double> tmp_sf(1.0, 0.0);
	if(conversion_type == 0)
	{
	// depending on pt and eta return the scale factor
	for(const HfakePhoton& ph_ptr : *m_hfakePhotonUnconverted)
	{
		if( (eta >= ph_ptr.eta_min) && (eta < ph_ptr.eta_max) && (pt >= ph_ptr.pt_min) && (pt < ph_ptr.pt_max) ){
			tmp_sf = std::make_tuple(ph_ptr.scale_factor, ph_ptr.scale_factor_error);

		}
	}
	}

	if(conversion_type != 0)
	{
	// depending on pt and eta return the scale factor
	for(const HfakePhoton& ph_ptr : *m_hfakePhotonConverted)
	{
		if( (eta >= ph_ptr.eta_min) && (eta < ph_ptr.eta_max) && (pt >= ph_ptr.pt_min) && (pt < ph_ptr.pt_max) ){
			tmp_sf = std::make_tuple(ph_ptr.scale_factor, ph_ptr.scale_factor_error);

		}
	}
	}

	return tmp_sf;

}

}


