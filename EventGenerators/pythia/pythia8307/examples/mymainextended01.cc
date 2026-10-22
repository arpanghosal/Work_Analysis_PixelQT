
// Headers and Namespaces.
#include "Pythia8/Pythia.h" // Include Pythia headers.
using namespace Pythia8;
int main() {
  // Set up generation.
  Pythia pythia;
  pythia.readString("Top:gg2ttbar = on"); // Switch on process.
  pythia.readString("Top:qqbar2ttbar = on"); // Switch on process.
  pythia.readString("Beams:eCM = 8000."); // 8 TeV CM energy.
  pythia.init(); // Initialize; incoming pp beams is default.
  // Generate event(s).
  for(int iEvent=0;iEvent<5;iEvent++){    //introducing Event loop
  pythia.next(); // Generate an(other) event. Fill event record.
  pythia.readString("Next:numberShowEvent = 5");
  }
  pythia.stat();  //to print stats, x-sec etc.
  return 0;
}
// End main program with error-free return.
