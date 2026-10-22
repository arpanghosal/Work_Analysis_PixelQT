#include "TTGammaNtupleSkimmer/ScaleFactorRetriever.h"

#include <iostream>

#include "TFile.h"
#include "TH2.h"


namespace TTGammaNtupleSkimmer{

//Takes input file, extracts the sf values and assigns into arrays
EFakeScaleFactorRetriever::EFakeScaleFactorRetriever(const TString& fileUnconv, const TString& fileConv)
	: m_efakePhotonConverted(new std::vector<EfakePhoton>)
	, m_efakePhotonUnconverted(new std::vector<EfakePhoton>)
	{

	// Load the root file
	fileNameUnconverted = new TFile(fileUnconv);
	fileNameConverted = new TFile(fileConv);
	if(!(fileNameUnconverted->IsOpen())){std::cout<<"root file containing efake SF Unconverted is not found"<<std::endl;}
	if(!(fileNameConverted->IsOpen())){std::cout<<"root file containing efake SF Converted is not found"<<std::endl;}
	TH2F *h2sf_converted = (TH2F*)fileNameConverted->Get("finalSF");
	TH2F *h2sf_unconverted = (TH2F*)fileNameUnconverted->Get("finalSF");


	//pt and eta bin range
	double pt_min[] = {20, 35, 45};
	double pt_max[] = {35, 45, 1000};
	double eta_min[] = {0, 0.6, 1.37, 1.52, 1.8 };
	double eta_max[] = {0.6, 1.37, 1.52, 1.8, 2.37};
	for(int eta = 0; eta < 2; eta++){
			for(int pt = 0; pt < 3; pt++){

				EfakePhoton phConv;
				EfakePhoton phUnconv;
				//Unconverted
				phUnconv.pt_min = pt_min[pt];
				phUnconv.pt_max = pt_max[pt];
				phUnconv.eta_min = eta_min[eta];
				phUnconv.eta_max = eta_max[eta];
				phUnconv.scale_factor = h2sf_unconverted->GetBinContent(eta+1, pt+1);
				phUnconv.scale_factor_error = h2sf_unconverted->GetBinError(eta+1, pt+1);
				// Converted
				phConv.pt_min = pt_min[pt];
				phConv.pt_max = pt_max[pt];
				phConv.eta_min = eta_min[eta];
				phConv.eta_max = eta_max[eta];
				phConv.scale_factor = h2sf_converted->GetBinContent(eta+1, pt+1);
				phConv.scale_factor_error = h2sf_converted->GetBinError(eta+1, pt+1);

				m_efakePhotonConverted->push_back(phConv);
				m_efakePhotonUnconverted->push_back(phUnconv);
		}

	}
	//Avoiding crack region in the 2D scale factor histo

	for(int eta = 2; eta < 4; eta++){
			for(int pt= 0; pt < 3; pt++){

				EfakePhoton phConv;
				EfakePhoton phUnconv;
				phUnconv.pt_min = pt_min[pt];
				phUnconv.pt_max = pt_max[pt];
				phUnconv.eta_min = eta_min[eta+1];
				phUnconv.eta_max = eta_max[eta+1];
				phUnconv.scale_factor = h2sf_unconverted->GetBinContent(eta+2, pt+1);
				phUnconv.scale_factor_error = h2sf_unconverted->GetBinError(eta+2, pt+1);

				phConv.pt_min = pt_min[pt];
				phConv.pt_max = pt_max[pt];
				phConv.eta_min = eta_min[eta+1];
				phConv.eta_max = eta_max[eta+1];
				phConv.scale_factor = h2sf_converted->GetBinContent(eta+2, pt+1);
				phConv.scale_factor_error = h2sf_converted->GetBinError(eta+2, pt+1);

				m_efakePhotonConverted->push_back(phConv);
				m_efakePhotonUnconverted->push_back(phUnconv);
		}
	}
}

EFakeScaleFactorRetriever::~EFakeScaleFactorRetriever(){
	this->clear();
	delete m_efakePhotonUnconverted;
	delete m_efakePhotonConverted;
	delete fileNameUnconverted;
	delete fileNameConverted;

}

void EFakeScaleFactorRetriever::clear(){
	m_efakePhotonConverted->clear();
	m_efakePhotonUnconverted->clear();
}

void EFakeScaleFactorRetriever::printEfakeSF(){
	std::cout<< "size of the m_efakePhotonConverted: "<<m_efakePhotonConverted->size()<<std::endl;
	std::cout<< "size of the m_efakePhotonUnconverted: "<<m_efakePhotonUnconverted->size()<<std::endl;

	std::cout<<" ================== Converted photon SF =============================" <<std::endl;
	for( const EfakePhoton& m_efakePhoton_ptr : *m_efakePhotonConverted){
		std::cout<<"pt[ "<<m_efakePhoton_ptr.pt_min<<","<<m_efakePhoton_ptr.pt_max<<"]"<<" eta["<<m_efakePhoton_ptr.eta_min<<", "<<m_efakePhoton_ptr.eta_max<<"] :"<<m_efakePhoton_ptr.scale_factor<<" +/- "<<m_efakePhoton_ptr.scale_factor_error<<std::endl;
	}
	std::cout<<" ====================================================================" <<std::endl;

	std::cout<<" ================== Unconverted photon SF =============================" <<std::endl;
	for( const EfakePhoton& m_efakePhoton_ptr : *m_efakePhotonUnconverted){
		std::cout<<"pt[ "<<m_efakePhoton_ptr.pt_min<<","<<m_efakePhoton_ptr.pt_max<<"]"<<" eta["<<m_efakePhoton_ptr.eta_min<<", "<<m_efakePhoton_ptr.eta_max<<"] :"<<m_efakePhoton_ptr.scale_factor<<" +/- "<<m_efakePhoton_ptr.scale_factor_error<<std::endl;
	}
	std::cout<<" ====================================================================" <<std::endl;
}

std::tuple<double, double> EFakeScaleFactorRetriever::getEfakeSF(double eta, double pt, int conversion_type){

	std::tuple<double, double> tmp_sf(1.0, 0.0);
	// depending on pt and eta return the scale factor
	if(conversion_type == 0 ) // unconverted
	{
		for(const EfakePhoton& ph_ptr : *m_efakePhotonUnconverted)
			{
				if( (eta >= ph_ptr.eta_min) && (eta < ph_ptr.eta_max) && (pt >= ph_ptr.pt_min) && (pt < ph_ptr.pt_max) ){
					tmp_sf = std::make_tuple(ph_ptr.scale_factor, ph_ptr.scale_factor_error);
                                        std::cout<<"Conv\t"<<eta<<"\t"<<pt<<"\t"<<ph_ptr.scale_factor<<std::endl;
				}

			}
	}
	if(conversion_type != 0 ) //converted
	{
		for(const EfakePhoton& ph_ptr : *m_efakePhotonConverted)
		{
			if( (eta >= ph_ptr.eta_min) && (eta < ph_ptr.eta_max) && (pt >= ph_ptr.pt_min) && (pt < ph_ptr.pt_max) ){
				tmp_sf = std::make_tuple(ph_ptr.scale_factor, ph_ptr.scale_factor_error);
                                std::cout<<"Unconv\t"<<eta<<"\t"<<pt<<"\t"<<ph_ptr.scale_factor<<std::endl;
	
			}
		
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

  LeptonFakeEfficiencyRetriever::LeptonFakeEfficiencyRetriever(const std::string &realEffRootFileElelctron,
                                                               const std::string &realEffRootFileMuon,
                                                               const std::string &fakeEffRootFileElectron,
                                                               const std::string &fakeEffRootFileMuon)
  {
    realEffRootFileEl = TFile::Open(realEffRootFileElelctron.c_str(), "READ");
    realEffRootFileMu = TFile::Open(realEffRootFileMuon.c_str(), "READ");
    fakeEffRootFileEl = TFile::Open(fakeEffRootFileElectron.c_str(), "READ");
    fakeEffRootFileMu = TFile::Open(fakeEffRootFileMuon.c_str(), "READ");

  }

  LeptonFakeEfficiencyRetriever::~LeptonFakeEfficiencyRetriever() {
  }

  void LeptonFakeEfficiencyRetriever::getHistos(){
    // get the objects
    m_realEfficiency2D_el = (TH1F*)realEffRootFileEl->Get(this->m_realHistName);
    m_realEfficiency2D_mu = (TH1F*)realEffRootFileMu->Get(this->m_realHistName);
    m_fakeEfficiency2D_el = (TH1F*)fakeEffRootFileEl->Get(this->m_fakeHistName);
    m_fakeEfficiency2D_mu = (TH1F*)fakeEffRootFileMu->Get(this->m_fakeHistName);
  }

  Double_t LeptonFakeEfficiencyRetriever::getEventWeight(bool isElectron, bool isMuon, bool isTight, float eta, float pt) {

    double weight=0;
    if(isElectron){
      double fake_eff = m_fakeEfficiency2D_el->GetBinContent(m_fakeEfficiency2D_el->FindFixBin(pt,eta));
      double real_eff = m_realEfficiency2D_el->GetBinContent(m_realEfficiency2D_el->FindFixBin(pt,eta));
      if(isTight){
        if(real_eff != fake_eff) weight = fake_eff*(real_eff - 1)/(real_eff - fake_eff);
      }
      else{
        if(real_eff != fake_eff) weight = fake_eff*(real_eff)/(real_eff - fake_eff);
      }
      // if any of the efficiency value is negative ignore that.
      if(fake_eff < 0 || real_eff < 0){weight = 0;} 
    }
    if(isMuon){
      double fake_eff = m_fakeEfficiency2D_mu->GetBinContent(m_fakeEfficiency2D_mu->FindFixBin(pt,eta));
      double real_eff = m_realEfficiency2D_mu->GetBinContent(m_realEfficiency2D_mu->FindFixBin(pt,eta));
      if(isTight){
        if(real_eff != fake_eff) weight = fake_eff*(real_eff - 1)/(real_eff - fake_eff);
      }
      else{
        if(real_eff != fake_eff) weight = fake_eff*(real_eff)/(real_eff - fake_eff);
      }

      // if any of the efficiency value is negative ignore that.
      if(fake_eff < 0 || real_eff < 0){weight = 0;} 
    }
    return weight;
  }

}


