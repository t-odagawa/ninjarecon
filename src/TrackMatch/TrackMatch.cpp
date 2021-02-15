// system includes
#include <vector>
#include <numeric>
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
#include "B2EventSummary.hh"
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
	&& ( GetScintillatorPosition(ninja_hit) >  scintillator_position_tmp + NINJA_TRACKER_SCI_WIDTH
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
 * Original function instead of B2HitsSet::HasDetector() as reconstructed track summary
 * does not have detector information on its own
 * @param track reconstructed B2TrackSummary object
 * @param det detector id
 * @return true if the object has hits inside the detector
 */
bool MyHasDetector(const B2TrackSummary *track, B2Detector det) {

  bool ret = false;

  auto it_cluster = track->BeginCluster();
  while (const auto *cluster = it_cluster.Next()) {
    auto it_hit = cluster->BeginHit();
    while (const auto *hit = it_hit.Next()) {
      if (hit->GetDetectorId() == det) ret = true;
    }
  }
  return ret;
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
 * Check if the reconstructed track summary expected to have hits
 * in the NINJA tracker
 * @param track B2TrackSummary object of Reconstructed Baby MIND track
 * @return true if the track expected to have hits else false
 */
bool NinjaHitExpected(const B2TrackSummary *track) {
  switch(track->GetType()) {
  case kBabyMind3DTrack :
    // only if extrapolated position is inside NINJA tracker
    /*if() return true;
    else return false;
    break;*/
  case kPrimaryTrack :
    // only if extrapolated position is inside NINJA tracker
    // and not downstream WAGASCI interaction
    /*if() return true;
    else return false;
    break;*/
  default :
    BOOST_LOG_TRIVIAL(debug) << "Reconstructed Track Summary is not in NINJA interest";
  }

  return true; // TODO
}
/**
 * Track matching between Baby MIND and NINJA tracker using x/y separated NTBMSummary
 * and Baby MIND B2TrackSummary
 * @param track B2TrackSummary object of Baby MIND track
 * @param ntbm_in NTBMSummary object created in the CreateNinjaCluster function
 * @param ntbm_out NTBMSummary object for 3D track matching
 */
void MatchBabyMindTrack(const B2TrackSummary *track, NTBMSummary* ntbm_in, NTBMSummary* ntbm_out) {

  int momentum_type = 0;
  double momentum = track->GetInitialAbsoluteMomentum().GetValue();
  double momentum_error = track->GetInitialAbsoluteMomentum().GetError();

  TVector3 track_initial_position = CalculateTrackInitialPosition(track);
  //TVector3 track_initial_position_error = track->GetInitialPosition().GetError(); // Baby MIND position error TODO
  TVector3 track_initial_direction = track->GetFinalDirection().GetValue();
  TVector3 track_initial_direction_error = track->GetFinalDirection().GetError(); // Baby MIND tangent error TODO

  int charge = 1;
  int direction = 1; // positive or negative
  int bunch = 0;

  // Get the nearest X/Y NINJA cluster respectively
  for (int intbmcluster = 0; intbmcluster < ntbm_in->GetNumberOfNinjaClusters(); intbmcluster++) {

  }

  // Push back output NTBMSummary object
  
}

/**
 * Get boolean if the value is in range [min, max]
 * @param pos position to be evaluated
 * @param min minimum value of the range
 * @param max maximum value of the range
 */
bool IsInRange(double pos, double min, double max) {
  if (min <= pos && pos <= max) return true;
  else return false;
}

/**
 * Get boolean if range [min, max] makes hit in a scintillator bar
 * @param min minimum value of the range
 * @param max maximum value of the range
 * @param view pln slot detector ids of the scintillator bar
 */
bool IsMakeHit(double min, double max, int view, int plane, int slot) {
  TVector3 position;
  B2Dimension::GetPosNinjaTracker((B2View)view, plane, slot, position);
  double position_xy;
  switch (view) {
  case B2View::kTopView : 
    position_xy = position.X();
    break;
  case B2View::kSideView :
    position_xy = position.Y();
    break;
  case B2View::kUnknownView :
    BOOST_LOG_TRIVIAL(error) << "Unknown view";
    std::exit(1);
  }

  return IsInRange(min, position_xy - NINJA_TRACKER_SCI_WIDTH / 2. - NINJA_TRACKER_GAP,
		   position_xy + NINJA_TRACKER_SCI_WIDTH / 2.)
    && IsInRange(max, position_xy - NINJA_TRACKER_SCI_WIDTH / 2.,
		 position_xy + NINJA_TRACKER_SCI_WIDTH / 2. + NINJA_TRACKER_GAP);
}

/**
 * Get boolean if range [min, max] does not make hit in a scintillator bar
 * @param min minimum value of the range
 * @param max maximum value of the range
 * @param view pln slot detector ids of the scintillator bar
 */
bool IsInGap(double min, double max, int view, int plane, int slot) {
  TVector3 position;
  B2Dimension::GetPosNinjaTracker((B2View)view, plane, slot, position);
  double position_xy;
  switch (view) {
  case B2View::kTopView : 
    position_xy = position.X();
    break;
  case B2View::kSideView :
    position_xy = position.Y();
    break;
  case B2View::kUnknownView :
    BOOST_LOG_TRIVIAL(error) << "Unknown view";
    std::exit(1);
  }

  switch (slot) {
  case 0 :
    return max <= position_xy - NINJA_TRACKER_SCI_WIDTH / 2.;
  case NINJA_TRACKER_NUM_CHANNELS_ONE_PLANE - 1 :
    return position_xy + NINJA_TRACKER_SCI_WIDTH / 2. <= min;
  default : 
    return position_xy + NINJA_TRACKER_SCI_WIDTH / 2. <= min
      && max <= position_xy - NINJA_TRACKER_SCI_WIDTH / 2.;
  }
}


/**
 * Get the minimum value of the position where the line intercepts
 * @param pos track start position
 * @param tangent track tangent
 * @param iplane plane id of the track starting scintillator bar
 * @param jplane plane id of the intercept evaluated scintillator bar
 * @param vertex vertex position of the track starting point
 */
double GetTrackAreaMin(double pos, double tangent, int iplane, int jplane, int vertex) {
  if (tangent > 0) return pos + tangent * (NINJA_TRACKER_OFFSET_Z[jplane] - NINJA_TRACKER_OFFSET_Z[iplane]
					   + NINJA_TRACKER_SCI_THICK * (0 - vertex % 2));
  else return pos + tangent * (NINJA_TRACKER_OFFSET_Z[jplane] - NINJA_TRACKER_OFFSET_Z[iplane]
			       + NINJA_TRACKER_SCI_THICK * (1 - vertex % 2));
}

/**
 * Get the maximum value of the position where the line intercepts
 * @param pos track start position
 * @param tangent track tangent
 * @param iplane plane id of the track starting scintillator bar
 * @param jplane plane id of the intercept evaluated scintillator bar
 * @param vertex vertex position of the track starting point
 */
double GetTrackAreaMax(double pos, double tangent, int iplane, int jplane, int vertex) {
  if (tangent > 0) return pos + tangent * (NINJA_TRACKER_OFFSET_Z[jplane] - NINJA_TRACKER_OFFSET_Z[iplane]
					   + NINJA_TRACKER_SCI_THICK * (1 - vertex % 2));
  else return pos + tangent * (NINJA_TRACKER_OFFSET_Z[jplane] - NINJA_TRACKER_OFFSET_Z[iplane]
			       + NINJA_TRACKER_SCI_THICK * (0 - vertex % 2));
}

/**
 * Get boolean if the normal track analysis is possible
 * @param condition four element array of boolean
 */
bool IsGoodTrack(bool *condition) {
  bool ret = true;
  for (int iplane = 0; iplane < NINJA_TRACKER_NUM_PLANES; iplane++)
    ret = ret && condition[iplane];
  return ret;
}

/**
 * Use Baby MIND information, reconstruct position (and angle) for matching
 * between NINJA tracker and Emulsion shifter
 * @param ntbmsummary NTBMSummary object after the MatchBabyMindTrack function
 */
void ReconstructNinjaPosition(NTBMSummary* ntbmsummary) {

  for (int icluster = 0; icluster < ntbmsummary->GetNumberOfNinjaClusters(); icluster++) {
    // Angle Reconstruction
    std::vector<double> tangent(2);
    tangent.at(0) = 0;
    tangent.at(1) = 0;
    ntbmsummary->SetNinjaTangent(icluster, tangent);

    // Position Reconstruction using reconstructed angle
    std::vector<double> position(2);

    std::vector<std::vector<double>> position_list = {}; // vector where good position candidates are filled

    for(int iview = 0; iview < 2; iview++) {
      for (int iplane = 0; iplane < NINJA_TRACKER_NUM_PLANES; iplane++) {
	for (int islot = 0; islot < NINJA_TRACKER_NUM_CHANNELS_ONE_PLANE; islot++) {
	  for (int ivertex = 0; ivertex < 4; ivertex++) { // Number of vertices in one scintillator bar
	    
	    TVector3 start_of_track;
	    B2Dimension::GetPosNinjaTracker((B2View)iview, iplane, islot, start_of_track);
	    double start_of_track_xy;
	    switch (iview) {
	    case 0 :
	      start_of_track_xy = start_of_track.X()
		+ NINJA_TRACKER_SCI_WIDTH / 2. * ( -1 + 2 * (ivertex/2) );
	      break;
	    case 1 :
	      start_of_track_xy = start_of_track.Y()
		+ NINJA_TRACKER_SCI_WIDTH / 2. * ( -1 + 2 * (ivertex/2) );
	      break;
	    }
	    
	    // Check the line can make a hit pattern
	    bool plane_condition[NINJA_TRACKER_NUM_PLANES] = {false};
	    
	    for (int jplane = 0; jplane < NINJA_TRACKER_NUM_PLANES; jplane++) {

	      double track_area_min = GetTrackAreaMin(start_of_track_xy, tangent.at(iview),
						      iplane, jplane, ivertex);
	      double track_area_max = GetTrackAreaMax(start_of_track_xy, tangent.at(iview),
						      iplane, jplane, ivertex);
	    
	      if (ntbmsummary->GetNumberOfHits(icluster, iview) > 0) {
		for (int ihit = 0; ihit < ntbmsummary->GetNumberOfHits(icluster, iview); ihit++) {
		  if (ntbmsummary->GetPlane(icluster, iview, ihit) == jplane) {
		    plane_condition[jplane] = IsMakeHit(track_area_min, track_area_max,
							iview, jplane, ntbmsummary->GetSlot(icluster, iview, ihit));
		    break;
		  }
		} // ihit
	      } else {
		for (int jslot = 0; jslot < NINJA_TRACKER_NUM_CHANNELS_ONE_PLANE; jslot++) {
		  plane_condition[jplane] = plane_condition[jplane] || IsInGap(track_area_min, track_area_max,
									       iview, jplane, jslot);
		}
	      } // fi
	    } // jplane

	    if (IsGoodTrack(plane_condition))
	      position_list.at(iview).push_back(start_of_track_xy + tangent.at(iview) * (0.));

	  } // ivertex
	} // islot
      } // iplane
    } // iview

    // Use lines with good plane condition and reconstruct position
    for (int iview = 0; iview < 2; iview++) {
      if (position_list.at(iview).size() > 0)
	position.at(iview) = (position_list.at(iview).front() + position_list.at(iview).back()) / 2.;
      else { // average of all scintillator bar position in the cluster
	position.at(iview) = 0.;
	for (int ihit = 0; ihit < ntbmsummary->GetNumberOfHits(icluster, iview); ihit++) {
	  TVector3 scintillator_position;
	  B2Dimension::GetPosNinjaTracker((B2View)iview, ntbmsummary->GetPlane(icluster, iview, ihit),
					  ntbmsummary->GetSlot(icluster, iview, ihit),
					  scintillator_position);
	  switch (iview) {
	  case B2View::kTopView :
	    position.at(iview) += scintillator_position.X();
	    break;
	  case B2View::kSideView :
	    position.at(iview) += scintillator_position.Y();
	    break;
	  } // switch
	} // ihit
	position.at(iview) /= ntbmsummary->GetNumberOfHits(icluster, iview);
      } // fi
    } // iview
    
    ntbmsummary->SetNinjaPosition(icluster, position);
  } // icluster
  
}

int main(int argc, char *argv[]) {

  logging::core::get()->set_filter
    (
     logging::trivial::severity >= logging::trivial::debug
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
    NTBMSummary* my_ntbm = nullptr;
    ntbm_tree->Branch("NTBMSummary", &my_ntbm);

    while (reader.ReadNextSpill() > 0) {
      auto &input_spill_summary = reader.GetSpillSummary();
      NTBMSummary* ntbm_tmp = new NTBMSummary();

      // Create NINJA tracker hit cluster
      auto it_hit = input_spill_summary.BeginHit();
      std::vector<const B2HitSummary* > ninja_hits;
      while (const auto *ninja_hit = it_hit.Next()) {
	if(ninja_hit->GetDetectorId() == B2Detector::kNinja)
	  ninja_hits.push_back(ninja_hit);
      }

      // Create X/Y NINJA clusters
      if (ninja_hits.size() > 0) {
	auto i = input_spill_summary.BeginTrueEvent();
	auto *event = i.Next();
	BOOST_LOG_TRIVIAL(debug) << event->GetEventId();
	CreateNinjaCluster(ninja_hits, ntbm_tmp);
      }

      // Extrapolate BabyMIND tracks to the NINJA position
      // and get the best cluster to match each BabyMIND track
      int number_of_tracks = 0;
      auto it_track = input_spill_summary.BeginReconTrack();
      while (const auto *track = it_track.Next()) {
	if (MyHasDetector(track, B2Detector::kBabyMind)) {
	  number_of_tracks++;
	  if (NinjaHitExpected(track)) {
	    //MatchBabyMindTrack(track, ntbm_tmp, my_ntbm);
	  }
	}
      }
      my_ntbm->SetNumberOfTracks(number_of_tracks);
      
      // Update NINJA hit summary information
      ReconstructNinjaPosition(my_ntbm);
      
      // Create output file
      ntbm_tree->Fill();
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
