#include "TTGammaNtupleSkimmer/SampleMetaData.h"

#include <iostream>

#include "PathResolver/PathResolver.h"
#include "TopDataPreparation/SampleXsection.h"
#include "TFile.h"
#include "TTreeReader.h"

namespace {
bool needs_generator_weights(float dsid) {
  if (dsid == 410389) return true;     // nominal tty
  if (dsid == 410404) return true;     // tty radiation up
  if (dsid == 410405) return true;     // tty radiation down
  if (dsid == 410395) return true;     // tty Herwig7
  if (dsid == 412120) return true;     // Wty (y prod) nominal
  if (dsid == 412122) return true;     // Wty (y prod) Herwig7 (because nominal weight is no. 85)
  if (dsid == 412006) return true;     // Wty (y decay) nominal
  if (dsid == 412007) return true;     // Wty (y decay) Herwig 7
  if (dsid == 410472) return true;     // nominal ttbar dilepton
  if (dsid == 410482) return true;     // ttbar dilepton with hdamp variation
  if (dsid == 410648) return true;     // nominal single-top Wt dilepton
  if (dsid == 410649) return true;     // nominal single-top Wt dilepton
  return false;
}
}

namespace TTGammaNtupleSkimmer {
SampleMetaData::SampleMetaData(const std::vector<std::string>& file_list)
  : total_events_weighted_{0.}
  , total_events_weighted_vector_{}
  , total_events_{0.}
  , xsec_{0.} {
  // Load xsec information from TopDataPreparation.
  SampleXsection tdp;
  auto tdp_file = PathResolverFindCalibFile("dev/AnalysisTop/TopDataPreparation/XSection-MC15-13TeV.data");
  if (!tdp.readFromFile(tdp_file.c_str())) {
    std::cout << "TopDataPreparation - could not read file\n";
    std::cout << tdp_file << "\n";
    std::cout << "Maybe check it exists, first?" << std::endl;
    exit(1);
  }

  for (const auto& entry : file_list) {
    auto file = TFile::Open(entry.c_str(), "READ");
    TTreeReader reader{"sumWeights", file};
    TTreeReaderValue<Int_t> mc_channel{reader, "dsid"};

    // First get some meta information about sample.
    while (reader.Next()) {
      dsid_ = *mc_channel;
      xsec_ = tdp.getXsection(*mc_channel);
      break;
    }

    // Now restart the reader and get the event numbers.
    reader.Restart();
    TTreeReaderValue<Float_t> total_events_weighted_reader{reader, "totalEventsWeighted"};
    TTreeReaderValue<ULong64_t> total_events_reader{reader, "totalEvents"};
    while (reader.Next()) {
      total_events_weighted_ += *total_events_weighted_reader;
      total_events_ += *total_events_reader;
    }

    // If we need the varied generator weights, restart once
    // again and get the entire weight vectors.
    if (needs_generator_weights(dsid_)) {
      reader.Restart();
      TTreeReaderValue<std::vector<Float_t> > total_events_weighted_vector_reader{reader, "totalEventsWeighted_mc_generator_weights"};
      while (reader.Next()) {
        total_events_weighted_vector_.resize(total_events_weighted_vector_reader->size());
        for (size_t i = 0; i < total_events_weighted_vector_reader->size(); ++i) {
          total_events_weighted_vector_.at(i) += total_events_weighted_vector_reader->at(i);
        }
      }
    } else {
      total_events_weighted_vector_.resize(0);
    }
  }

  std::cout << "TotalEventsWeighted: " << total_events_weighted_ << std::endl;
  std::cout << "TotalEvents: " << total_events_ << std::endl;
}
}  // namespace TTGammaNtupleSkimmer
