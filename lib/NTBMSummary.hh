#ifndef NTBMSUMMARY_HH
#define NTBMSUMMARY_HH

#include <TObject.h>

#include <vector>
#include <array>

#ifdef __ROOTCLING__
#pragma link off globals;
#pragma link off classes;
#pragma link off functions;
#pragma link C++ class NTBMSummary+;
#endif

/**
 * Class containing info about scintillator detectors for (muon) track matching
 * between Scintillation Tracker/BabyMIND and TSS/Emulsion Shifter/ECC
 */

class NTBMSummary: public TObject {

public :

private :
  ///> Beam information extracted from B2BeamSummary
  ///> total POT of the spill
  int spill_pot_;
  ///> POT for each bunch
  int bunch_pot_[8];
  ///> BSD spill number
  int bsd_spill_number_;
  ///> spill time stamp
  double timestamp_;
  ///> BSD spill flag
  int bsd_good_spill_flag_;
  ///> WAGASCI spill flag
  int wagasc_good_spill_flag_;
  ///> Baby MIND information extracted from B2TrackSummary and B2ClusterSummary
  ///> track type (0:ECC interaction cand, 1:sand muon, -1:other)
  std::vector<int> track_type_;
  ///> momentum measurement type (0:range, 1:curvature)
  std::vector<int> momentum_type_;
  ///> Baby MIND reconstructed momentum
  std::vector<double> momentum_;
  ///> Baby MIND reconstructed position
  std::vector<array<double,2>> position_;
  ///> Baby MIND reconstructed tangent
  std::vector<array<double,2>> tangent_;
  ///> Baby MIND reconstructed charge (assuming muon +/-)
  std::vector<int> charge_;
  ///> Baby MIND reconstructed track direction (+/-)
  std::vector<int> direction_;
  ///> Bunch number where the track detected
  std::vector<int> bunch_;
  ///> NINJA tracker information for muon track matching
  ///> Corresponding Baby MIND track id
  std::vector<int> baby_mind_track_id_;
}

#endif
