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
     logging::trivial::severity >= logging::trivial::info
     //logging::trivial::severity >= logging::trivial::debug
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

    TH1D *hist_pos_y = new TH1D("hist_pos_y", "NINJA Tracker reconstructed Y;Y [mm];Entries", 100, -650, 650);
    TH1D *hist_pos_x = new TH1D("hist_pos_x", "NINJA Tracker reconstructed X;X [mm];Entries", 100, -650, 650);
    TH2D *hist_pos_xy = new TH2D("hist_pos_xy", "NINJA Tracker reconstructed position;X [mm];Y [mm]",
				100, -650, 650, 100, -650, 650);

    for (int ientry = 0; ientry < tree->GetEntries(); ientry++) {
      tree->GetEntry(ientry);
      for (int icluster = 0; icluster < ntbm->GetNumberOfNinjaClusters(); icluster++) {
	// Only 2d matched cluster
	if (ntbm->GetNumberOfHits(icluster, B2View::kSideView) == 0 ||
	    ntbm->GetNumberOfHits(icluster, B2View::kTopView) == 0) continue;
	BOOST_LOG_TRIVIAL(debug) << "2d cluster!";
	std::vector<double> position = ntbm->GetNinjaPosition(icluster);
	hist_pos_y->Fill(position.at(B2View::kSideView));
	hist_pos_x->Fill(position.at(B2View::kTopView));
	hist_pos_xy->Fill(position.at(B2View::kTopView), position.at(B2View::kSideView));
      }
    }

    output->cd();
    hist_pos_y->Write();
    hist_pos_x->Write();
    hist_pos_xy->Write();
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
