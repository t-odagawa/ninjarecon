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

    TH1D *hist_pos_y = new TH1D("hist_pos_y", "Y difference;#DeltaY [mm];Entries", 100, -200, 200);
    TH1D *hist_pos_x = new TH1D("hist_pos_x", "X difference;#DeltaX [mm];Entries", 100, -200, 200);
    TH1D *hist_ang_y =  new TH1D("hist_ang_y", "tan Y difference;#Deltatan_Y;Entries", 100, -0.25, 0.25);
    TH1D *hist_ang_x =  new TH1D("hist_ang_x", "tan X difference;#Deltatan_X;Entries", 100, -0.25, 0.25);

    for (int ientry = 0; ientry < tree->GetEntries(); ientry++) {
      tree->GetEntry(ientry);
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
	  hit_expected_position.at(view) = baby_mind_position.at(view)
	    - baby_mind_tangent.at(view) * NINJA_BABYMIND_DISTANCE[view];
	  switch (view) {
	  case B2View::kTopView :
	    hit_expected_position.at(view) = hit_expected_position.at(view)
	      + BABYMIND_POS_X // global coordinate
	      - NINJA_POS_X // NINJA overall
	      - NINJA_TRACKER_POS_X; // NINJA tracker
	    break;
	  case B2View::kSideView :
	    hit_expected_position.at(view) = hit_expected_position.at(view)
	      + BABYMIND_POS_Y // global coordinate
	      - NINJA_POS_Y // NINJA overall
	      - NINJA_TRACKER_POS_Y; // NINJA tracker
	    break;
	  }
	}
	std::vector<double> ninja_position = ntbm->GetNinjaPosition(icluster);
	std::vector<double> ninja_tangent = ntbm->GetNinjaTangent(icluster);
	hist_pos_y->Fill(hit_expected_position.at(B2View::kSideView) - ninja_position.at(B2View::kSideView));
	hist_pos_x->Fill(hit_expected_position.at(B2View::kTopView) - ninja_position.at(B2View::kTopView));
	hist_ang_y->Fill(baby_mind_tangent.at(B2View::kSideView) - ninja_tangent.at(B2View::kSideView));
	hist_ang_x->Fill(baby_mind_tangent.at(B2View::kTopView) - ninja_tangent.at(B2View::kTopView));
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
