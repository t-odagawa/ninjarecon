#include <vector>
#include <algorithm>
#include <numeric>
#include <iostream>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#include <TFile.h>
#include <TTree.h>

#include <B2Reader.hh>
#include <B2Enum.hh>
#include <B2Dimension.hh>
#include <B2SpillSummary.hh>
#include <B2HitSummary.hh>
#include <B2EventSummary.hh>
#include <B2TrackSummary.hh>
#include <B2VertexSummary.hh>
#include <B2ClusterSummary.hh>

namespace logging = boost::log;

std::vector<Double_t > CalcTangentUpY(std::vector<const B2HitSummary*> hits) {
  std::vector<Double_t > y_position;
  std::vector<Double_t > y_err;
  std::vector<Double_t > z_position;
  std::vector<Double_t > z_err;

  for ( auto hit : hits ) {
    
  }

}

int main (int argc, char* argv[]) {

  gErrorIgnoreLevel = kError;

  logging::core::get()->set_filter
    (
     logging::trivial::severity >= logging::info
     );

  BOOST_LOG_TRIVIAL(info) << "==========Comparison of up/downstream angle Start==========";

  if ( argc != 4 ) {
    BOOST_LOG_TRIVIAL(error) << "Usage : " << argv[0]
			     << " <input B2 file path> <output file path> <MC(0)/data(1)>";
    std::exit(1);
  }

  try {

    B2Reader reader(argv[1]);
    
    TFile *ofile = new TFile(argv[2], "recreate");
    TTree *otree = new TTree("tree", "tree");
    Int_t entry;
    Double_t tangent_y_up, tangent_y_down;
    Double_t tangent_x_up, tangent_x_down;
    Double_t bm_pm_posdiff_y, bm_pm_posdiff_x;
    Int_t bm_layer;
    Double_t bm_range_mom;
    otree->Branch("entry",           &entry,           "entry/I");
    otree->Branch("tangent_y_up",    &tangent_y_up,    "tangent_y_up/D");
    otree->Branch("tangent_y_down",  &tangent_y_down,  "tangent_y_down/D");
    otree->Branch("tangent_x_up",    &tangent_x_up,    "tangent_x_up/D");
    otree->Branch("tangent_x_down",  &tangent_x_down,  "tangent_x_down/D");
    otree->Branch("bm_pm_posdiff_y", &bm_pm_posdiff_y, "bm_pm_posdiff_y/D");
    otree->Branch("bm_pm_posdiff_x", &bm_pm_posdiff_x, "bm_pm_posdiff_x/D");
    otree->Branch("bm_layer",        &bm_layer,        "bm_layer/I");
    otree->Branch("bm_range_mom",    &bm_range_mom,    "bm_range_mom/D");
    
    int datatype = std::stoi(argv[3]);

    int nspill = 0;
    
    while ( reader.ReadNextSpill() > 0 ) {
      
      auto &spill_summary = reader.GetSpillSummary();
      entry = reader.GetEntryNumber();
      BOOST_LOG_TRIVIAL(debug) << "Entry : " << reader.GetEntryNumber();

      // Collect all BM 3d tracks
      std::vector<B2TrackSummary*> bm_pm_tracks;
      auto it_recon_vertex = spill_summary.BeginReconVertex();
      while ( auto *vertex = it_recon_vertex.Next() ) {
	auto it_outgoing_track = vertex->BeginTrack();
	while ( auto *track = it_outgoing_track.Next() ) {
	  if ( track->GetTrackType() != B2TrackType::kPrimaryTrack ) continue;
	  if ( track->GetPrimaryTrackType() == B2PrimaryTrackType::kMatchingTrack &&
	       track->HasDetector(B2Detector::kBabyMind) &&
	       track->HasDetector(B2Detector::kProtonModule) ) {
	    bm_pm_tracks.push_back(track);
	  } // fi
	} // track
      } // vertex

      if ( bm_pm_tracks.empty() ) continue;

      for ( auto *track : bm_pm_tracks ) {
	auto it_cluster = track.BeginCluster();
	std::vector<const B2HitSummary* > bm_hits_side;
	std::vector<const B2HitSummary* > bm_hits_top;
	std::vector<const B2HitSummary* > pm_hits_side;
	std::vector<const B2HitSummary* > pm_hits_top;
	while ( auto *cluster= it_cluster.Next() ) {
	  auto it_hit = cluster.BeginHit();
	  while ( const auto *hit = it_hit.Next() ) {
	    if ( hit->GetDetectorId() == B2Detector::kBabyMind ) {
	      if ( hit->GetView() == B2View::kSideView )
		bm_hits_side.push_back(hit);
	      else if ( hit->GetView() == B2View::kTopView )
		bm_hits_top.push_back(hit);
	    }
	    else if (hit->GetDetectorId() == B2Detector::kProtonModule ) {
	      if ( hit->GetView() == B2View::kSideView )
		pm_hits_side.push_back(hit);
	      else if ( hit->GetView() == B2View::kTopView )
		pm_hits_top.push_back(hit);
	    }
	  } // hit
	} // cluster
	
	std::vector<Double_t> up_param_y = CalcTangentUpY(pm_hits_side);
	std::vector<Double_t> up_param_x = CalcTangentUpX(pm_hits_top);
	std::vector<Double_t> down_param_y = CalcTangentDownY(bm_hits_side);
	std::vector<Double_t> down_param_x = CalcTangentDownX(bm_hits_top);

	tangent_up_y = up_param_y.at(0);
	tangent_up_x = up_param_x.at(0);
	tangent_down_y = down_param_y.at(0);
	tangent_down_x = down_param_x.at(0);
	bm_pm_posdiff_y = down_param_y.at(0) * (0.) - up_param_y.at(1);
	bm_pm_posdiff_x = down_param_x.at(0) * (0.) - up_param_x.at(1);
	
	bm_layer = track->GetDownStreamHit().GetPlate();
	bm_range_mom = track->GetReconMomByRange();
	
      } // bm pm tracks

    }

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
