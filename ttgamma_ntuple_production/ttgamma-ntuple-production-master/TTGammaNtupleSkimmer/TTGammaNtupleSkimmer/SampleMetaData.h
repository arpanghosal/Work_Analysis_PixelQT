#ifndef TTGAMMA_SKIMMING_PACKAGE_SAMPLE_META_DATA_H_
#define TTGAMMA_SKIMMING_PACKAGE_SAMPLE_META_DATA_H_

#include <string>
#include <vector>

namespace TTGammaNtupleSkimmer {
/// A class to retrieve some meta information from the sample and
/// provide functionality to retrieve it easily.
class SampleMetaData {
 public:
  /// Default constructor. This takes the list of input files and
  /// extracts all information from them.
  SampleMetaData(const std::vector<std::string>& file_list);

  /// Defaulted destructor.
  ~SampleMetaData() = default;

  /// Get the DSID of the sample.
  int getDSID() { return dsid_; }

  /// Get the total events weighted of the sample.
  float getTotalEventsWeighted() { return total_events_weighted_; }

  /// Get the total events weighted (rad-up variation) of the sample.
  std::vector<float>* getTotalEventsWeightedVector() { return &total_events_weighted_vector_; }

  /// Get the number of total events.
  float getTotalEvents() { return total_events_; }

  /// Get the cross section associated with the sample.
  float getXSec() { return xsec_; }

 private:
  float dsid_;
  float total_events_weighted_;
  std::vector<float> total_events_weighted_vector_;
  float total_events_;
  float xsec_;
};
}  // namespace TTGammaNtupleSkimmer

#endif  // TTGAMMA_SKIMMING_PACKAGE_SAMPLE_META_DATA_H_
