# Overlap removal procedure between samples

This page is dedicated to describing the overlap-removal procedures between
different samples used in the ttgamma analysis. In principal, we have to
address overlap between these three pairs of samples:
1. ttbar and the dedicated tt+gamma sample
2. Z+jets and the dedicated Z+gamma+jets samples
3. W+jets and the dedicated W+gamma+jets samples

The code currently implements three different overlap-removal techniques. In
principal, either of the three could be used to address the above three sample
pairs and stitch them together. The following subsections contain descriptions
of the three techniques:
1. overlap removal as used in the ttH multi-lepton analysis
2. overlap removal as used in the multiboson focus group to remove overlap
   between V+jets and V+gamma samples.
3. overlap removal as used in the 20.7 analysis 

> Documentation for these methods still needs to be added/improved.


## Procedures as used in ttH ML analysis

> The following paragraphs are directly taken from the [ttH multi-lepton INT
> note](https://cds.cern.ch/record/2244753/)

In the 0tau channels of the ttH multi-lepton analysis, especially in the
3-lepton category, probability of the final signal region selection being faked
by the ttgamma process is non-negligible. In order to take that into account
correctly, a dedicated ttgamma MC sample was produced with MADGRAPH5_AMC@NLO
(for details, see Sec. 2.2). This sample contains the photon production
calculated at matrix element level for the initial and final state radiation
(initial - from the incoming quark, final - from the top quark). This process is
illustrated in Fig. 54a. However, the MADGRAPH5_AMC@NLO does not include matrix
element photon emission of the top quark decay products. This additional photon
emission is simulated by the PYTHIA 8 / PHOTOS parton showering. On the other
hand, the MC generator generally used for the ttbar process, is POWHEG-BOX (with
a two lepton filter at generator level). In this case, the matrix element
calculation does not initial and final state (top quark) radiation, however it
implements photon Bremsstrahlung off the top quark decay products (W,b and
charged leptons - see Fig. 54b). Additionally, as in the first case, PYTHIA 8 /
PHOTOS is used for parton showering and mimics the missing radiation.


The ttgamma MC sample was generated with large statistics with filter on the
matrix element photon being defined as dR(truthparent, ME photon) > 0.2 and
pT(truth photon) > 15 GeV. In order to avoid possible double counting coming
from different parts of the process being simulated in different ways for both
generators, a ttbar−ttgamma overlap removal procedure was adopted from [64]. The
procedure is based on finding the matrix element photon in the truth record,
following conditions have to be met:

* a mother particle exists with non-zero momentum and it is not a photon
* PDG ID of the mother particle < 100 to make sure the parent is coming from the
  ME calculation
* ME photon candidate barcode < 200000 to make sure that the photon is not
  originating from interaction with the detector
* pT(truth photon) > 15 GeV

Once the candidate has been found, its proximity to the mother was calculated
and following sample- stitching logic was applied:
* if deltaR(truthparent, ME photon) in the ttgamma sample was greater than 0.2,
  accept all events where the parent was not a lepton
* in any other cases accept events from the ttbar sample

The selection is motivated by the fact that in the ttgamma sample the matrix
element calculation which involves initial and final state radiation (IFSR) is
assumed to be more accurate that photon emission through PYTHIA 8 / PHOTOS. The
condition above actually should exclude also the W boson and b-quarks from the
ttgamma, however it was found that there are no photons originating from W
bosons or b-quarks in the ttbar sample, hence the ones from ttgamma were kept.
Open question remains if it is not due to an incorrect photon origin labelling
in the ttbar sample. For the cases where deltaR(truthparent, ME photon) < 0.2,
all events from ttbar sample were used regardless the mother ID due to a much
larger statistics of the ttbar sample in that part of phase space.

The result of the stitching is presented in Fig. 55 at inclusive level, showing
in the bottom part of the plot the increasing fraction of the ttgamma. The plot
has been performed at an early stage of the analysis and hence hasn't been
updated.
