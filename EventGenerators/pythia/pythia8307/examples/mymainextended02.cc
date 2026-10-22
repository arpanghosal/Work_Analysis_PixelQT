
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
	cout << "\nStarting event" <<iEvent<<endl;
  pythia.next(); // Generate an(other) event. Fill event record.
  int iTop = 0;
  for (int i = 0; i < pythia.event.size(); ++i) {   //introducing particle loop to go through every particle within the event
	cout << "i = " << i << ", id = " << pythia.event[i].id() << endl;
  if (pythia.event[i].id() == 6) iTop = i;
  }
  pythia.readString("Next:numberShowEvent = 5");

  cout << "\nFinal top is at : "<<iTop;
  }

  pythia.stat();  //to print stats, x-sec etc.
  return 0;
}
// End main program with error-free return.
