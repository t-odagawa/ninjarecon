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

  // Setter/Getter
  
  void SetSpillPot(double spill_pot);

  double GetSpillPot() const;

  void SetBunchPot(int bunch, double bunch_pot);

  double GetBunchPot(int bunch) const;

  void SetBsdSpillNumber(int bsd_spill_number);

  int GetBsdSpillNumber() const;

  void SetTimestamp(double timestamp);

  double GetTimestamp() const;

  void SetBsdGoodSpillFlag(int bsd_good_spill_flag);

  int GetBsdGoodSpillFlag() const;

  void SetWagasciGoodSpillFlag(int wagasci_good_spill_flag);

  int GetWagasciGoodSpillFlag() const;

  void SetNumberOfTracks(int number_of_tracks);

  int GetNumberOfTracks() const;

  void SetTrackType(int track, int track_type);

  int GetTrackType(int track) const;

  void SetMomentumType(int track, int momentum_type);

  int GetMomentumType(int track) const;

  void SetMomentum(int track, double momentum);

  double GetMomentum(int track) const;

  void SetBabyMindPosition(int track, int view, double baby_mind_position);

  void SetBabyMindPosition(int track, std::array<double,2> baby_mind_position);

  std::array<double,2> GetBabyMindPosition(int track) const;

  double GetBabyMindPosition(int track, int view) const;

  void SetBabyMindTangent(int track, int view, double baby_mind_tangent);

  void SetBabyMindTangent(int track, std::array<double,2> baby_mind_tangent);

  std::array<double,2> GetBabyMindTangent(int track) const;

  double GetBabyMindTangent(int track, int view) const;

  void SetCharge(int track, int charge);

  int GetCharge(int track) const;

  void SetDirection(int track, int direction);

  int GetDirection(int track) const;

  void SetBunch(int track, int bunch);

  int GetBunch(int track) const;

  void SetNumberOfClusters(int number_of_clusters);

  int GetNumberOfClusters() const;

  void SetBabyMindTrackId(int cluster, int baby_mind_track_id);

  int GetBabyMindTrackId(int cluster) const;

  void SetNumberOfHits(int cluster, int view, int number_of_hits);

  void SetNumberOfHits(int cluster, std::array<int,2> number_of_hits);

  std::array<int,2> GetNumberOfHits(int cluster) const;

  int GetNumberOfHits(int cluster, int view) const;

  void SetPlane(int cluster, int view, int hit, int plane);

  void SetPlane(int cluster, int view, std::vector<int> plane);

  void SetPlane(int cluster, std::array<std::vector<int>,2> plane);

  std::array<std::vector<int>,2> GetPlane(int cluster) const;

  std::vector<int> GetPlane(int cluster, int view) const;

  int GetPlane(int cluster, int view, int hit) const;

  void SetSlot(int cluster, int view, int hit, int slot);

  void SetSlot(int cluster, int view, std::vector<int> slot);

  void SetSlot(int cluster, std::array<std::vector<int>,2> slot);

  std::array<std::vector<int>,2> GetSlot(int cluster) const;

  std::vector<int> GetSlot(int cluster, int view) const;

  int GetSlot(int cluster, int view, int hit) const;

  void SetPe(int cluster, int view, int hit, double pe);

  void SetPe(int cluster, int view, std::vector<double> pe);

  void SetPe(int cluster, std::array<std::vector<double>,2> pe);

  std::array<std::vector<double>,2> GetPe(int cluster) const;

  std::vector<double> GetPe(int cluster, int view) const;

  double GetPe(int cluster, int view, int hit) const;

  void SetBunchDifference(int cluster, int bunch_difference);

  int GetBunchDifference(int cluster) const;

  void SetNinjaPosition(int cluster, int view, double ninja_position);

  void SetNinjaPosition(int cluster, std::array<double,2> ninja_position);

  std::array<double,2> GetNinjaPosition(int cluster) const;

  double GetNinjaPosition(int cluster, int view) const;

  void SetNinjaTangent(int cluster, int view, double ninja_tangent);

  void SetNinjaTangent(int cluster, std::array<double,2> ninja_tangent);

  std::array<double,2> GetNinjaTangent(int cluster) const;

  double GetNinjaTangent(int cluster, int view) const;

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
  int wagasci_good_spill_flag_;
  ///> Baby MIND information extracted from B2TrackSummary and B2ClusterSummary
  ///> number of tracks;
  int number_of_tracks_;
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
  int number_of_ninja_clusters_;
  ///> Corresponding Baby MIND track id
  std::vector<int> baby_mind_track_id_;
  ///> Number of NINJA tracker hits
  std::vector<std::array<int,2>> number_of_hits_;
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

  ClassDefOverride(NTBMSummary, 3) // NT BM Summary
};

#endif
