// system includes
#include <vector>
#include <algorithm>
#include <iostream>

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

void ReconstructNinjaPosition(NTBMSummary* ntbmsummary) {

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

      if (ninja_hits.size() > 0) 
	CreateNinjaCluster(ninja_hits, my_ntbm);

      // Extrapolate BabyMIND tracks to the NINJA position
      // and get the best cluster to match each BabyMIND track
      auto it_track = input_spill_summary.BeginReconTrack();
      while (const auto *track = it_track.Next()) {
	if(track->HasDetector(B2Detector::kBabyMind)) {

	}
      }
      // Update NINJA hit summary information?
      
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
