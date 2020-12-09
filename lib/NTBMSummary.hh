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

  NTBMSummary();

  /**
   * Set the all members to zero
   * no memory is released.
   * @param option same as TObject option
   */
  void Clear(Option_t *option) override;

  /**
   * Print all the NTBM members value to the output stream
   * @param os output stream
   * @param obj object
   * @return output stream
   */
  friend std::ostream &operator<<(std::ostream &os, const NTBMSummary &obj);

  void SetSpillPot(double spill_pot);

  int GetSpillPot() const;

  void SetBunchPot(int bunch, double bunch_pot);

  double GetBunchPot(int bunch) const;

  void SetBsdSpillNumber(int bsd_spill_number);

  int GetBsdSpillNumber() const;

private :
  ///> Beam information extracted from B2BeamSummary
  ///> total POT of the spill
  double spill_pot_;
  ///> POT for each bunch
  double bunch_pot_[8];
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
  std::vector<std::array<double,2>> baby_mind_position_;
  ///> Baby MIND reconstructed tangent
  std::vector<std::array<double,2>> baby_mind_tangent_;
  ///> Baby MIND reconstructed charge (assuming muon +/-)
  std::vector<int> charge_;
  ///> Baby MIND reconstructed track direction (+/-)
  std::vector<int> direction_;
  ///> Bunch number where the track detected
  std::vector<int> bunch_;
  ///> NINJA tracker information for muon track matching
  ///> Number of NINJA tracker 3d clusters
  int clusters_;
  ///> Corresponding Baby MIND track id
  std::vector<int> baby_mind_track_id_;
  ///> Number of NINJA tracker hits
  std::vector<std::array<int,2>> hits_;
  ///> List of hit scintillator plane in NINJA tracker
  std::vector<std::array<std::vector<int>,2>> plane_;
  ///> List of hit scintillator slot in NINJA tracker
  std::vector<std::array<std::vector<int>,2>> slot_;
  ///> List of hit scintillator pe/tot in NINJA tracker
  std::vector<std::array<std::vector<double>,2>> pe_;
  ///> Difference from the first hit bunch in NINJA tracker
  std::vector<int> bunch_difference_;
  ///> Reconstructed position for track matching
  std::vector<std::array<double,2>> ninja_position_;
  ///> Reconstructed tangent for track matching
  std::vector<std::array<double,2>> ninja_tangent_;

  //ClassDefOverride(NTBMSummary, 1) // NT BM Summary
}

#endif
