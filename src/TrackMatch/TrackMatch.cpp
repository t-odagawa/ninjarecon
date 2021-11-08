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
#include <TGraphAsymmErrors.h>
#include <TF1.h>
#include <TString.h>
#include <TCanvas.h>

// B2 includes
#include <B2Reader.hh>
#include <B2Writer.hh>
#include <B2Enum.hh>
#include <B2Dimension.hh>
#include <B2SpillSummary.hh>
#include <B2BeamSummary.hh>
#include <B2HitSummary.hh>
#include <B2VertexSummary.hh>
#include <B2ClusterSummary.hh>
#include <B2TrackSummary.hh>
#include <B2EventSummary.hh>
#include <B2EmulsionSummary.hh>
#include <B2Pdg.hh>
#include "NTBMSummary.hh"

#include "TrackMatch.hpp"

namespace logging = boost::log;

// Comparator for sort functions

bool CompareNinjaHits(const B2HitSummary* lhs, const B2HitSummary* rhs) {
  if ( lhs->GetBunch() != rhs->GetBunch() )
    return lhs->GetBunch() < rhs->GetBunch();
  if ( lhs->GetView()  != rhs->GetView() )
    return lhs->GetView() < rhs->GetView();
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

bool CompareBabyMindHitsInOneTrack(const B2HitSummary* lhs, const B2HitSummary *rhs) {
  if ( lhs->GetView()  != rhs->GetView() )
    return lhs->GetView() < rhs->GetView();
  if ( lhs->GetPlane() != rhs->GetPlane() )
    return lhs->GetPlane() < rhs->GetPlane();
  if ( lhs->GetSlot().GetValue(lhs->GetSingleReadout()) != rhs->GetSlot().GetValue(rhs->GetSingleReadout()) )
    return lhs->GetSlot().GetValue(lhs->GetSingleReadout())
      < rhs->GetSlot().GetValue(rhs->GetSingleReadout());
}

// NINJA cluster creation

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
  std::vector<int> bunch_difference = {};

  for ( int ihit = 0; ihit < ninja_hits.size(); ihit++ ) {
    const auto ninja_hit = ninja_hits.at(ihit);
    double ninja_hit_position = 0.;
    if ( ninja_hit->GetView() == B2View::kSideView )
      ninja_hit_position = ninja_hit->GetScintillatorPosition().GetValue().Y();
    else if ( ninja_hit->GetView() == B2View::kTopView ) 
      ninja_hit_position = ninja_hit->GetScintillatorPosition().GetValue().X();

    int view_next = -1;
    int bunch_difference_next = -1;
    double ninja_hit_next_position = 0.;
    if ( ihit != ninja_hits.size() - 1 ) {
      const auto ninja_hit_next = ninja_hits.at(ihit + 1);
      view_next = ninja_hit_next->GetView();
      bunch_difference_next = ninja_hit_next->GetBunch();
      if ( ninja_hit_next->GetView() == B2View::kSideView )
	ninja_hit_next_position = ninja_hit_next->GetScintillatorPosition().GetValue().Y();
      else if ( ninja_hit_next->GetView() == B2View::kTopView )
	ninja_hit_next_position = ninja_hit_next->GetScintillatorPosition().GetValue().X();
    }

    number_of_hits_tmp.at(ninja_hit->GetView())++;
    plane_tmp.at(ninja_hit->GetView()).push_back(ninja_hit->GetPlane());
    slot_tmp.at(ninja_hit->GetView()).push_back(ninja_hit->GetSlot().GetValue(ninja_hit->GetSingleReadout()));
    if ( ninja_hit->GetBunch() == 0 )
      pe_tmp.at(ninja_hit->GetView()).push_back(ninja_hit->GetHighGainPeu().GetValue(ninja_hit->GetSingleReadout()));
    else 
      pe_tmp.at(ninja_hit->GetView()).push_back(ninja_hit->GetTimeNs().GetValue(ninja_hit->GetSingleReadout()));
    
    // create a new NINJA cluster
    if ( ( ( ihit < ninja_hits.size() - 1 ) && 
	   ( ninja_hit_next_position > ninja_hit_position + NINJA_SCI_WIDTH // when there is a gap
	     || view_next != ninja_hit->GetView() // when view is changed
	     || bunch_difference_next != ninja_hit->GetBunch() ) ) // when bunch difference is changed
	 || ( ihit == ninja_hits.size() - 1 ) ) { // when it is the last hit
      number_of_ninja_clusters++;
      number_of_hits.push_back(number_of_hits_tmp); number_of_hits_tmp.assign(2,0);
      plane.push_back(plane_tmp);
      plane_tmp.at(0) = {}; plane_tmp.at(1) = {};
      slot.push_back(slot_tmp);
      slot_tmp.at(0) = {}; slot_tmp.at(1) = {};
      pe.push_back(pe_tmp);
      pe_tmp.at(0) = {}; pe_tmp.at(1) = {};
      bunch_difference.push_back(ninja_hit->GetBunch());
    }

  }

  std::vector<double> tangent(2);
  tangent.at(0) = 0.; tangent.at(1) = 0.;
  ninja_clusters->SetNumberOfNinjaClusters(number_of_ninja_clusters);
  for(int icluster = 0; icluster < number_of_ninja_clusters; icluster++) {
    ninja_clusters->SetBabyMindTrackId(icluster, -1);
    ninja_clusters->SetNumberOfHits(icluster, number_of_hits.at(icluster));
    ninja_clusters->SetPlane(icluster, plane.at(icluster));
    ninja_clusters->SetSlot(icluster, slot.at(icluster));
    ninja_clusters->SetPe(icluster, pe.at(icluster));
    ninja_clusters->SetBunchDifference(icluster, bunch_difference.at(icluster));
    ninja_clusters->SetNinjaTangent(icluster, tangent);
  }

  BOOST_LOG_TRIVIAL(debug) << "NINJA tracker clusters created";
}

std::vector<std::vector<double> > CalcMergedOnePlanePositionAndError(std::vector<std::vector<double> > position, int view) {

  std::vector<double> xy_position = position.at(0);
  std::sort(xy_position.begin(), xy_position.end());
  std::vector<double> z_position = position.at(1);
  std::sort(z_position.begin(), z_position.end());

  std::vector<std::vector<double> > position_and_error(3);
  for (int i = 0; i < 3; i++) position_and_error.at(i).resize(2);
  // position_and_error.at(pos/higherr/lowerr).at(xy/z)
  const std::size_t number_of_hits = xy_position.size();

  // Calculate position
  // X/Y
  position_and_error.at(0).at(0) = std::accumulate(xy_position.begin(), xy_position.end(), 0.);
  position_and_error.at(0).at(0) /= (double) number_of_hits;
  // Z
  position_and_error.at(0).at(1) = ( z_position.front() + z_position.back() ) / 2.;

  // Calculate error
  switch (view) {
    double xy_area_max, xy_area_min;
    double z_area_max, z_area_min;
  case B2View::kSideView :
    xy_area_max = xy_position.back()  + 0.5 * BM_HORIZONTAL_SCINTI_LARGE / 3.;
    xy_area_min = xy_position.front() - 0.5 * BM_HORIZONTAL_SCINTI_LARGE / 3.;
    z_area_max = z_position.back()  + 0.5 * BM_HORIZONTAL_SCINTI_THICK;
    z_area_min = z_position.front() - 0.5 * BM_HORIZONTAL_SCINTI_THICK;
    // y errors
    position_and_error.at(1).at(0) = xy_area_max - position_and_error.at(0).at(0);
    position_and_error.at(2).at(0) = position_and_error.at(0).at(0) - xy_area_min;
    // z errors
    position_and_error.at(1).at(1) = z_area_max - position_and_error.at(0).at(1);
    position_and_error.at(2).at(1) = position_and_error.at(0).at(1) - z_area_min;
    break;
  case B2View::kTopView :
    if ( xy_position.size() == 2 &&
	 std::fabs( xy_position.front() - xy_position.back() ) < BM_VERTICAL_SCINTI_LARGE ) {
      double overlap = BM_VERTICAL_SCINTI_LARGE - std::fabs( xy_position.front() - xy_position.back() );
      xy_area_max = position_and_error.at(0).at(0) + 0.5 * overlap;
      xy_area_min = position_and_error.at(0).at(0) - 0.5 * overlap;
    } else {
      xy_area_max = xy_position.back()  + 0.5 * BM_VERTICAL_SCINTI_LARGE;
      xy_area_min = xy_position.front() - 0.5 * BM_VERTICAL_SCINTI_LARGE;
    }
    z_area_max = z_position.back()  + 0.5 * BM_VERTICAL_SCINTI_THICK;
    z_area_min = z_position.front() - 0.5 * BM_VERTICAL_SCINTI_THICK;
    // x errors
    position_and_error.at(1).at(0) = xy_area_max - position_and_error.at(0).at(0);
    position_and_error.at(2).at(0) = position_and_error.at(0).at(0) - xy_area_min;
    // z errors
    position_and_error.at(1).at(1) = z_area_max - position_and_error.at(0).at(1);
    position_and_error.at(2).at(1) = position_and_error.at(0).at(1) - z_area_min;
    break;
  default :
    BOOST_LOG_TRIVIAL(error) << "View is not correctly assigned : " << view;
    std::exit(1);
  }

  BOOST_LOG_TRIVIAL(trace) << "Position (XY) : "   << position_and_error.at(0).at(0) << ", "
			   << "Position (Z) : "    << position_and_error.at(0).at(1) << ", "
			   << "Error (XY high) : " << position_and_error.at(1).at(0) << ", "
			   << "Error (XY low) : "  << position_and_error.at(2).at(0) << ", "
			   << "Error (Z) : "       << position_and_error.at(1).at(1);

  return position_and_error;

}


std::vector<std::vector<std::vector<std::vector<double> > > > GenerateMergedPositionAndErrors(std::vector<const B2HitSummary* > hits, int datatype){

  std::sort(hits.begin(), hits.end(), CompareBabyMindHitsInOneTrack);

  BOOST_LOG_TRIVIAL(trace) << "New track information with hits";
  BOOST_LOG_TRIVIAL(trace) << "Number of Baby MIND hits used for fitting : " << hits.size();

  std::vector<std::vector<std::vector<std::vector<double> > > > merged_position_and_error(2);
  // merged_position_and_error.at(view).at(pos/higherr/lowerr).at(xy/z).at(plane)
  for ( int iview = 0; iview < 2; iview++ ) {
    merged_position_and_error.at(iview).resize(3);
    for ( int iposerr = 0; iposerr < 3; iposerr++ ) {
      merged_position_and_error.at(iview).at(iposerr).resize(2);
    }
  }

  std::vector<std::vector<double> > position_tmp(2);
  // position_tmp.at(xy/z).at(hits)

  for ( int ihit = 0; ihit < hits.size(); ihit++ ) {
    const auto hit = hits.at(ihit);

    int view = hit->GetView();
    int plane = hit->GetPlane();
    int channel = hit->GetSlot().GetValue(hit->GetSingleReadout());
    BOOST_LOG_TRIVIAL(trace) << "Detector : " << DETECTOR_NAMES.at(hit->GetDetectorId()) << ", "
			     << "View : "     << VIEW_NAMES.at(hit->GetView()) << ", "
			     << "Plane : "    << hit->GetPlane() << ", "
			     << "Channel : "  << hit->GetSlot().GetValue(hit->GetSingleReadout());

    const TVector3 &pos = hit->GetScintillatorPosition().GetValue();

    if ( datatype == B2DataType::kRealData && plane >= 2 )
      position_tmp.at(1).push_back(pos.Z() + BM_SCI_CORRECTION);
    else
      position_tmp.at(1).push_back(pos.Z());

    switch (view) {
    case B2View::kSideView :
      position_tmp.at(0).push_back(pos.Y());
      break;
    case B2View::kTopView : 
      position_tmp.at(0).push_back(pos.X());
      break;
    default :
      BOOST_LOG_TRIVIAL(error) << "View is not correctly assigned";
      std::exit(1);
    }

    if ( ( hit != hits.back() &&
	   ( view != hits.at(ihit+1)->GetView() ||
	     plane != hits.at(ihit+1)->GetPlane() ) ) ||
	 hit == hits.back() ) {
      std::vector<std::vector<double> > one_plane_pos_and_err = CalcMergedOnePlanePositionAndError(position_tmp, view);
      for ( int iposerr = 0; iposerr < 3; iposerr++ )
	for ( int ixyz = 0; ixyz < 2; ixyz++ )
      merged_position_and_error.at(view).at(iposerr).at(ixyz).push_back(one_plane_pos_and_err.at(iposerr).at(ixyz));
      position_tmp.clear(); position_tmp.resize(2);
    }
    
  }

  return merged_position_and_error;

}


std::vector<std::vector<double> > FitBabyMind(const B2TrackSummary *track, int datatype) {
  std::vector<std::vector<double> > param(2);
  param.at(0).resize(2); param.at(1).resize(2);
  for ( int iview = 0; iview < 2; iview++ )
    for ( int iparam = 0; iparam < 2; iparam++ )
      param.at(iview).at(iparam) = -9999; // If fitting cannot be done correctly, ignore the track

  TF1 *linear[2];
  TGraphAsymmErrors *hit_graph[2];
  for ( int iview = 0; iview < 2; iview++ ) {
    linear[iview] = new TF1(Form("linear %d", iview), "[0] * x + [1]", -2000., 2000.);
    linear[iview]->SetParameter(0, 0.);
    linear[iview]->SetParameter(1, 0.);
  }

  std::vector<const B2HitSummary* > hits;

  auto it_cluster = track->BeginCluster();
  while ( const auto *cluster = it_cluster.Next() ) {
    auto it_hit = cluster->BeginHit();
    while ( const auto *hit = it_hit.Next() ) {
      if ( hit->GetDetectorId() != B2Detector::kBabyMind ) continue;
      if ( hit->GetView() == B2View::kSideView &&
	   hit->GetPlane() > 2) continue; // We only use upstream three planes for sideview
      hits.push_back(hit);
    } // hit
  } // cluster

  std::vector<std::vector<std::vector<std::vector<double> > > > position_and_errors = GenerateMergedPositionAndErrors(hits, datatype);
  // position_and_errors.at(view).at(pos/higherr/lowerr).at(xy/z).at(plane)
  // sideview vectors
  std::vector<Double_t> position_side_y = position_and_errors.at(0).at(0).at(0);
  std::vector<Double_t> position_side_z = position_and_errors.at(0).at(0).at(1);
  std::vector<Double_t> higherr_side_y = position_and_errors.at(0).at(1).at(0);
  std::vector<Double_t> higherr_side_z = position_and_errors.at(0).at(1).at(1);
  std::vector<Double_t> lowerr_side_y = position_and_errors.at(0).at(2).at(0);
  std::vector<Double_t> lowerr_side_z = position_and_errors.at(0).at(2).at(1);
  // topview vectors
  std::vector<Double_t> position_top_x = position_and_errors.at(1).at(0).at(0);
  std::vector<Double_t> position_top_z = position_and_errors.at(1).at(0).at(1);
  std::vector<Double_t> higherr_top_x = position_and_errors.at(1).at(1).at(0);
  std::vector<Double_t> higherr_top_z = position_and_errors.at(1).at(1).at(1);
  std::vector<Double_t> lowerr_top_x = position_and_errors.at(1).at(2).at(0);
  std::vector<Double_t> lowerr_top_z = position_and_errors.at(1).at(2).at(1);


  for ( int iview = 0; iview < 2; iview++ ) {
    if ( iview == B2View::kSideView ) {
      hit_graph[iview] = new TGraphAsymmErrors(position_side_z.size(),
					       &position_side_z[0],
					       &position_side_y[0],
					       &lowerr_side_z[0],
					       &higherr_side_z[0],
					       &lowerr_side_y[0],
					       &higherr_side_y[0]);
    } else if ( iview == B2View::kTopView ) {
      hit_graph[iview] = new TGraphAsymmErrors(position_top_z.size(),
					       &position_top_z[0],
					       &position_top_x[0],
					       &lowerr_top_z[0],
					       &higherr_top_z[0],
					       &lowerr_top_x[0],
					       &higherr_top_x[0]);
    }

    hit_graph[iview]->Fit(linear[iview],"Q","");
    param.at(iview).at(0) = linear[iview]->GetParameter(0);
    param.at(iview).at(1) = linear[iview]->GetParameter(1);
  }
  
  delete linear[0];
  delete linear[1];

  return param;

}

std::vector<double> GetBabyMindInitialDirectionAndPosition(const B2TrackSummary *track, int datatype) {

  std::vector<double> initial_direction_and_position(4);
  // 0 : tan Y, 1 : tan X, 2 : pos Y, 3 : pos X

  std::vector<std::vector<double> > param = FitBabyMind(track, datatype);
  for (int iview = 0; iview < 2; iview++) {
    initial_direction_and_position.at(iview) = param.at(iview).at(0);
    initial_direction_and_position.at(iview + 2)
      = param.at(iview).at(1) + param.at(iview).at(0) * BM_SECOND_LAYER_POS;
  }

  return initial_direction_and_position;

}

std::vector<double> CalculateExpectedPosition(NTBMSummary *ntbm, int itrack, double z_shift) {

  // Pre reconstructed position/direction in BM coordinate
  std::vector<double> baby_mind_pre_direction = ntbm->GetBabyMindTangent(itrack);
  std::vector<double> baby_mind_pre_position = ntbm->GetBabyMindPosition(itrack);

  std::vector<double> position(2);
  std::vector<double> distance(2);

  std::vector<double> baby_mind_position = {BABYMIND_POS_Y, BABYMIND_POS_X};
  std::vector<double> ninja_overall_position = {NINJA_POS_Y, NINJA_POS_X};
  std::vector<double> ninja_tracker_position = {NINJA_TRACKER_POS_Y, NINJA_TRACKER_POS_X};

  for ( int iview = 0; iview < 2; iview++ ) {
    // extrapolate Baby MIND track to the tracker position
    distance.at(iview) = BABYMIND_POS_Z + BM_SECOND_LAYER_POS
      - NINJA_POS_Z - NINJA_TRACKER_POS_Z - (2 * iview - 1) * 10. + z_shift;
    position.at(iview) = baby_mind_pre_position.at(iview) - baby_mind_pre_direction.at(iview) * distance.at(iview);
    // convert coordinate from BM to the tracker
    position.at(iview) = position.at(iview) + baby_mind_position.at(iview)
      - ninja_overall_position.at(iview) - ninja_tracker_position.at(iview);
  }

  return position;

}

bool NinjaHitExpected(NTBMSummary *ntbm, int itrack, double z_shift) {

  std::vector<double> hit_expected_position = CalculateExpectedPosition(ntbm, itrack, z_shift);
  // Extrapolated position inside tracker area TODO
  if ( ( hit_expected_position.at(B2View::kTopView) < -600. - 100. ||
         hit_expected_position.at(B2View::kTopView) > 448. + 100. ) ||
       ( hit_expected_position.at(B2View::kSideView) < -448. - 100. ||
	 hit_expected_position.at(B2View::kSideView) > 600.  + 100. ) )
    return false;

  // Downstream WAGASCI interaction
  if ( ntbm->GetNinjaTrackType(itrack) == -1 )
    return false;

  return true;

}

bool MatchBabyMindTrack(NTBMSummary* ntbm, int itrack, int &bunch_diff, double z_shift) {

  std::vector<double> hit_expected_position = CalculateExpectedPosition(ntbm, itrack, z_shift);

  std::vector<int> matched_cluster_tmp(2);
  bool is_match = false;

  // set bunch difference loop region
  int start_bunch_difference = 0;
  int end_bunch_difference = 7;
  if ( bunch_diff != -1 ) {
    start_bunch_difference = bunch_diff;
    end_bunch_difference = bunch_diff + 1;
  }

  // Too many loops can be reduced TODO
  for ( int ibunch_difference = start_bunch_difference; ibunch_difference < end_bunch_difference; ibunch_difference++ ) {

    std::vector<double> position_difference_tmp(2);
    position_difference_tmp.at(B2View::kSideView) = TEMPORAL_ALLOWANCE[B2View::kSideView];
    position_difference_tmp.at(B2View::kTopView) = TEMPORAL_ALLOWANCE[B2View::kTopView];

    for ( int icluster = 0; icluster < ntbm->GetNumberOfNinjaClusters(); icluster++ ) {
      if ( ntbm->GetBunchDifference(icluster) != ibunch_difference ) continue;
      std::vector<int> number_of_hits = ntbm->GetNumberOfHits(icluster);
      // Get view information from 1d NINJA cluster
      int view = -1;
      if ( number_of_hits.at(B2View::kTopView) > 0 ) {
	if ( number_of_hits.at(B2View::kSideView) > 0 ) continue;
	else view = B2View::kTopView;
      } else if ( number_of_hits.at(B2View::kSideView > 0) ) {
	view = B2View::kSideView;
      } else {
	BOOST_LOG_TRIVIAL(error) << "Cluster with no hit : "
				 << *ntbm;
	std::exit(1);
      }

      // pre reconstructed NINJA position
      std::vector<double> ninja_position = ntbm->GetNinjaPosition(icluster);
      
      if ( std::fabs(hit_expected_position.at(view) - ninja_position.at(view))
	   < std::fabs(position_difference_tmp.at(view)) ) {
	matched_cluster_tmp.at(view) = icluster;
	position_difference_tmp.at(view) = hit_expected_position.at(view) - ninja_position.at(view);
	BOOST_LOG_TRIVIAL(debug) << "matched 1d cluster update : view : " << view << " cluster : " << icluster;
      }
    } // icluster

    if ( std::fabs(position_difference_tmp.at(B2View::kSideView)) < TEMPORAL_ALLOWANCE[B2View::kSideView] &&
	 std::fabs(position_difference_tmp.at(B2View::kTopView))  < TEMPORAL_ALLOWANCE[B2View::kTopView] ) {
      // If initial bunch difference = -1, bunch_diff is first set for this spill
      // else ibunch_diff is sweeped only ibunch_diff == bunch_diff and nothing changes
      bunch_diff = ibunch_difference;
      is_match = true;
      break;
    }
    
  } // ibunch_difference

  if ( !is_match ) return false;
  // Create a new 2d cluster and add it
  std::vector<int> number_of_hits(2);
  std::vector<std::vector<int> > plane(2);
  std::vector<std::vector<int> > slot(2);
  std::vector<std::vector<double> > pe(2);
  std::vector<double> ninja_position(2);
  std::vector<double> ninja_tangent(2);

  ntbm->SetNumberOfNinjaClusters(ntbm->GetNumberOfNinjaClusters() + 1);
  int new_cluster_id = ntbm->GetNumberOfNinjaClusters() - 1;
  ntbm->SetBabyMindTrackId(new_cluster_id, itrack);
  for ( int view = 0; view < 2; view++ ) {
    number_of_hits.at(view) = ntbm->GetNumberOfHits(matched_cluster_tmp.at(view), view);
    ninja_position.at(view) = ntbm->GetNinjaPosition(matched_cluster_tmp.at(view)).at(view);
    ninja_tangent.at(view) = ntbm->GetNinjaTangent(matched_cluster_tmp.at(view)).at(view);
    for ( int hit = 0; hit < number_of_hits.at(view); hit++ ) {
      plane.at(view).push_back(ntbm->GetPlane(matched_cluster_tmp.at(view), view, hit));
      slot.at(view).push_back(ntbm->GetSlot(matched_cluster_tmp.at(view), view, hit));
      pe.at(view).push_back(ntbm->GetPe(matched_cluster_tmp.at(view), view, hit));
    } // hit
  } // view

  ntbm->SetBunchDifference(new_cluster_id, bunch_diff);
  ntbm->SetNumberOfHits(new_cluster_id, number_of_hits);
  ntbm->SetNinjaPosition(new_cluster_id, ninja_position);
  ntbm->SetNinjaTangent(new_cluster_id, ninja_tangent);
  ntbm->SetPlane(new_cluster_id, plane);
  ntbm->SetSlot(new_cluster_id, slot);
  ntbm->SetPe(new_cluster_id, pe);

  return true;

}

bool IsInRange(double pos, double min, double max) {
  if ( min > max ) 
    throw std::invalid_argument(" Minimum should be smaller than maximum");
  if ( min <= pos && pos <= max ) return true;
  else return false;
}

bool IsMakeHit(double min, double max, int view, int plane, int slot) {

  TVector3 position;
  double position_xy = 0.;
  B2Dimension::GetPosNinjaTracker((B2View)view, plane, slot, position);
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

  // Edge channels
  if ( view == B2View::kSideView ) {
    if ( slot == 0 ) {
      return IsInRange(max, position_xy - NINJA_SCI_WIDTH / 2., position_xy + NINJA_SCI_WIDTH / 2.) ||
	( IsInRange(max, position_xy + NINJA_SCI_WIDTH / 2.,
		    position_xy + NINJA_SCI_WIDTH / 2. + NINJA_TRACKER_GAP) &&
	  IsInRange(min, position_xy - NINJA_SCI_WIDTH / 2., position_xy + NINJA_SCI_WIDTH / 2.) );
    } else if ( slot == NINJA_TRACKER_NUM_CHANNELS_ONE_PLANE - 1 ) {
      return IsInRange(min, position_xy - NINJA_SCI_WIDTH / 2., position_xy + NINJA_SCI_WIDTH / 2.) ||
	( IsInRange(min, position_xy - NINJA_SCI_WIDTH / 2. - NINJA_TRACKER_GAP, 
		    position_xy - NINJA_SCI_WIDTH / 2.) &&
	  IsInRange(max, position_xy - NINJA_SCI_WIDTH / 2., position_xy + NINJA_SCI_WIDTH / 2.) );
    }
  } else if ( view == B2View::kTopView ) {
    if ( slot == 0 ) {
      return IsInRange(min, position_xy - NINJA_SCI_WIDTH / 2., position_xy + NINJA_SCI_WIDTH / 2.) ||
	( IsInRange(min, position_xy - NINJA_SCI_WIDTH / 2. - NINJA_TRACKER_GAP, 
		    position_xy - NINJA_SCI_WIDTH / 2.) &&
	  IsInRange(max, position_xy - NINJA_SCI_WIDTH / 2., position_xy + NINJA_SCI_WIDTH / 2.) );
    } else if ( slot == NINJA_TRACKER_NUM_CHANNELS_ONE_PLANE - 1 ) {
      return IsInRange(max, position_xy - NINJA_SCI_WIDTH / 2., position_xy + NINJA_SCI_WIDTH / 2.) ||
	( IsInRange(max, position_xy + NINJA_SCI_WIDTH / 2.,
		    position_xy + NINJA_SCI_WIDTH / 2. + NINJA_TRACKER_GAP) &&
	  IsInRange(min, position_xy - NINJA_SCI_WIDTH / 2., position_xy + NINJA_SCI_WIDTH / 2.) );
    }
  }

  // The other channels
  return IsInRange(min, position_xy - NINJA_SCI_WIDTH / 2. - NINJA_TRACKER_GAP,
		   position_xy + NINJA_SCI_WIDTH / 2.)
    && IsInRange(max, position_xy - NINJA_SCI_WIDTH / 2.,
		 position_xy + NINJA_SCI_WIDTH / 2. + NINJA_TRACKER_GAP);
}

bool IsInGap(double min, double max, int view, int plane, int slot) {

  int slot_tmp = slot;
  if (slot == -1)
    slot_tmp = 0;

  TVector3 position;
  double position_xy = 0.;  
  B2Dimension::GetPosNinjaTracker((B2View)view, plane, slot_tmp, position);
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

  // Dead channels
  if ( view == B2View::kSideView && plane == 2 ) {
    if ( slot == 29 ) return position_xy + NINJA_SCI_WIDTH / 2. <= min;
    else if ( slot == 30 ) return false;
  }
  if ( view == B2View::kTopView && plane == 1 ) {
    if ( slot == -1 || slot == 0 ) return false;
    else if ( slot == 1 ) return position_xy - NINJA_SCI_WIDTH / 2. - NINJA_TRACKER_GAP <= min;
    else if ( slot == 6 ) return position_xy - NINJA_SCI_WIDTH * 3 / 2. - 2 * NINJA_TRACKER_GAP <= min &&
			    max <= position_xy - NINJA_SCI_WIDTH / 2.;
    else if ( slot == 7 ) return false;
  }
  if ( view == B2View::kTopView && plane == 2 ) {
    if ( slot == 19 ) return position_xy - NINJA_SCI_WIDTH * 3 / 2. - 2 * NINJA_TRACKER_GAP <= min &&
			max <= position_xy - NINJA_SCI_WIDTH / 2.;
    else if ( slot == 20 ) return false;
  }

  // The other channels
  if ( view == B2View::kSideView ) {
    if ( slot == -1 )
      return max <= position_xy - NINJA_SCI_WIDTH / 2.;
    else if ( slot == NINJA_TRACKER_NUM_CHANNELS_ONE_PLANE - 1 )
      return position_xy + NINJA_SCI_WIDTH / 2. <= min;
    else
      return position_xy + NINJA_SCI_WIDTH / 2. <= min &&
	max <= position_xy + NINJA_SCI_WIDTH / 2. + NINJA_TRACKER_GAP;
  } else if ( view == B2View::kTopView ) {
    if ( slot == -1 )
      return position_xy + NINJA_SCI_WIDTH / 2. <= min;
    else if ( slot == NINJA_TRACKER_NUM_CHANNELS_ONE_PLANE - 1 )
      return max <= position_xy - NINJA_SCI_WIDTH / 2.;
    else
      return position_xy - NINJA_SCI_WIDTH / 2. - NINJA_TRACKER_GAP <= min &&
	max <= position_xy - NINJA_SCI_WIDTH / 2.;
  }
}

double GetTrackAreaMin(double pos, double tangent, int iplane, int jplane, int vertex) {
  if ( tangent > 0 ) return pos + tangent * (NINJA_TRACKER_OFFSET_Z[jplane] - NINJA_TRACKER_OFFSET_Z[iplane]
					     + NINJA_SCI_THICK * (0 - vertex % 2));
  else return pos + tangent * (NINJA_TRACKER_OFFSET_Z[jplane] - NINJA_TRACKER_OFFSET_Z[iplane]
			       + NINJA_SCI_THICK * (1 - vertex % 2));
}

double GetTrackAreaMax(double pos, double tangent, int iplane, int jplane, int vertex) {
  if ( tangent > 0 ) return pos + tangent * (NINJA_TRACKER_OFFSET_Z[jplane] - NINJA_TRACKER_OFFSET_Z[iplane]
					     + NINJA_SCI_THICK * (1 - vertex % 2));
  else return pos + tangent * (NINJA_TRACKER_OFFSET_Z[jplane] - NINJA_TRACKER_OFFSET_Z[iplane]
			       + NINJA_SCI_THICK * (0 - vertex % 2));
}

bool IsGoodTrack(bool *condition) {
  bool ret = true;
  for ( int iplane = 0; iplane < NINJA_TRACKER_NUM_PLANES; iplane++ )
    ret = ret && condition[iplane];
  return ret;
}

std::vector<std::vector<int> > GetNinjaTrackerNumberOfPlaneHits(NTBMSummary* ntbm, int cluster) {
  std::vector<std::vector<int> > number_of_plane_hits(2);
  for ( int iview = 0; iview < 2; iview++ ) {
    number_of_plane_hits.at(iview).resize(4);
    for ( int iplane = 0; iplane < 4; iplane++ ) {
      number_of_plane_hits.at(iview).at(iplane) = 0;
    }
    for ( int ihit = 0; ihit < ntbm->GetNumberOfHits(cluster, iview); ihit++ ) {
      number_of_plane_hits.at(iview).at(ntbm->GetPlane(cluster, iview, ihit))++;
    }
  }
  
  return number_of_plane_hits;

}

void ReconstructNinjaTangent(NTBMSummary* ntbm) {

  std::vector<double> baby_mind_position = {BABYMIND_POS_Y, BABYMIND_POS_X};
  std::vector<double> ninja_overall_position = {NINJA_POS_Y, NINJA_POS_X};
  std::vector<double> ninja_tracker_position = {NINJA_TRACKER_POS_Y, NINJA_TRACKER_POS_X};

  for (int icluster = 0; icluster < ntbm->GetNumberOfNinjaClusters(); icluster++) {
    
    std::vector<double> tangent(2);
    int trackid = ntbm->GetBabyMindTrackId(icluster);
    if (trackid < 0) continue;
    std::vector<double> baby_mind_initial_position = ntbm->GetBabyMindPosition(trackid);
    std::vector<double> ninja_position_tmp = ntbm->GetNinjaPosition(icluster);
    for (int iview = 0; iview < 2; iview++) {
      baby_mind_initial_position.at(iview) = baby_mind_initial_position.at(iview)
	+ baby_mind_position.at(iview)
	- ninja_overall_position.at(iview)
	- ninja_tracker_position.at(iview);
    }

    for (int iview = 0; iview < 2; iview++) {
      tangent.at(iview) = (baby_mind_initial_position.at(iview) - ninja_position_tmp.at(iview))
	/ (BABYMIND_POS_Z + BM_SECOND_LAYER_POS - NINJA_POS_Z - NINJA_TRACKER_POS_Z - (2*iview - 1) * 10.);
    }
    ntbm->SetNinjaTangent(icluster, tangent);
  }

}

void ReconstructNinjaPosition(NTBMSummary* ntbm) {

  for ( int icluster = 0; icluster < ntbm->GetNumberOfNinjaClusters(); icluster++ ) {
 
    std::vector<double> tangent = ntbm->GetNinjaTangent(icluster);

    std::vector<double> position(2);
    // vector where good position candidates are filled
    std::vector<std::vector<double> > position_list(2);

    // Number of hits in NINJA tracker one plane
    // number_of_plane_hits.at(view).at(plane)
    std::vector<std::vector<int> > number_of_plane_hits = GetNinjaTrackerNumberOfPlaneHits(ntbm, icluster);

    for ( int iview = 0; iview < 2; iview++ ) {

      // skip invalid view for 1d cluster
      if ( ntbm->GetNumberOfHits(icluster, iview) == 0 ) continue;

      // Draw lines from each vertex of each scintillator and check every line
      for ( int iplane = 0; iplane < NINJA_TRACKER_NUM_PLANES; iplane++ ) {
	for ( int islot = 0; islot < NINJA_TRACKER_NUM_CHANNELS_ONE_PLANE; islot++ ) {
	  for ( int ivertex = 0; ivertex < 4; ivertex++ ) { // Number of vertices in one scintillator bar

	    BOOST_LOG_TRIVIAL(trace) << "Cluster : " << icluster << " "
				     << "View : "    << iview << " "
				     << "Plane : "   << iplane << " "
				     << "Slot : "    << islot << " "
				     << "Vertex : "  << ivertex;

	    TVector3 start_of_track;
	    // Position in tracker box coordinate
	    B2Dimension::GetPosNinjaTracker((B2View)iview, iplane, islot, start_of_track);
	    double start_of_track_xy;
	    switch (iview) {
	    case B2View::kTopView :
	      start_of_track_xy = start_of_track.X()
		+ NINJA_SCI_WIDTH / 2. * ( -1 + 2 * (ivertex/2) );
	      break;
	    case B2View::kSideView :
	      start_of_track_xy = start_of_track.Y()
		+ NINJA_SCI_WIDTH / 2. * ( -1 + 2 * (ivertex/2) );
	      break;
	    }

	    // Check the line can make a hit pattern
	    bool plane_condition[NINJA_TRACKER_NUM_PLANES] = {false};

	    for ( int jplane = 0; jplane < NINJA_TRACKER_NUM_PLANES; jplane++ ) {

	      double track_area_min = GetTrackAreaMin(start_of_track_xy, tangent.at(iview),
						      iplane, jplane, ivertex);
	      double track_area_max = GetTrackAreaMax(start_of_track_xy, tangent.at(iview),
						      iplane, jplane, ivertex);

	      // When there is a hit in the plane, check the track penetrates the slot
	      if ( number_of_plane_hits.at(iview).at(jplane) > 0 ) {
		for ( int ihit = 0; ihit < ntbm->GetNumberOfHits(icluster, iview); ihit++ ) {
		  if ( ntbm->GetPlane(icluster, iview, ihit) == jplane ) {
		    plane_condition[jplane] = IsMakeHit(track_area_min, track_area_max,
							iview, jplane, ntbm->GetSlot(icluster, iview, ihit));
		  }
		} // ihit
	      } else { // When there are no hits in the plane, check the track penetrates some gap
		for ( int jslot = -1; jslot < NINJA_TRACKER_NUM_CHANNELS_ONE_PLANE; jslot++ ) {
		  plane_condition[jplane] = plane_condition[jplane] ||
		    IsInGap(track_area_min, track_area_max, iview, jplane, jslot);
		} // jslot
	      } // fi
	    } // jplane

	    if ( IsGoodTrack(plane_condition) ) {
	      BOOST_LOG_TRIVIAL(debug) << "Is Good Track";
	      position_list.at(iview).push_back(start_of_track_xy + tangent.at(iview)
						* (- NINJA_SCI_THICK * (ivertex % 2) - NINJA_TRACKER_OFFSET_Z[iplane] + NINJA_TRACKER_OFFSET_Z[2]));
	    }
	  } // ivertex
	} // islot
      } // iplane
    } // iview

    // Use lines with good plane condition and reconstruct position
    for ( int iview = 0; iview < 2; iview++ ) {
      if (position_list.at(iview).size() > 0) {
	std::sort(position_list.at(iview).begin(), position_list.at(iview).end());
	position.at(iview) = (position_list.at(iview).front() + position_list.at(iview).back()) / 2.;
      }
      else { // average of all scintillator bar position in the cluster
	position.at(iview) = 0.;
	for ( int ihit = 0; ihit < ntbm->GetNumberOfHits(icluster, iview); ihit++ ) {
	  TVector3 scintillator_position;
	  B2Dimension::GetPosNinjaTracker((B2View)iview, ntbm->GetPlane(icluster, iview, ihit),
					  ntbm->GetSlot(icluster, iview, ihit),
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
	position.at(iview) /= ntbm->GetNumberOfHits(icluster, iview);
      } // fi
    } // iview

    ntbm->SetNinjaPosition(icluster, position);
  } // icluster
  
}

void SetTruePositionAngle(const B2SpillSummary& spill_summary, NTBMSummary* ntbm_summary) {

  auto it_emulsion = spill_summary.BeginEmulsion();
  while (const auto *emulsion = it_emulsion.Next()) {
    // Get position of TSS downstream film position
    if (emulsion->GetFilmType() == B2EmulsionType::kShifter && emulsion->GetPlate() == 15) {
      int particle_id = emulsion->GetParentTrack().GetParticlePdg();
      TVector3 true_position = emulsion->GetAbsolutePosition().GetValue();
      if (!B2Pdg::IsMuonPlusOrMinus(particle_id)) continue;

      // Get most muon-like cluster and set true info
      for (int icluster = 0; icluster < ntbm_summary->GetNumberOfNinjaClusters(); icluster++) {
	std::vector<double> ninja_position = ntbm_summary->GetNinjaPosition(icluster);

      }

    }
  }
}

// Transfer B2Summary information

void TransferBeamInfo(const B2SpillSummary &spill_summary, NTBMSummary *ntbm_summary) {
  auto beam_summary = spill_summary.GetBeamSummary();
  ntbm_summary->SetSpillPot(beam_summary.GetSpillPot());
  for (std::size_t bunch = 0; bunch < 8; bunch++)
    ntbm_summary->SetBunchPot(bunch, beam_summary.GetBunchPot(bunch));
  ntbm_summary->SetBsdSpillNumber(beam_summary.GetBsdSpillNumber());
  ntbm_summary->SetTimestamp(beam_summary.GetTimestamp());
  ntbm_summary->SetBsdGoodSpillFlag(beam_summary.GetBsdGoodSpillFlag());
  ntbm_summary->SetWagasciGoodSpillFlag(beam_summary.GetWagasciGoodSpillFlag());
  for (int i = 0; i < 8; i++) {
    ntbm_summary->SetDetectorFlags(i, beam_summary.GetDetectorFlags().at(i));
  }
}

void TransferBabyMindTrackInfo(const B2SpillSummary &spill_summary, NTBMSummary *ntbm_summary, int datatype) {
  
  int itrack = 0;

  auto it_recon_vertex = spill_summary.BeginReconVertex();
  while ( auto *vertex = it_recon_vertex.Next() ) {
    auto it_outgoing_track = vertex->BeginTrack();
    while ( auto *track = it_outgoing_track.Next() ) {
      if ( track->GetTrackType() == B2TrackType::kPrimaryTrack ) {
	if ( track->GetPrimaryTrackType() == B2PrimaryTrackType::kBabyMind3DTrack ) {
	  ntbm_summary->SetNinjaTrackType(itrack, 0); // ECC interaction candidate (or sand muon)
	} else if ( track->GetPrimaryTrackType() == B2PrimaryTrackType::kMatchingTrack &&
		    track->HasDetector(B2Detector::kBabyMind) ) {
	  if ( track->HasDetector(B2Detector::kProtonModule) ||
	       track->HasDetector(B2Detector::kWagasciUpstream) ) {
	    if ( vertex->GetInsideFiducialVolume() ) {
	      ntbm_summary->SetNinjaTrackType(itrack, 2); // upstream modules interaction
	    } else {
	      ntbm_summary->SetNinjaTrackType(itrack, 1); // sand muon from wall
	    }
	  } else if ( track->HasDetector(B2Detector::kWagasciDownstream) ) {
	    if ( vertex->GetInsideFiducialVolume() ){
	      ntbm_summary->SetNinjaTrackType(itrack, -1); // downstream mdoule interaction
	    } else {
	      ntbm_summary->SetNinjaTrackType(itrack, 0); // ECC interaction candidate
	    }
	  } else continue;
	} else continue;
      } else { // not primary track
	continue;
      }
      
      ntbm_summary->SetBabyMindMaximumPlane(itrack, track->GetDownstreamHit().GetPlane());
      ntbm_summary->SetTrackLengthTotal(itrack, track->GetTrackLengthTotal());
      double nll_plus = track->GetNegativeLogLikelihoodPlus();
      double nll_minus = track->GetNegativeLogLikelihoodMinus();
      if ( nll_minus - nll_plus <= 4 )
	ntbm_summary->SetCharge(itrack, 1);
      else 
	ntbm_summary->SetCharge(itrack, -1);
      ntbm_summary->SetBunch(itrack, track->GetBunch());
      
      if ( track->GetIsStopping() )
	ntbm_summary->SetMomentumType(itrack, 0); // Baby MIND range method
      else 
	ntbm_summary->SetMomentumType(itrack, 1); // should be curvature type but not yet implemented
      ntbm_summary->SetMomentum(itrack, track->GetFinalAbsoluteMomentum().GetValue());
      ntbm_summary->SetMomentumError(itrack, track->GetFinalAbsoluteMomentum().GetError());
      std::vector<Double_t> direction_and_position = GetBabyMindInitialDirectionAndPosition(track, datatype);
      for (int view = 0; view < 2; view++) {
	ntbm_summary->SetBabyMindPosition(itrack, view, direction_and_position.at(view+2));
	ntbm_summary->SetBabyMindTangent(itrack, view, direction_and_position.at(view));
      }
      
      itrack++;
      
    } // while track
  } // while vertex

}

void TransferMCInfo(const B2SpillSummary &spill_summary, NTBMSummary *ntbm_summary) {
  auto it_event = spill_summary.BeginTrueEvent();
  const auto *event = it_event.Next();
  ntbm_summary->SetNormalization(event->GetNormalization());
  ntbm_summary->SetTotalCrossSection(event->GetTotalCrossSection());
}

// main

int main(int argc, char *argv[]) {

  gErrorIgnoreLevel = kError;

  logging::core::get()->set_filter
    (
     logging::trivial::severity >= logging::trivial::info
     //logging::trivial::severity >= logging::trivial::debug
     //logging::trivial::severity >= logging::trivial::trace
     );

  BOOST_LOG_TRIVIAL(info) << "==========NINJA Track Matching Start==========";

  if ( argc != 5 ) {
    BOOST_LOG_TRIVIAL(error) << "Usage : " << argv[0]
			     << " <input B2 file path> <output NTBM file path> <z shift> <MC(0)/data(1)>";
    std::exit(1);
  }

  try {
    B2Reader reader(argv[1]);

    TFile *ntbm_file = new TFile(argv[2], "recreate");
    TTree *ntbm_tree = new TTree("tree", "NINJA BabyMIND Original Summary");
    NTBMSummary* my_ntbm = nullptr;
    ntbm_tree->Branch("NTBMSummary", &my_ntbm);

    double z_shift = std::stof(argv[3]);
    int datatype = std::stoi(argv[4]);

    int nspill = 0;

    while ( reader.ReadNextSpill() > 0 ) {

      auto &input_spill_summary = reader.GetSpillSummary();
      int timestamp = input_spill_summary.GetBeamSummary().GetTimestamp();
      BOOST_LOG_TRIVIAL(debug) << "entry : " << reader.GetEntryNumber();
      BOOST_LOG_TRIVIAL(debug) << "timestamp : " << timestamp;
 
      TransferBeamInfo(input_spill_summary, my_ntbm);
      TransferMCInfo(input_spill_summary, my_ntbm);

      // Collect all NINJA hits
      auto it_hit = input_spill_summary.BeginHit();
      std::vector<const B2HitSummary* > ninja_hits;
      while ( const auto *ninja_hit = it_hit.Next() ) {
	if ( ninja_hit->GetDetectorId() == B2Detector::kNinja ) {
	  if ( B2Dimension::CheckDeadChannel(B2Detector::kNinja, ninja_hit->GetView(),
					     ninja_hit->GetSingleReadout(), ninja_hit->GetPlane(),
					     ninja_hit->GetSlot().GetValue(ninja_hit->GetSingleReadout())) )
	    continue;

	  if ( ninja_hit->GetView() == B2View::kTopView &&
	       ninja_hit->GetPlane() == 0 &&
	       ninja_hit->GetSlot().GetValue(ninja_hit->GetSingleReadout()) == 25 &&
	       ninja_hit->GetHighGainPeu(ninja_hit->GetSingleReadout()) < 3.5 )
	    continue; // noisy channel
	  // if ( ninja_hit->GetHighGainPeu(ninja_hit->GetSingleReadout()) < 3.5 )
	  if ( ninja_hit->GetHighGainPeu(ninja_hit->GetSingleReadout()) < 2.5 )
	    continue;

	  ninja_hits.push_back(ninja_hit);
	}
      }

      // Create X/Y NINJA clusters
      if ( ninja_hits.size() > 0 ) {
	CreateNinjaCluster(ninja_hits, my_ntbm);
	// Position reconstruction w/o angle info
	ReconstructNinjaPosition(my_ntbm);
      }

      // Collect all BM 3d tracks
      int number_of_tracks = 0;
      
      auto it_recon_vertex = input_spill_summary.BeginReconVertex();
      while ( auto *vertex = it_recon_vertex.Next() ) {
	auto it_outgoing_track = vertex->BeginTrack();
	while ( auto *track = it_outgoing_track.Next() ) {
	  if ( track->GetTrackType() == B2TrackType::kPrimaryTrack ) {
	    // not start from the other WAGASCI modules
	    if ( track->GetPrimaryTrackType() == B2PrimaryTrackType::kBabyMind3DTrack ) {
	      number_of_tracks++;
	    // start from the other modules and have hits in Baby MIND
	    } else if ( track->GetPrimaryTrackType() == B2PrimaryTrackType::kMatchingTrack && 
			track->HasDetector(B2Detector::kBabyMind) ) {
	      if ( track->HasDetector(B2Detector::kProtonModule) ||
		   track->HasDetector(B2Detector::kWagasciUpstream) ||
		   track->HasDetector(B2Detector::kWagasciDownstream) ) {
		number_of_tracks++;
	      }
	    }
	  }
	} // track
      } // vertex
            
      my_ntbm->SetNumberOfTracks(number_of_tracks);

      // Extrapolate BabyMIND tracks to the NINJA position
      // and get the best cluster to match each BabyMIND track
      if ( number_of_tracks > 0 ) {

	TransferBabyMindTrackInfo(input_spill_summary, my_ntbm, datatype);

	int start_bunch = 0; // bunch id (1-8) corresponds to NINJA tracker ADC triggered timing
	int bunch_difference = -1; // difference between the bunch in interest and the start_bunch
	
	for ( int ibmtrack = 0; ibmtrack < my_ntbm->GetNumberOfTracks(); ibmtrack++ ) {
	  if ( start_bunch > 0 ) // when the start bunch is already determined
	    bunch_difference = my_ntbm->GetBunch(ibmtrack) - start_bunch;
	  if ( NinjaHitExpected(my_ntbm, ibmtrack, z_shift) && // Extrapolated position w/i tracker area
	       bunch_difference < 7 ) { // Multi hit TDC range
	    if ( MatchBabyMindTrack(my_ntbm, ibmtrack, bunch_difference, z_shift) ) {
	      // If this is the first matching, set start_bunch
	      if ( start_bunch == 0 ) {
		start_bunch = my_ntbm->GetBunch(ibmtrack) - bunch_difference;
		BOOST_LOG_TRIVIAL(debug) << "This is the first matching: "
					 << "start bunch = " << start_bunch;
	      }
	    }
	  }
	} // ibmtrack
	
	// Update NINJA hit summary information
	ReconstructNinjaTangent(my_ntbm); // reconstruct tangent
	ReconstructNinjaPosition(my_ntbm); // use reconstructed tangent info
      }
      
      // Create output tree
      BOOST_LOG_TRIVIAL(debug) << *my_ntbm;
      ntbm_tree->Fill();
      my_ntbm->Clear("C");
    }

    ntbm_file->cd();
    ntbm_tree->Write();
    ntbm_file->Close();
    
  } catch (const std::runtime_error &error) {
    BOOST_LOG_TRIVIAL(fatal) << "Runtime error : " << error.what();
    std::exit(1);
  } catch (const std::invalid_argument &error) {
    BOOST_LOG_TRIVIAL(fatal) << "Invalid argument error : " << error.what();
    std::exit(1);
  } catch (const std::out_of_range &error) {
    BOOST_LOG_TRIVIAL(fatal) << "Out of range error : " << error.what();
    std::exit(1);
  }
  
  BOOST_LOG_TRIVIAL(info) << "==========NINJA Track Matching Finish==========";
  std::exit(0);

}
