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
  int number_of_hits_tmp = 0;
  std::vector<std::vector<int>> number_of_hits = {};
  std::vector<std::vector<int>> plane_tmp = {}, slot_tmp = {};
  std::vector<std::vector<double>> pe_tmp = {};
  std::vector<std::vector<std::vector<int>>> plane = {}, slot = {};
  std::vector<std::vector<std::vector<double>>> pe = {};

  for(const auto ninja_hit : ninja_hits) {
    // when scintillators have a gap, create new NINJA cluster
    number_of_hits_tmp++;
    plane_tmp.at(ninja_hit->GetView()).push_back(ninja_hit->GetPlane());
    B2ScintillatorType scintillator_type = (ninja_hit->GetView() == B2View::kTopView) ?
      B2ScintillatorType::kVertical : B2ScintillatorType::kHorizontal;
    B2Readout readout = detector_to_single_readout(B2Detector::kNinja, scintillator_type, ninja_hit->GetPlane());
    //slot_tmp.at(ninja_hit->GetView()).push_back(ninja_hit->GetSlot().GetValue(readout));
    //pe_tmp.at(ninja_hit->GetView()).push_back(ninja_hit->GetHighGainPeu().GetValue(readout));
    
    if ( GetScintillatorPosition(ninja_hit) + 24. < scintillator_position_tmp
	 || ninja_hit->GetView() != view_tmp ) {
      number_of_ninja_clusters++;
      //number_of_hits.at(ninja_hit->GetView()).push_back(number_of_hits_tmp);
      number_of_hits_tmp = 0;
      plane.push_back(plane_tmp); plane_tmp.clear();
      //slot.push_back(slot_tmp); slot_tmp.clear();
      //pe.push_back(pe_tmp); pe_tmp.clear();
    }
    scintillator_position_tmp = GetScintillatorPosition(ninja_hit);
    view_tmp = ninja_hit->GetView();
  }

  ninja_clusters->SetNumberOfNinjaClusters(number_of_ninja_clusters);
  //ninja_clusters->SetNumberOfHits(number_of_hits);
  //ninja_clusters->SetPlane(plane);
  //ninja_clusters->SetSlot(slot);
  //ninja_clusters->SetPe(pe);
  
}

inline std::string ExtractPathWithoutExt(const std::string &fn) {
  std::string::size_type pos;
  if ((pos = fn.find_last_of(".")) == std::string::npos)
    return fn;

  return fn.substr(0, pos);
}

int main(int argc, char *argv[]) {

  logging::core::get()->set_filter
    (
     logging::trivial::severity >= logging::trivial::info
     );

  BOOST_LOG_TRIVIAL(info) << "==========NINJA Track Matching Start==========";

  if (argc != 3) {
    BOOST_LOG_TRIVIAL(error) << "Usage : " << argv[0]
			     << " <input B2 file path> <output B2 file path>";
    std::exit(1);
  }

  try {
    B2Reader reader(argv[1]);
    B2Writer writer(argv[2]);

    TString ntbm_ext = "_ntbm.root";
    TFile *ntbm_writer = new TFile(ExtractPathWithoutExt(argv[2]) + ntbm_ext,
				   "recreate");
    ntbm_writer->cd();
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

      //if (ninja_hits.size() > 0) 
	//CreateNinjaCluster(ninja_hits, my_ntbm);

      // Extrapolate BabyMIND tracks to the NINJA position
      // and get the best cluster to match each BabyMIND track
      auto it_track = input_spill_summary.BeginReconTrack();
      while (const auto *track = it_track.Next()) {
	if(track->HasDetector(B2Detector::kBabyMind)) {

	}
      }
      // Update NINJA hit summary information?
      
      // Create output file
      // writer.Fill();
      ntbm_tree->Fill();

    }

    ntbm_writer->cd();
    ntbm_tree->Write();
    ntbm_writer->Close();
    
  } catch (const std::runtime_error &error) {
    BOOST_LOG_TRIVIAL(fatal) << "Runtime error : " << error.what();
  } catch (const std::invalid_argument &error) {
    BOOST_LOG_TRIVIAL(fatal) << "Invalid argument error : " << error.what();
    std::exit(1);
  }
  
  BOOST_LOG_TRIVIAL(info) << "==========NINJA Track Matching Finish==========";
  std::exit(0);

}
