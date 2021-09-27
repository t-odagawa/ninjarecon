// system includes
#include <vector>
#include <iostream>

// boost includes
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

// B2 includes
#include <B2Enum.hh>

// root includes
#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TString.h>
#include <TCanvas.h>

#include "NTBMSummary.hh"
#include "NTBMConst.hh"

namespace logging = boost::log;

double weightcalculator(int detector, int material, double norm, double xsec) {
  static const double avogadro_constant = 6.0 * std::pow(10, 23);
  static const double xsec_factor = std::pow(10, -38);
  double thick;
  switch (detector) {
  case B2Detector::kProtonModule :
    thick = PM_SCIBAR_REGION_THICKNESS + PM_VETO_REGION_THICKNESS;
    break;
  case B2Detector::kWagasciUpstream :
  case B2Detector::kWagasciDownstream :
    thick = WGS_WATER_BOX_DEPTH;
    break;
  case B2Detector::kWallMrdSouth :
  case B2Detector::kWallMrdNorth :
    thick = WM_INNER_IRON_PLATE_LARGE;
    break;
  case B2Detector::kBabyMind :
    thick = BM_IRON_PLATE_DEPTH * BM_NUM_IRON_PLANES;
    break;
  case B2Detector::kYasuTracker :
    thick = YASU_NUM_PLANES * WM_SCINTI_THICK;
    break;
  case B2Detector::kWall :
    thick = HALL_RADIUS_THICK;
    break;
  case B2Detector::kNinja :
    switch (material) {
    case B2Material::kWater :
      thick = NINJA_WATER_LAYER_THICK * NINJA_ECC_WATER_LAYERS;
      break;
    case B2Material::kIron :
      thick = NINJA_IRON_LAYER_THICK * NINJA_ECC_IRON_LAYERS;
      break;
    default :
      throw std::invalid_argument("NINJA material not recognized");
    }
    break;
  default :
    throw std::invalid_argument("Detector not recognized");
  }

  thick *= .1; // convert from mm to cm
  return avogadro_constant * thick * norm * xsec * xsec_factor;

}

int main (int argc, char *argv[]) {

  logging::core::get()->set_filter
    (
     logging::trivial::severity >= logging::trivial::info
     //logging::trivial::severity >= logging::trivial::debug
     );

  BOOST_LOG_TRIVIAL(info) << "==========NINJA Tracker Check Start==========";
  
  if (argc != 4) {
    BOOST_LOG_TRIVIAL(error) << "Usage : " << argv[0]
			     << " <input NTBM file path> <output file path> <0 (MC)/1(Physics data)>";
    std::exit(1);
  }
  
  try {
    
    TFile *file = new TFile(argv[1], "read");
    TTree *tree = (TTree*)file->Get("tree");

    NTBMSummary *ntbm = new NTBMSummary();
    tree->SetBranchAddress("NTBMSummary", &ntbm);

    TFile *output = new TFile(argv[2], "recreate");

    TH1D *hist_pos_y = new TH1D("hist_pos_y", "Y difference;#DeltaY [mm];Entries", 100, -200, 200);
    TH1D *hist_pos_x = new TH1D("hist_pos_x", "X difference;#DeltaX [mm];Entries", 100, -200, 200);
    TH1D *hist_ang_y =  new TH1D("hist_ang_y", "tan Y difference;#Deltatan_Y;Entries", 100, -0.25, 0.25);
    TH1D *hist_ang_x =  new TH1D("hist_ang_x", "tan X difference;#Deltatan_X;Entries", 100, -0.25, 0.25);

    Int_t datatype = atoi(argv[3]);

    for (int ientry = 0; ientry < tree->GetEntries(); ientry++) {
      tree->GetEntry(ientry);
      double weight = 1.;
      if (datatype == 0)
	double weight = weightcalculator(B2Detector::kWall, B2Material::kWater,
					 ntbm->GetNormalization(), ntbm->GetTotalCrossSection());
      else if (datatype == 1)
	double weight = 1.;
      else 
	throw std::invalid_argument("Datatype should be 0 (MC) or 1 (Physics data)!!");

      for (int icluster = 0; icluster < ntbm->GetNumberOfNinjaClusters(); icluster++) {
	// Only 2d matched cluster
	if (ntbm->GetNumberOfHits(icluster, B2View::kSideView) == 0 ||
	    ntbm->GetNumberOfHits(icluster, B2View::kTopView) == 0) continue;
	BOOST_LOG_TRIVIAL(debug) << "2d cluster!";
	int baby_mind_track_id = ntbm->GetBabyMindTrackId(icluster);
	if (baby_mind_track_id == -1) continue;
	std::vector<double> baby_mind_position = ntbm->GetBabyMindPosition(baby_mind_track_id);
	std::vector<double> baby_mind_tangent = ntbm->GetBabyMindTangent(baby_mind_track_id);
	std::vector<double> hit_expected_position(2);
	for (int view = 0; view < 2; view++) {
	  switch (view) {
	  case B2View::kTopView :
	  hit_expected_position.at(view) = baby_mind_position.at(view)
	    - baby_mind_tangent.at(view) * (BABYMIND_POS_Z + BM_SECOND_LAYER_POS
					    - NINJA_POS_Z - NINJA_TRACKER_POS_Z - 10.);
	    hit_expected_position.at(view) = hit_expected_position.at(view)
	      + BABYMIND_POS_X // global coordinate
	      - NINJA_POS_X // NINJA overall
	      - NINJA_TRACKER_POS_X + 15.; // NINJA tracker
	    break;
	  case B2View::kSideView :
	  hit_expected_position.at(view) = baby_mind_position.at(view)
	    - baby_mind_tangent.at(view) * (BABYMIND_POS_Z + BM_SECOND_LAYER_POS
					    - NINJA_POS_Z - NINJA_TRACKER_POS_Z + 10.);
	    hit_expected_position.at(view) = hit_expected_position.at(view)
	      + BABYMIND_POS_Y // global coordinate
	      - NINJA_POS_Y // NINJA overall
	      - NINJA_TRACKER_POS_Y + 21.; // NINJA tracker
	    break;
	  }
	}
	std::vector<double> ninja_position = ntbm->GetNinjaPosition(icluster);
	std::vector<double> ninja_tangent = ntbm->GetNinjaTangent(icluster);
	hist_pos_y->Fill(hit_expected_position.at(B2View::kSideView) - ninja_position.at(B2View::kSideView), weight);
	hist_pos_x->Fill(hit_expected_position.at(B2View::kTopView) - ninja_position.at(B2View::kTopView), weight);
	hist_ang_y->Fill(baby_mind_tangent.at(B2View::kSideView) - ninja_tangent.at(B2View::kSideView), weight);
	hist_ang_x->Fill(baby_mind_tangent.at(B2View::kTopView) - ninja_tangent.at(B2View::kTopView), weight);
      }
    }

    output->cd();
    hist_pos_y->Write();
    hist_pos_x->Write();
    hist_ang_y->Write();
    hist_ang_x->Write();
    output->Close();
    
  } catch (const std::runtime_error &error) {
    BOOST_LOG_TRIVIAL(fatal) << "Runtime error : " << error.what();
    std::exit(1);
  } catch (const std::invalid_argument &error) {
    BOOST_LOG_TRIVIAL(fatal) << "Invalide argument error : " << error.what();
    std::exit(1);
  }

  BOOST_LOG_TRIVIAL(info) << "==========NINJA Tracker Check Finish==========";
  std::exit(0);

}
