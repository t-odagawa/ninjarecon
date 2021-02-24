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
#include "B2EmulsionSummary.hh"
#include "B2Pdg.hh"
#include "NTBMSummary.hh"

#include "TrackMatch.hpp"

namespace logging = boost::log;

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

bool CompareNinjaHits(const B2HitSummary* lhs, const B2HitSummary* rhs) {
  if (lhs->GetView()!=rhs->GetView()) return lhs->GetView() < rhs->GetView();
  return GetScintillatorPosition(lhs) < GetScintillatorPosition(rhs);
}

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

std::vector<double> GetPlatePosition(const B2TrackSummary *track, int view, int plane) {

  std::vector<double> position(2);
  int hits = 0;

  auto it_cluster = track->BeginCluster();
  while (const auto *cluster = it_cluster.Next()) {
    auto it_hit = cluster->BeginHit();
    while (const auto *hit = it_hit.Next()) {
      if (hit->GetDetectorId() == B2Detector::kBabyMind) {
	if (hit->GetView() == view &&
	    hit->GetPlane() == plane) {
	  TVector3 babymind_position;
	  B2Dimension::GetBabyMindPosition(track, (B2View)view, plane, hit->GetSlot(),
					   babymind_position);
	  if (view == B2View::kTopView)
	    position.at(0) += babymind_position.GetValue().X();
	  else if (view == B2View::kSideView)
	    position.at(0) += babymind_position.GetValue().Y();
	  position.at(1) += babymind_position.GetValue().Z();
	  hits++
	}

      }
    }
    position.at(0) /= (double) hits; 
    position.at(1) /= (double) hits;
  }

  return position;

}

std::vector<double> GetBabyMindInitialDirection(const B2TrackSummary *track) {

  std::vector<double> babymind_initial_direction(2);


}

std::vector<double> GetBabyMindInitialPosition(const B2TrackSummary *track) {

  std::vector<double> babymind_initial_position(2);
  babymind_initial_position.at(0) = 0.; babymind_initial_position.at(1) = 0.;
  std::vector<int> babymind_hits(2);
  babymind_hits.at(0) = 0; babymind_hits.at(1) = 0;

  auto it_cluster = track->BeginCluster();
  while (const auto *cluster = it_cluster.Next()) {
    auto it_hit = cluster->BeginHit();
    while (const auto *hit = it_hit.Next()) {
      if (hit->GetDetectorId() == B2Detector::kBabyMind) {
	if (hit->GetView() == B2View::kSideView
	    && hit->GetPlane() == 0) {
	  babymind_initial_position.at(0) += hit->GetAbsolutePosition().GetValue().Y();
	  babymind_hits.at(0)++;
	}
	else if (hit->GetView() == B2View::kTopView 
		 && hit->GetPlane() == 1) {
	  babymind_initial_position.at(1) += hit->GetAbsolutePosition().GetValue().X();
	  babymind_hits.at(1)++;
	}
      }
    }
  }

  babymind_initial_position.at(0) /= (double) babymind_hits.at(0);
  babymind_initial_position.at(1) /= (double) babymind_hits.at(1);
  return babymind_initial_position;
}

bool NinjaHitExpected(const B2TrackSummary *track) {

  // Extrapolated position

  if (track->GetType() == B2TrackedParticle::kPrimaryTrack) {
    // Downstream WAGASCI interaction
    return false;
  }
  return true; // TODO
}

void MatchBabyMindTrack(const B2TrackSummary *track, NTBMSummary* ntbm_in, NTBMSummary* ntbm_out) {
  
}

bool IsInRange(double pos, double min, double max) {
  if (min <= pos && pos <= max) return true;
  else return false;
}

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

double GetTrackAreaMin(double pos, double tangent, int iplane, int jplane, int vertex) {
  if (tangent > 0) return pos + tangent * (NINJA_TRACKER_OFFSET_Z[jplane] - NINJA_TRACKER_OFFSET_Z[iplane]
					   + NINJA_TRACKER_SCI_THICK * (0 - vertex % 2));
  else return pos + tangent * (NINJA_TRACKER_OFFSET_Z[jplane] - NINJA_TRACKER_OFFSET_Z[iplane]
			       + NINJA_TRACKER_SCI_THICK * (1 - vertex % 2));
}

double GetTrackAreaMax(double pos, double tangent, int iplane, int jplane, int vertex) {
  if (tangent > 0) return pos + tangent * (NINJA_TRACKER_OFFSET_Z[jplane] - NINJA_TRACKER_OFFSET_Z[iplane]
					   + NINJA_TRACKER_SCI_THICK * (1 - vertex % 2));
  else return pos + tangent * (NINJA_TRACKER_OFFSET_Z[jplane] - NINJA_TRACKER_OFFSET_Z[iplane]
			       + NINJA_TRACKER_SCI_THICK * (0 - vertex % 2));
}

bool IsGoodTrack(bool *condition) {
  bool ret = true;
  for (int iplane = 0; iplane < NINJA_TRACKER_NUM_PLANES; iplane++)
    ret = ret && condition[iplane];
  return ret;
}

void ReconstructNinjaTangent(NTBMSummary* ntbmsummary) {

  for (int icluster = 0; icluster < ntbmsummary->GetNumberOfNinjaClusters(); icluster++) {

    std::vector<double> tangent(2);
    int trackid = ntbmsummary->GetBabyMindTrackId(icluster);
    std::vector<double> baby_mind_position = ntbmsummary->GetBabyMindPosition(trackid);
    std::vector<double> ninja_position_tmp = ntbmsummary->GetNinjaPosition(icluster);
    for (int iview = 0; iview < 2; iview++) {
      tangent.at(iview) = (baby_mind_position.at(iview) - ninja_position_tmp.at(iview))
	/ NINJA_BABYMIND_DISTANCE[iview];
    }
    ntbmsummary->SetNinjaTangent(icluster, tangent);
  }

}

void ReconstructNinjaPosition(NTBMSummary* ntbmsummary) {

  for (int icluster = 0; icluster < ntbmsummary->GetNumberOfNinjaClusters(); icluster++) {
 
    std::vector<double> tangent = ntbmsummary->GetNinjaTangent(icluster);
    // coordinate direction is fliped in reconstruction X coordinate
    tangent.at(B2View::kTopView) = -tangent.at(B2View::kTopView);

    std::vector<double> position(2);
    std::vector<std::vector<double>> position_list = {}; // vector where good position candidates are filled

    for (int iview = 0; iview < 2; iview++) {
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
    
    position.at(B2View::kSideView) -= NINJA_SCI_DIFF;
    position.at(B2View::kTopView) = -position.at(B2View::kTopView);
    position.at(B2View::kTopView) += NINJA_SCI_DIFF;

    ntbmsummary->SetNinjaPosition(icluster, position);
  } // icluster
  
}

void SetTruePositionAngle(B2SpillSummary* spill_summary, NTBMSummary* ntbmsummary) {

  auto it_emulsion = spill_summary->BeginEmulsion();
  while (const auto *emulsion = it_emulsion.Next()) {
    // Get position of TSS downstream film position
    if (emulsion->GetFilmType() == B2EmulsionType::kShifter && emulsion->GetPlate() == 15) {
      int particle_id = emulsion->GetParentTrack().GetParticlePdg();
      TVector3 true_position = emulsion->GetAbsolutePosition().GetValue();
      if (!B2Pdg::IsMuon(particle_id)) continue;

      // Get most muon-like cluster and set true info
      for (int icluster = 0; icluster < ntbmsummary->GetNumberOfNinjaClusters(); icluster++) {
	std::vector<double> ninja_position = ntbmsummary->GetNinjaPosition(icluster);

      }

    }
  }
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
      if (ninja_hits.size() > 0)
	CreateNinjaCluster(ninja_hits, ntbm_tmp);

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
      ReconstructNinjaTangent(my_ntbm); // reconstruct tangent
      ReconstructNinjaPosition(my_ntbm); // use reconstructed tangent info
      
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
