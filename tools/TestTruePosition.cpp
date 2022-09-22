// system includes
#include <vector>
#include <iostream>

// boost includes
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

// B2 includes
#include <B2Enum.hh>
#include <B2Const.hh>

// root includes
#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TString.h>
#include <TCanvas.h>

#include "NTBMSummary.hh"

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
     );

  BOOST_LOG_TRIVIAL(info) << "==========NINJA Tracker Check Start==========";

  if ( argc != 3 ) {
    BOOST_LOG_TRIVIAL(error) << "Usage : " << argv[0]
			     << " <input NTBM file path> <output file path>";
    std::exit(1);
  }

  try {

    TFile *file = new TFile(argv[1], "read");
    TTree *tree = (TTree*)file->Get("tree");

    NTBMSummary *ntbm = new NTBMSummary();
    tree->SetBranchAddress("NTBMSummary", &ntbm);

    TFile *output = new TFile(argv[2], "recreate");

    TH1D *hist_pos_y = new TH1D("hist_pos_y", "NINJA position difference Y;#Delta Y [mm];Entries", 100,-25, 25);
    TH1D *hist_pos_x = new TH1D("hist_pos_x", "NINJA position differnece X;#Delta X [mm];Entries", 100, -25 ,25);

    TH1D *hist_pos_y_slice[10];
    TH1D *hist_pos_x_slice[10];
    for (int islice = 0; islice < 10; islice++) {
      hist_pos_y_slice[islice] = new TH1D(Form("hist_pos_y_slice%d", islice),
					  Form("Y difference ( %.1f < tan#theta_{Y} < %.1f);#DeltaY [mm];Entries",
					       islice * 0.1, (islice+1) * 0.1),
					  100, -25, 25);
      hist_pos_x_slice[islice] = new TH1D(Form("hist_pos_x_slice%d", islice),
					  Form("X difference ( %.1f < tan#theta_{X} < %.1f);#DeltaX [mm];Entries",
					       islice * 0.1, (islice+1) * 0.1),
					  100, -25, 25);
    }

    TH2D *hist_pos_y_dy = new TH2D("hist_pos_y_dy", ";Y [mm];#Delta Y [mm]", 500, -500, 500, 100, -30, 30);    
    TH2D *hist_pos_x_dx = new TH2D("hist_pos_x_dx", ";X [mm];#Delta X [mm]", 500, -500, 500, 100, -30, 30);
    

    for ( int ientry = 0; ientry < tree->GetEntries(); ientry++ ) {

      tree->GetEntry(ientry);

      if ( ntbm->GetNumberOfTracks() != 1 ) continue;
      double weight = weightcalculator(B2Detector::kNinja, B2Material::kWater,
				       ntbm->GetNormalization(), ntbm->GetTotalCrossSection());
      
      for ( int icluster = 0; icluster< ntbm->GetNumberOfNinjaClusters(); icluster++ ) {
	
	// Only 2d matched cluster
	if ( ntbm->GetNumberOfHits(icluster, B2View::kSideView) == 0 ||
	     ntbm->GetNumberOfHits(icluster, B2View::kTopView) == 0 ) continue;
	if ( ntbm->GetNumberOfTrueParticles(icluster) == 0 ) continue;
	BOOST_LOG_TRIVIAL(debug) << "2d cluster!";
	std::vector<double> ninja_recon_position = ntbm->GetNinjaPosition(icluster);
	std::vector<double> ninja_recon_tangent = ntbm->GetNinjaTangent(icluster);
	std::vector<double> ninja_true_position = ntbm->GetTruePosition(icluster, 0);
	
	hist_pos_y->Fill(ninja_recon_position.at(B2View::kSideView) - ninja_true_position.at(B2View::kSideView), weight);
	hist_pos_x->Fill(ninja_recon_position.at(B2View::kTopView) - ninja_true_position.at(B2View::kTopView), weight);
	hist_pos_y_dy->Fill(ninja_true_position.at(B2View::kSideView),
			    ninja_recon_position.at(B2View::kSideView) - ninja_true_position.at(B2View::kSideView),
			    weight);
	hist_pos_x_dx->Fill(ninja_true_position.at(B2View::kTopView),
			    ninja_recon_position.at(B2View::kTopView) - ninja_true_position.at(B2View::kTopView),
			    weight);
	int side_bin_id = GetBinId(ninja_recon_tangent.at(B2View::kSideView));
	int top_bin_id = GetBinId(ninja_recon_tangent.at(B2View::kTopView));
	if (side_bin_id < 10)
	  hist_pos_y_slice[side_bin_id]->Fill(ninja_recon_position.at(B2View::kSideView) - ninja_true_position.at(B2View::kSideView), weight);
	if (top_bin_id < 10)
	  hist_pos_x_slice[top_bin_id]->Fill(ninja_recon_position.at(B2View::kTopView) - ninja_true_position.at(B2View::kTopView), weight);
	
      } // icluster

    } // ientry

    output->cd();
    hist_pos_y->Write();
    hist_pos_x->Write();
    for (int islice = 0; islice < 10; islice++) {
      hist_pos_y_slice[islice]->Write();
      hist_pos_x_slice[islice]->Write();
    }
    hist_pos_y_dy->Write();
    hist_pos_x_dx->Write();
    output->Close();

  } catch (const std::runtime_error &error) {
    BOOST_LOG_TRIVIAL(fatal) << "Runtime error :" << error.what();
    std::exit(1);
  } catch (const std::invalid_argument &error) {
    BOOST_LOG_TRIVIAL(fatal) << "Invalid argument error : " << error.what();
    std::exit(1);
  }

  BOOST_LOG_TRIVIAL(info) << "==========NINJA Tracker Check Start==========";
  std::exit(0);

}
