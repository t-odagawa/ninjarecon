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

int main (int argc, char *argv[]) {

  logging::core::get()->set_filter
    (
     //logging::trivial::severity >= logging::trivial::info
     logging::trivial::severity >= logging::trivial::debug
     );

  BOOST_LOG_TRIVIAL(info) << "==========NINJA Tracker Check Start==========";
  
  if (argc != 3) {
    BOOST_LOG_TRIVIAL(error) << "Usage : " << argv[0]
			     << "<input NTBM file path> <output file path>";
    std::exit(1);
  }
  
  try {
    
    TFile *file = new TFile(argv[1], "read");
    TTree *tree = (TTree*)file->Get("tree");

    NTBMSummary *ntbm = new NTBMSummary();
    tree->SetBranchAddress("NTBMSummary", &ntbm);

    TFile *output = new TFile(argv[2], "recreate");

    TH1D *hist_ang_y = new TH1D("hist_ang_y", "NINJA Tracker reconstructed Y angle;tan#theta_Y;Entries", 100, -2, 2);
    TH1D *hist_ang_x = new TH1D("hist_ang_x", "NINJA Tracker reconstructed X angle;tan#theta_X;Entries", 100, -2, 2);
    TH1D *hist_ang_y_nonzero = new TH1D("hist_ang_y_nonzero", "NINJA Tracker reconstructed Y angle;tan#theta_Y;Entries", 100, -2, 2);
    TH1D *hist_ang_x_nonzero = new TH1D("hist_ang_x_nonzero", "NINJA Tracker reconstructed X angle;tan#theta_X;Entries", 100, -2, 2);
    TH2D *hist_ang_xy = new TH2D("hist_ang_xy", "NINJA Tracker reconstructed tangent;tan#theta_X;tan#theta_Y",
				100, -2, 2, 100, -2, 2);
    TH2D *hist_ang_xy_nonzero = new TH2D("hist_ang_xy_nonzero", "NINJA Tracker reconstructed tangent;tan#theta_X;tan#theta_Y",
					 100, -2, 2, 100, -2, 2);

    for (int ientry = 0; ientry < tree->GetEntries(); ientry++) {
      tree->GetEntry(ientry);
      for (int icluster = 0; icluster < ntbm->GetNumberOfNinjaClusters(); icluster++) {
	// Only 2d matched cluster
	if (ntbm->GetNumberOfHits(icluster, B2View::kSideView) == 0 ||
	    ntbm->GetNumberOfHits(icluster, B2View::kTopView) == 0) continue;
	BOOST_LOG_TRIVIAL(debug) << "2d cluster!";
	std::vector<double> tangent = ntbm->GetNinjaTangent(icluster);
	hist_ang_y->Fill(tangent.at(B2View::kSideView));
	hist_ang_x->Fill(tangent.at(B2View::kTopView));
	hist_ang_xy->Fill(tangent.at(B2View::kTopView), tangent.at(B2View::kSideView));
	if (tangent.at(B2View::kSideView) != 0)
	  hist_ang_y_nonzero->Fill(tangent.at(B2View::kSideView));
	if (tangent.at(B2View::kTopView) != 0)
	  hist_ang_x_nonzero->Fill(tangent.at(B2View::kTopView));
	if (tangent.at(B2View::kSideView) != 0 && tangent.at(B2View::kTopView) != 0)
	  hist_ang_xy_nonzero->Fill(tangent.at(B2View::kTopView), tangent.at(B2View::kSideView));
      }
    }

    output->cd();
    hist_ang_y->Write();
    hist_ang_x->Write();
    hist_ang_xy->Write();
    hist_ang_y_nonzero->Write();
    hist_ang_x_nonzero->Write();
    hist_ang_xy_nonzero->Write();
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
