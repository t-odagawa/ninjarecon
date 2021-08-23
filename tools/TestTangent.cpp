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

    TH1D *hist_ang_y = new TH1D("hist_ang_y", "NINJA Tracker reconstructed Y angle;tan#theta_{Y};Entries", 100, -2, 2);
    TH1D *hist_ang_x = new TH1D("hist_ang_x", "NINJA Tracker reconstructed X angle;tan#theta_{X};Entries", 100, -2, 2);
    TH2D *hist_ang_xy = new TH2D("hist_ang_xy", "NINJA Tracker reconstructed tangent;tan#theta_{X};tan#theta_{Y}",
				100, -2, 2, 100, -2, 2);

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
	std::vector<double> tangent = ntbm->GetNinjaTangent(icluster);
	hist_ang_y->Fill(tangent.at(B2View::kSideView), weight);
	hist_ang_x->Fill(tangent.at(B2View::kTopView), weight);
	hist_ang_xy->Fill(tangent.at(B2View::kTopView), tangent.at(B2View::kSideView), weight);
      }
    }

    output->cd();
    hist_ang_y->Write();
    hist_ang_x->Write();
    hist_ang_xy->Write();
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
