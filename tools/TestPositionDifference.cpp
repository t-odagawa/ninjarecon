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

int GetBinId(double tangent) {
  return std::floor(std::fabs(tangent) * 10);
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

    TH1D *hist_pos_y = new TH1D("hist_pos_y", "Y difference;#DeltaY [mm];Entries", 250, -500, 500);
    TH1D *hist_pos_x = new TH1D("hist_pos_x", "X difference;#DeltaX [mm];Entries", 250, -500, 500);
    TH1D *hist_ang_y =  new TH1D("hist_ang_y", "tan Y difference;#Deltatan_Y;Entries", 100, -0.25, 0.25);
    TH1D *hist_ang_x =  new TH1D("hist_ang_x", "tan X difference;#Deltatan_X;Entries", 100, -0.25, 0.25);

    TH1D *hist_pos_y_slice[10];
    TH1D *hist_pos_x_slice[10];
    for (int islice = 0; islice < 10; islice++) {
      hist_pos_y_slice[islice] = new TH1D(Form("hist_pos_y_slice%d", islice),
					  Form("Y difference ( %.1f < tan#theta_{Y} < %.1f);#DeltaY [mm];Entries",
					       islice * 0.1, (islice+1) * 0.1),
					  250, -500, 500);
      hist_pos_x_slice[islice] = new TH1D(Form("hist_pos_x_slice%d", islice),
					  Form("X difference ( %.1f < tan#theta_{X} < %.1f);#DeltaX [mm];Entries",
					       islice * 0.1, (islice+1) * 0.1),
					  250, -500, 500);
    }

    Int_t datatype = atoi(argv[3]);

    for (int ientry = 0; ientry < tree->GetEntries(); ientry++) {
      tree->GetEntry(ientry);
      double weight = 1.;
      switch (datatype) {
      case B2DataType::kMonteCarlo :
	weight = weightcalculator(B2Detector::kWall, B2Material::kWater,
				  ntbm->GetNormalization(), ntbm->GetTotalCrossSection());
	break;
      case B2DataType::kRealData :
	weight = 1.;
	break;
      default :
	throw std::invalid_argument("Datatype should be 0 (MC) or 1 (Physics data)!!");
      }

      for (int icluster = 0; icluster < ntbm->GetNumberOfNinjaClusters(); icluster++) {

	// Only 2d matched cluster
	if (ntbm->GetNumberOfHits(icluster, B2View::kSideView) == 0 ||
	    ntbm->GetNumberOfHits(icluster, B2View::kTopView) == 0) continue;

	BOOST_LOG_TRIVIAL(debug) << "2d cluster!";
	int baby_mind_track_id = ntbm->GetBabyMindTrackId(icluster);

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
	      - NINJA_TRACKER_POS_X; // NINJA tracker
	    break;
	  case B2View::kSideView :
	    hit_expected_position.at(view) = baby_mind_position.at(view)
	      - baby_mind_tangent.at(view) * (BABYMIND_POS_Z + BM_SECOND_LAYER_POS
					      - NINJA_POS_Z - NINJA_TRACKER_POS_Z + 10.);
	    hit_expected_position.at(view) = hit_expected_position.at(view)
	      + BABYMIND_POS_Y // global coordinate
	      - NINJA_POS_Y // NINJA overall
	      - NINJA_TRACKER_POS_Y; // NINJA tracker
	    break;
	  }
	}
	std::vector<double> ninja_position = ntbm->GetNinjaPosition(icluster);
	std::vector<double> ninja_tangent = ntbm->GetNinjaTangent(icluster);
	hist_pos_y->Fill(hit_expected_position.at(B2View::kSideView) - ninja_position.at(B2View::kSideView), weight);
	hist_pos_x->Fill(hit_expected_position.at(B2View::kTopView) - ninja_position.at(B2View::kTopView), weight);
	hist_ang_y->Fill(baby_mind_tangent.at(B2View::kSideView) - ninja_tangent.at(B2View::kSideView), weight);
	hist_ang_x->Fill(baby_mind_tangent.at(B2View::kTopView) - ninja_tangent.at(B2View::kTopView), weight);
	int side_bin_id = GetBinId(baby_mind_tangent.at(B2View::kSideView));
	int top_bin_id = GetBinId(baby_mind_tangent.at(B2View::kTopView));
	if (side_bin_id < 10)
	  hist_pos_y_slice[side_bin_id]->Fill(hit_expected_position.at(B2View::kSideView) - ninja_position.at(B2View::kSideView), weight);
	if (top_bin_id < 10)
	  hist_pos_x_slice[top_bin_id]->Fill(hit_expected_position.at(B2View::kTopView) - ninja_position.at(B2View::kTopView), weight);
      } // icluster
    } // ientry
    
    output->cd();
    hist_pos_y->Write();
    hist_pos_x->Write();
    hist_ang_y->Write();
    hist_ang_x->Write();
    for (int islice = 0; islice < 10; islice++) {
      hist_pos_y_slice[islice]->Write();
      hist_pos_x_slice[islice]->Write();
    }
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
