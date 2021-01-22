// system includes
#include <vector>
#include <algorithm>

// boost includes
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

// B2 includes
#include "B2Reader.hh"
#include "B2Writer.hh"
#include "B2Enum.hh"
#include "B2Dimension.hh"
#include "B2HitSummary.hh"
#include "B2VertexSummary.hh"
#include "B2ClusterSummary.hh"
#include "B2TrackSummary.hh"
#include "NTBMSummary.hh"

namespace logging = boost::log;

/**
 * Get scintillator center position of B2HitSummary in NINJA tracker coordinate
 * @param ninja_hit B2HitSummary object
 * @return scintillator position in NINJA tracker coordinate
 */
double GetScintillatorPosition(const B2HitSummary* ninja_hit) {
  TVector3 position;
  B2Dimension::GetPosNinjaTracker(ninja_hit->GetView(), ninja_hit->GetPlane(),
				  ninja_hit->GetSlot().GetValue(ninja_hit->GetSingleReadout()),
				  position);
  switch (ninja_hit->GetView()) {
  case B2View::kTopView :
    return position.X();
  case B2View::kSideView :
    return position.Y();
  case B2View::kUnknownView :
    BOOST_LOG_TRIVIAL(error) << "Unknown view";
    return B2_NON_INITIALIZED_VALUE;
  }
}

/**
 * Comparator for B2HitSummary vector sort
 * @param lhs left hand side object
 * @param rhs right hand side object
 * @return true if the objects should not be swapped
 */
bool CompareNinjaHits(const B2HitSummary* lhs, const B2HitSummary* rhs) {
  if (lhs->GetView()!=rhs->GetView()) return lhs->GetView() < rhs->GetView();
  return GetScintillatorPosition(lhs) < GetScintillatorPosition(rhs);
}

/**
 * Create NINJA tracker clusters
 * @param ninja_hits NINJA Hit summary vector
 * @param ninja_clusters NTBMSummary for the spill (x/y separated and only NINJA tracker data)
 */
void CreateNinjaCluster(std::vector<const B2HitSummary* > ninja_hits,
			NTBMSummary* ninja_clusters) {
  std::sort(ninja_hits.begin(), ninja_hits.end(), CompareNinjaHits);

  double scintillator_position_tmp;
  int view_tmp;
  
  int number_of_ninja_clusters = 0;
  std::vector<int> number_of_hits_tmp(2, 0);
  std::vector<std::vector<int>> number_of_hits = {};
  std::vector<std::vector<int>> plane_tmp, slot_tmp;
  std::vector<std::vector<double>> pe_tmp;
  plane_tmp.resize(2); slot_tmp.resize(2);
  pe_tmp.resize(2);
  std::vector<std::vector<std::vector<int>>> plane = {}, slot = {};
  std::vector<std::vector<std::vector<double>>> pe = {};

  for(const auto ninja_hit : ninja_hits) {

    // when scintillators have a gap, create new NINJA cluster
    if ( ( ninja_hit != ninja_hits.front() )
	&& ( GetScintillatorPosition(ninja_hit) >  scintillator_position_tmp + 24.
	     || ninja_hit->GetView() != view_tmp || ninja_hit == ninja_hits.back()) ) {
      number_of_ninja_clusters++;
      number_of_hits.push_back(number_of_hits_tmp); number_of_hits_tmp.assign(2,0);
      plane.push_back(plane_tmp);
      plane_tmp.at(0) = {}; plane_tmp.at(1) = {};
      slot.push_back(slot_tmp);
      slot_tmp.at(0) = {}; slot_tmp.at(1) = {};
      pe.push_back(pe_tmp);
      pe_tmp.at(0) = {}; pe_tmp.at(1) = {};
    }

    number_of_hits_tmp.at(ninja_hit->GetView())++;
    plane_tmp.at(ninja_hit->GetView()).push_back(ninja_hit->GetPlane());
    slot_tmp.at(ninja_hit->GetView()).push_back(ninja_hit->GetSlot().GetValue(ninja_hit->GetSingleReadout()));
    pe_tmp.at(ninja_hit->GetView()).push_back(ninja_hit->GetHighGainPeu().GetValue(ninja_hit->GetSingleReadout()));

    scintillator_position_tmp = GetScintillatorPosition(ninja_hit);
    view_tmp = ninja_hit->GetView();

  }

  ninja_clusters->SetNumberOfNinjaClusters(number_of_ninja_clusters);
  for(int icluster = 0; icluster < number_of_ninja_clusters; icluster++) {
    ninja_clusters->SetNumberOfHits(icluster, number_of_hits.at(icluster));
    ninja_clusters->SetPlane(icluster, plane.at(icluster));
    ninja_clusters->SetSlot(icluster, slot.at(icluster));
    ninja_clusters->SetPe(icluster, pe.at(icluster));
  }

  BOOST_LOG_TRIVIAL(debug) << "NINJA tracker clusters created";
  
}

/**
 * Get incidenct track position on the upstream surface of Baby MIND
 * calculation is changed depending on the track type
 * @param track B2TrackSummary object
 * @return track position on the upstream surface of Baby MIND
 */
TVector3 CalculateTrackInitialPosition(const B2TrackSummary *track) {

  TVector3 track_initial_position = {};
  
  if ( track->GetType() == kPrimaryTrack ) {
    TVector3 track_direction = track->GetFinalDirection().GetValue();    
    const Double_t bm_pos_z = 2.85 * m - 2. * m;
    Double_t x = track->GetInitialPosition().GetValue().X()
      + track_direction.X() * (bm_pos_z - track->GetInitialPosition().GetValue().Z());
    Double_t y = track->GetInitialPosition().GetValue().Y()
      + track_direction.Y() * (bm_pos_z - track->GetInitialPosition().GetValue().Z());
    track_initial_position.SetX(x);
    track_initial_position.SetY(y);
    track_initial_position.SetZ(bm_pos_z);
  } else if ( track->GetType() == kBabyMind3DTrack ) {
    track_initial_position = track->GetInitialPosition().GetValue();
  } else {
    BOOST_LOG_TRIVIAL(error) << "Reconstructed Track Summary is not in NINJA interest";
  }

  return track_initial_position;
  
}

/**
 * Track matching between Baby MIND and NINJA tracker using x/y separated NTBMSummary
 * and Baby MIND B2TrackSummary
 * @param track B2TrackSummary object of Baby MIND track
 * @param ntbm_in NTBMSummary object created in the CreateNinjaCluster function
 * @param ntbm_out NTBMSummary object for 3D track matching
 */
void MatchBabyMindTrack(const B2TrackSummary *track, NTBMSummary* ntbm_in, NTBMSummary* ntbm_out) {

  int track_type;
  if ( track->GetType() == kSandMuon ) track_type = 1;
  else if ( track->GetType() == kPrimaryTrack ) track_type = -1;
  else track_type = 0;


  
  int momentum_type = 0;
  double momentum = track->GetInitialAbsoluteMomentum().GetValue();
  double momentum_error = track->GetInitialAbsoluteMomentum().GetError();

  TVector3 track_initial_position = CalculateTrackInitialPosition(track);
  //TVector3 track_initial_position_error = track->GetInitialPosition().GetError(); // Baby MIND position erro TODO
  TVector3 track_initial_direction = track->GetFinalDirection().GetValue();
  TVector3 track_initial_direction_error = track->GetFinalDirection().GetError();

  int charge = 1;
  int direction = 1; // positive or negative
  int bunch = 0;

  // Get the nearest X/Y NINJA cluster respectively
  for (int intbmcluster = 0; intbmcluster < ntbm_in->GetNumberOfNinjaClusters(); intbmcluster++) {

  }

  // Push back output NTBMSummary object
  
}

/**
 * Use Baby MIND information, reconstruct position (and angle) for matching
 * between NINJA tracker and Emulsion shifter
 * @param ntbmsummary NTBMSummary object after the MatchBabyMindTrack function
 */
void ReconstructNinjaPosition(NTBMSummary* ntbmsummary) {

  for (int icluster = 0; icluster < ntbmsummary->GetNumberOfNinjaClusters(); icluster++) {

  }
  
}

int main(int argc, char *argv[]) {

  logging::core::get()->set_filter
    (
     logging::trivial::severity >= logging::trivial::info
     );

  BOOST_LOG_TRIVIAL(info) << "==========NINJA Track Matching Start==========";

  if (argc != 3) {
    BOOST_LOG_TRIVIAL(error) << "Usage : " << argv[0]
			     << " <input B2 file path> <output NTBM file path>";
    std::exit(1);
  }

  try {
    B2Reader reader(argv[1]);

    TFile *ntbm_file = new TFile(argv[2], "recreate");
    ntbm_file->cd();
    TTree *ntbm_tree = new TTree("tree", "NINJA BabyMIND Original Summary");
    NTBMSummary* ntbm_tmp = nullptr;
    NTBMSummary* my_ntbm = nullptr;
    ntbm_tree->Branch("NTBMSummary", &my_ntbm);

    while (reader.ReadNextSpill() > 0) {
      auto &input_spill_summary = reader.GetSpillSummary();

      // Create NINJA tracker hit cluster
      auto it_hit = input_spill_summary.BeginHit();
      std::vector<const B2HitSummary* > ninja_hits;
      while (const auto *ninja_hit = it_hit.Next()) {
	if(ninja_hit->GetDetectorId() == B2Detector::kNinja)
	  ninja_hits.push_back(ninja_hit);
      }

      // Create X/Y NINJA clusters
      if (ninja_hits.size() > 0) 
	CreateNinjaCluster(ninja_hits, ntbm_tmp);

      // Extrapolate BabyMIND tracks to the NINJA position
      // and get the best cluster to match each BabyMIND track
      int number_of_tracks = 0;
      auto it_track = input_spill_summary.BeginReconTrack();
      while (const auto *track = it_track.Next()) {
	if (track->HasDetector(B2Detector::kBabyMind)) {
	  number_of_tracks++;
	  // if (NinjaHitExpected(track)) {
	  MatchBabyMindTrack(track, ntbm_tmp, my_ntbm);
	  //}
	}
      }
      my_ntbm->SetNumberOfTracks(number_of_tracks);
      
      // Update NINJA hit summary information
      ReconstructNinjaPosition(my_ntbm);
      
      // Create output file
      ntbm_tree->Fill();
      ntbm_tmp->Clear("C");
      my_ntbm->Clear("C");
    }

    ntbm_file->cd();
    ntbm_tree->Write();
    ntbm_file->Close();
    
  } catch (const std::runtime_error &error) {
    BOOST_LOG_TRIVIAL(fatal) << "Runtime error : " << error.what();
  } catch (const std::invalid_argument &error) {
    BOOST_LOG_TRIVIAL(fatal) << "Invalid argument error : " << error.what();
    std::exit(1);
  }
  
  BOOST_LOG_TRIVIAL(info) << "==========NINJA Track Matching Finish==========";
  std::exit(0);

}
