// system includes
#include <vector>
#include <numeric>
#include <algorithm>
#include <iostream>

// boost includes
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

// root includes
#include <TGraphErrors.h>
#include <TF1.h>
#include <TString.h>
#include <TCanvas.h>

// B2 includes
#include "B2Reader.hh"
#include "B2Writer.hh"
#include "B2Enum.hh"
#include "B2Dimension.hh"
#include "B2SpillSummary.hh"
#include "B2BeamSummary.hh"
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

bool CompareNinjaHits(const B2HitSummary* lhs, const B2HitSummary* rhs) {
  if (lhs->GetView()!=rhs->GetView()) return lhs->GetView() < rhs->GetView();
  switch (lhs->GetView()) {
  case B2View::kSideView :
    return lhs->GetScintillatorPosition().GetValue().Y()
      < rhs->GetScintillatorPosition().GetValue().Y();
  case B2View::kTopView :
    return lhs->GetScintillatorPosition().GetValue().X()
      < rhs->GetScintillatorPosition().GetValue().X();
  default :
    throw std::invalid_argument("View not valid : " + lhs->GetView());
  }
}

void CreateNinjaCluster(std::vector<const B2HitSummary* > ninja_hits,
			NTBMSummary* ninja_clusters) {
  std::sort(ninja_hits.begin(), ninja_hits.end(), CompareNinjaHits);

  double scintillator_position_tmp = -9999.;
  int view_tmp = -1;
  
  int number_of_ninja_clusters = 0;
  std::vector<int> number_of_hits_tmp(2, 0);
  std::vector<std::vector<int>> number_of_hits = {};
  std::vector<std::vector<int>> plane_tmp, slot_tmp;
  std::vector<std::vector<double>> pe_tmp;
  plane_tmp.resize(2); slot_tmp.resize(2);
  pe_tmp.resize(2);
  std::vector<std::vector<std::vector<int>>> plane = {}, slot = {};
  std::vector<std::vector<std::vector<double>>> pe = {};

  for (const auto ninja_hit : ninja_hits) {
    double ninja_hit_position_tmp_ = 0.;
    if (ninja_hit->GetView() == B2View::kSideView)
      ninja_hit_position_tmp_ = ninja_hit->GetScintillatorPosition().GetValue().Y();
    else if (ninja_hit->GetView() == B2View::kTopView)
      ninja_hit_position_tmp_ = ninja_hit->GetScintillatorPosition().GetValue().X();

    // when scintillators have a gap, create new NINJA cluster
    if ( ( ninja_hit != ninja_hits.front() )
	&& ( ninja_hit_position_tmp_ >  scintillator_position_tmp + NINJA_TRACKER_SCI_WIDTH
	     || ninja_hit->GetView() != view_tmp || ninja_hit == ninja_hits.back()) ) {

      if (ninja_hit == ninja_hits.back()) { // push back at the end of the vector
	number_of_hits_tmp.at(ninja_hit->GetView())++;
	plane_tmp.at(ninja_hit->GetView()).push_back(ninja_hit->GetPlane());
	slot_tmp.at(ninja_hit->GetView()).push_back(ninja_hit->GetSlot().GetValue(ninja_hit->GetSingleReadout()));
	pe_tmp.at(ninja_hit->GetView()).push_back(ninja_hit->GetHighGainPeu().GetValue(ninja_hit->GetSingleReadout()));	
      }

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

    scintillator_position_tmp = ninja_hit_position_tmp_;
    view_tmp = ninja_hit->GetView();

  }

  std::vector<double> tangent(2);
  tangent.at(0) = 0; tangent.at(1) = 0;
  ninja_clusters->SetNumberOfNinjaClusters(number_of_ninja_clusters);
  for(int icluster = 0; icluster < number_of_ninja_clusters; icluster++) {
    ninja_clusters->SetBabyMindTrackId(icluster, -1);
    ninja_clusters->SetNumberOfHits(icluster, number_of_hits.at(icluster));
    ninja_clusters->SetPlane(icluster, plane.at(icluster));
    ninja_clusters->SetSlot(icluster, slot.at(icluster));
    ninja_clusters->SetPe(icluster, pe.at(icluster));
    ninja_clusters->SetNinjaTangent(icluster, tangent);
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

int GetBabyMindPlaneHits(const B2TrackSummary *track, int view, int plane) {

  int number_of_hits = 0;

  auto it_cluster = track->BeginCluster();
  while (const auto *cluster = it_cluster.Next()) {
    auto it_hit = cluster->BeginHit();
    while (const auto *hit = it_hit.Next()) {
      if (hit->GetDetectorId() == B2Detector::kBabyMind)
	if (hit->GetView() == view &&
	    hit->GetPlane() == plane)
	  number_of_hits++;
    }
  }

  return number_of_hits;
}

std::vector<double> GetBabyMindPlanePosition(const B2TrackSummary *track, int view, int plane) {

  std::vector<double> position(2); // 0:X/Y, 1:Z
  for (int i = 0; i < 2; i++)
    position.at(i) = 0.;

  int number_of_hits = GetBabyMindPlaneHits(track, view, plane);
  if(number_of_hits == 0) return position;

  auto it_cluster = track->BeginCluster();
  while (const auto *cluster = it_cluster.Next()) {
    auto it_hit = cluster->BeginHit();
    while (const auto *hit = it_hit.Next()) {
      if (hit->GetDetectorId() == B2Detector::kBabyMind) {
	if (hit->GetView() == view &&
	    hit->GetPlane() == plane) {
	  if (view == B2View::kTopView)
	    position.at(0) += hit->GetScintillatorPosition().GetValue().X();
	  else if (view == B2View::kSideView)
	    position.at(0) += hit->GetScintillatorPosition().GetValue().Y();

	  position.at(1) += hit->GetScintillatorPosition().GetValue().Z();
	}
      }
    }

    for (int i = 0; i < 2; i++) position.at(i) /= number_of_hits;
  }

  return position;

}

std::vector<double> GetBabyMindPlanePositionError(const B2TrackSummary *track, int view, int plane) {

  std::vector<double> position_error(2); // 0:X/Y, 1:Z
  for (int i = 0; i < 2; i++)
    position_error.at(i) = 0.;

  int number_of_hits = GetBabyMindPlaneHits(track, view, plane);
  if (number_of_hits == 0) return position_error;

  switch (view) {
  case B2View::kSideView :
    if (number_of_hits == 1) {
      position_error.at(0) = 0.5 * BM_HORIZONTAL_SCINTI_LARGE / 3.;
      position_error.at(1) = 0.5 * BM_HORIZONTAL_SCINTI_THICK;
    }
    else if (number_of_hits == 2) {
      position_error.at(0) = 0.5 * BM_HORIZONTAL_SCINTI_LARGE / 3.;
      position_error.at(1) = BM_HORIZONTAL_SCINTI_THICK;
    }
    else {
      position_error.at(0) = 0.5 * (BM_HORIZONTAL_SCINTI_LARGE * (2 * number_of_hits - 1) / 3.);
      position_error.at(1) = BM_HORIZONTAL_SCINTI_THICK;
    }
    break;
  case B2View::kTopView :
    if (number_of_hits == 1) {
      position_error.at(0) = 0.5 * (BM_VERTICAL_SCINTI_LARGE - 2 * BM_VERTICAL_SCINTI_OVERLAP);
      position_error.at(1) = 0.5 * BM_VERTICAL_SCINTI_THICK;
    }
    else if (number_of_hits == 2) {
      position_error.at(0) = 0.5 * BM_VERTICAL_SCINTI_OVERLAP;
      position_error.at(1) = 20.;
    }
    else {
      position_error.at(0) = 0.5 * (BM_VERTICAL_SCINTI_LARGE * number_of_hits - BM_VERTICAL_SCINTI_OVERLAP * (number_of_hits + 1));
      position_error.at(1) = 20.;
    }
    break;
  }
  
  return position_error;

}

std::vector<double> FitBabyMindTopView(const B2TrackSummary *track, TCanvas *c, int entry, bool draw) {

  std::vector<double> param(2);

  std::vector<double> x, z;
  std::vector<double> x_error, z_error;
  for (int iplane = 0; iplane < BM_NUM_TRACKING_PLANES; iplane++) {
    if (GetBabyMindPlaneHits(track, B2View::kTopView, iplane)) {
      x.push_back(GetBabyMindPlanePosition(track, B2View::kTopView, iplane).at(0));
      z.push_back(GetBabyMindPlanePosition(track, B2View::kTopView, iplane).at(1));
      x_error.push_back(GetBabyMindPlanePositionError(track, B2View::kTopView, iplane).at(0));
      z_error.push_back(GetBabyMindPlanePositionError(track, B2View::kTopView, iplane).at(1));
    }
  }
  BOOST_LOG_TRIVIAL(debug) << "Track type : " << track->GetType();
  BOOST_LOG_TRIVIAL(debug) << "Number of hit planes : " << z.size();

  if (z.size() == 0) {
    param.at(0) = 0.; param.at(1) = 0.;
    return param;
  }
  TGraphErrors *ge = new TGraphErrors(z.size(), &z[0], &x[0], &z_error[0], &x_error[0]);

#ifdef CANVAS
  if (draw) {
    ge->SetTitle(Form("Entry : %d", entry));
    ge->Draw("AP");
  }
#endif

  TF1 *linear = new TF1("linear", "[0] + [1] * x");
  linear->SetParameter(0, x.front());
  linear->SetParameter(1, (x.back() - x.front()) / (z.back() - z.front()));

  ge->Fit(linear, "Q", "", z.front() - 100., z.back()+100.);
  param.at(0) = linear->GetParameter(0);
  param.at(1) = linear->GetParameter(1);

#ifdef CANVAS
  if (draw)
    c->Print((TString)"test.pdf", "pdf");
#endif

  delete linear;

  return param;

}

std::vector<double> GetBabyMindInitialDirection(const B2TrackSummary *track, TCanvas *c, int entry, bool draw) {
  std::vector<double> initial_direction(2); // 0:X, 1:Y

  // Vertical direction
  if (GetBabyMindPlaneHits(track, B2View::kSideView, 0) > 0 &&
      GetBabyMindPlaneHits(track, B2View::kSideView, 1) > 0)
    initial_direction.at(1) = (GetBabyMindPlanePosition(track, B2View::kSideView, 1).at(0) - GetBabyMindPlanePosition(track, B2View::kSideView, 0).at(0)) / (GetBabyMindPlanePosition(track, B2View::kSideView, 1).at(1) - GetBabyMindPlanePosition(track, B2View::kSideView, 0).at(1));
  else 
    initial_direction.at(1) = 0.;

  // Horizontal direction
  std::vector<double> param = FitBabyMindTopView(track, c, entry, draw);
  initial_direction.at(0) = param.at(1);

  return initial_direction;

}

std::vector<double> GetBabyMindInitialPosition(const B2TrackSummary *track, int view, TCanvas *c, int entry, bool draw) {
  std::vector<double> initial_position(2); // 0:X/Y, 1:Z

  if (view == B2View::kSideView)
    initial_position = GetBabyMindPlanePosition(track, view, 0);
  else if (view == B2View::kTopView) {
    std::vector<double> param = FitBabyMindTopView(track, c, entry, draw);
    initial_position.at(0) = param.at(0) + param.at(1) * 0.;
    initial_position.at(1) = 0.;
  }
    
  return initial_position;

}

std::vector<double> CalculateExpectedPosition(const B2TrackSummary *track) {

  TCanvas *tmp = new TCanvas();

  std::vector<double> initial_direction =  GetBabyMindInitialDirection(track, tmp, 0, false);
  std::vector<double> initial_position_x = GetBabyMindInitialPosition(track, B2View::kTopView, tmp, 0, false);
  std::vector<double> initial_position_y = GetBabyMindInitialPosition(track, B2View::kSideView, tmp, 0, false);

  std::vector<double> position(2);

  position.at(B2View::kTopView)  = initial_position_x.at(0) - initial_direction.at(B2View::kTopView) * 0.;
  position.at(B2View::kSideView) = initial_position_y.at(0) - initial_direction.at(B2View::kSideView) * 0.;

  delete tmp;

  return position;

}

bool NinjaHitExpected(const B2TrackSummary *track, TCanvas *c, int entry) {

  // Extrapolated position
  std::vector<double> hit_expected_position = CalculateExpectedPosition(track);

  if (std::fabs(hit_expected_position.at(B2View::kTopView)  - 0.) > 0. ||
      std::fabs(hit_expected_position.at(B2View::kSideView) - 0.) > 0.)
    return false;

  // Has hits in upstream planes
  if ((GetBabyMindPlaneHits(track, B2View::kSideView, 0) == 0 &&
       GetBabyMindPlaneHits(track, B2View::kSideView, 1) == 0) ||
      (GetBabyMindPlaneHits(track, B2View::kTopView, 0) == 0 &&
       GetBabyMindPlaneHits(track, B2View::kTopView, 1) == 0))
    return false;

  // Downstream WAGASCI interaction TODO
  if (track->GetType() == B2TrackedParticle::kPrimaryTrack) {
    if (false) return false;
  }

  return true; // TODO
}

void MatchBabyMindTrack(const B2TrackSummary *track, NTBMSummary* ntbm_in, NTBMSummary* ntbm_out) {

  std::vector<double> hit_expected_position = CalculateExpectedPosition(track);

  std::vector<int> matched_cluster_tmp(2);
  std::vector<double> position_difference_tmp(2);
  for (int view = 0; view < 2; view++)
    position_difference_tmp.at(view) = 50.;

  for (int icluster = 0; icluster < ntbm_in->GetNumberOfNinjaClusters(); icluster++) {
    std::vector<int> number_of_hits = ntbm_in->GetNumberOfHits(icluster);
    std::vector<double> ninja_position = ntbm_in->GetNinjaPosition(icluster);

    // Get view information from 1d NINJA cluster
    int view = -1;
    if (number_of_hits.at(B2View::kTopView) > 0) {
      if (number_of_hits.at(B2View::kSideView) > 0) continue;
      else view = B2View::kTopView;
    } else if (number_of_hits.at(B2View::kSideView > 0))
      view = B2View::kSideView;
    else continue;

    std::vector<double> ninja_tangent = ntbm_in->GetNinjaTangent(icluster);

    if (std::fabs(hit_expected_position.at(view) - ninja_position.at(view))
	< position_difference_tmp.at(view)) {
      matched_cluster_tmp.at(view) = icluster;
      position_difference_tmp.at(view) = std::fabs(hit_expected_position.at(view) - ninja_position.at(view));
    }
  
  }

  if (false) return;

  // Create a new 2d cluster and add it
  std::vector<int> number_of_hits(2);
  std::vector<std::vector<int>> plane(2);
  std::vector<std::vector<int>> slot(2);
  std::vector<std::vector<double>> pe(2);
  std::vector<double> ninja_position(2);
  std::vector<double> ninja_tangent(2);

  ntbm_in->SetNumberOfNinjaClusters(ntbm_in->GetNumberOfNinjaClusters() + 1);
  for (int view = 0; view < 2; view++) {
    number_of_hits.at(view) = ntbm_in->GetNumberOfHits(matched_cluster_tmp.at(view), view);
    ninja_position.at(view) = ntbm_in->GetNinjaPosition(matched_cluster_tmp.at(view)).at(view);
    ninja_tangent.at(view) = ntbm_in->GetNinjaTangent(matched_cluster_tmp.at(view)).at(view);
    for (int hit = 0; hit < number_of_hits.at(view); hit++) {
      plane.at(view).push_back(ntbm_in->GetPlane(matched_cluster_tmp.at(view), view, hit));
      slot.at(view).push_back(ntbm_in->GetSlot(matched_cluster_tmp.at(view), view, hit));
      pe.at(view).push_back(ntbm_in->GetPe(matched_cluster_tmp.at(view), view, hit));
    }
  }

}

bool IsInRange(double pos, double min, double max) {
  if (min <= pos && pos <= max) return true;
  else return false;
}

bool IsMakeHit(double min, double max, int view, int plane, int slot) {
  TVector3 position;
  B2Dimension::GetPosNinjaTracker((B2View)view, plane, slot, position);
  double position_xy = 0.;
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
  double position_xy = 0.;
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
	    case B2View::kTopView :
	      start_of_track_xy = start_of_track.X()
		+ NINJA_TRACKER_SCI_WIDTH / 2. * ( -1 + 2 * (ivertex/2) );
	      break;
	    case B2View::kSideView :
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

void TransferBeamInfo(const B2SpillSummary &spill_summary, NTBMSummary *ntbm_summary) {
  auto beam_summary = spill_summary.GetBeamSummary();
  ntbm_summary->SetSpillPot(beam_summary.GetSpillPot());
  for (std::size_t bunch = 0; bunch < 8; bunch++)
    ntbm_summary->SetBunchPot(bunch, beam_summary.GetBunchPot(bunch));
  ntbm_summary->SetBsdSpillNumber(beam_summary.GetBsdSpillNumber());
  ntbm_summary->SetTimestamp(beam_summary.GetTimestamp());
  ntbm_summary->SetBsdGoodSpillFlag(beam_summary.GetBsdGoodSpillFlag());
  ntbm_summary->SetWagasciGoodSpillFlag(beam_summary.GetWagasciGoodSpillFlag());
}

void TransferBabyMindTrackInfo(const B2SpillSummary &spill_summary, NTBMSummary *ntbm_summary) {

  auto it_track = spill_summary.BeginReconTrack();
  int itrack = 0;
  TCanvas *tmp = new TCanvas();
  while (const auto *track = it_track.Next()) {
    if (MyHasDetector(track, B2Detector::kBabyMind)) {
      ntbm_summary->SetTrackType(itrack, track->GetType());
      ntbm_summary->SetMomentum(itrack, track->GetInitialAbsoluteMomentum().GetValue());
      ntbm_summary->SetMomentumError(itrack, track->GetInitialAbsoluteMomentum().GetError());
      for (int view = 0; view < 2; view++) {
	ntbm_summary->SetBabyMindPosition(itrack, view,
					  GetBabyMindInitialPosition(track, view, tmp, 0, false).at(0));
      }
      ntbm_summary->SetBabyMindTangent(itrack, GetBabyMindInitialDirection(track, tmp, 0, false));
      itrack++;
    }
  } 

  delete tmp;
}

int main(int argc, char *argv[]) {

#ifdef CANVAS
  gErrorIgnoreLevel = kWarning;
#endif

  logging::core::get()->set_filter
    (
     logging::trivial::severity >= logging::trivial::info
     //logging::trivial::severity >= logging::trivial::debug
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

    TCanvas *c = new TCanvas("c", "c");
#ifdef CANVAS
    c->Print((TString)"test.pdf" + "[", "pdf");
#endif

    int nspill = 0;

    while (reader.ReadNextSpill() > 0) {
      //if (nspill > 1000) break;
      nspill++;

      auto &input_spill_summary = reader.GetSpillSummary();
      int entry = input_spill_summary.GetBeamSummary().GetTimestamp();
      BOOST_LOG_TRIVIAL(debug) << "timestamp : " << entry;

      NTBMSummary* ntbm_tmp = new NTBMSummary();
 
      TransferBeamInfo(input_spill_summary, my_ntbm);

      // Create NINJA tracker hit cluster
      auto it_hit = input_spill_summary.BeginHit();
      std::vector<const B2HitSummary* > ninja_hits;
      while (const auto *ninja_hit = it_hit.Next()) {
	if(ninja_hit->GetDetectorId() == B2Detector::kNinja)
	  ninja_hits.push_back(ninja_hit);
      }

      // Create X/Y NINJA clusters
      if (ninja_hits.size() > 0)
	//CreateNinjaCluster(ninja_hits, ntbm_tmp);
	CreateNinjaCluster(ninja_hits, my_ntbm);

      // Extrapolate BabyMIND tracks to the NINJA position
      // and get the best cluster to match each BabyMIND track
      int number_of_tracks = 0;
      auto it_track = input_spill_summary.BeginReconTrack();
      while (const auto *track = it_track.Next()) {
	if (MyHasDetector(track, B2Detector::kBabyMind)) {
	  number_of_tracks++;
	  if (NinjaHitExpected(track, c, entry)) {
	    // MatchBabyMindTrack(track, ntbm_tmp, my_ntbm);
	  }
	}
      }
      my_ntbm->SetNumberOfTracks(number_of_tracks);
      if (number_of_tracks > 0)
	TransferBabyMindTrackInfo(input_spill_summary, my_ntbm);
      
      // Update NINJA hit summary information
      //ReconstructNinjaTangent(my_ntbm); // reconstruct tangent
      //ReconstructNinjaPosition(my_ntbm); // use reconstructed tangent info
      
      // Create output file
      ntbm_tree->Fill();
      my_ntbm->Clear("C");
    }

#ifdef CANVAS
    c->Print((TString)"test.pdf" + "]", "pdf");
#endif
    delete c;

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
