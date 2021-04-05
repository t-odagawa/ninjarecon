// boost includes
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

// root includes
#include <TFile.h>
#include <TTree.h>
#include <TSpectrum.h>
#include <TF1.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TCanvas.h>

namespace logging = boost::log;

///> number of slosts used in the NINJA tracker
const int NUM_SLOTS = 250;

int main (int argc, char *argv[]) {

  logging::core::get()->set_filter
    (
     // logging::trivial::severity >= logging::trivial::info
     logging::trivial::severity >= logging::trivial::debug
     );

  BOOST_LOG_TRIVIAL(info) << "==========Multi Hit TDC Evaluation Start==========";

  if (argc != 4) {
    BOOST_LOG_TRIVIAL(error) << "Usage : " << argv[0]
			     << " <input file name> <output pdf path> <evaluation mode>";
    std::exit(1);
  }

  try {

    BOOST_LOG_TRIVIAL(info) << "-----Setting Summary-----";
    BOOST_LOG_TRIVIAL(info) << "Input file :" << argv[1];
    BOOST_LOG_TRIVIAL(info) << "Output file : " << argv[2];

    TFile *ifile = new TFile(argv[1], "read");
    TTree* itree = (TTree*)ifile->Get("tree");
    Int_t lt[NUM_SLOTS], tt[NUM_SLOTS];
    Float_t pe[NUM_SLOTS];
    itree->SetBranchAddress("LEADTIME", lt);
    itree->SetBranchAddress("TRAILTIME", tt);
    itree->SetBranchAddress("PE", pe);

    //TH1D *h_lt = new TH1D("h_lt", "Leading Time;Leading time [ns];Entries", 1024, 0, 4096);
    TH1D *h_lt = new TH1D("h_lt", "Leading Time;Leading time [ns];Entries", 600, 100, 3700);

    for (Int_t ientry = 0; ientry < itree->GetEntries(); ientry++) {
      itree->GetEntry(ientry);
      for (Int_t islot = 0; islot < NUM_SLOTS; islot++) {
	if (islot == 50 || islot == 115) continue;
	h_lt->Fill(lt[islot]);
      }
    }

    TCanvas *c = new TCanvas("c","c");
    TString canvasname = argv[2];
    c->Print(canvasname + "[", "pdf");

    c->cd();
    h_lt->Draw();
    gPad->SetLogy();

    TSpectrum *s = new TSpectrum(8);
    s->Search(h_lt, 5);
    Double_t *xpeaks = s->GetPositionX();
    const Int_t npeaks = s->GetNPeaks();
    std::sort(xpeaks, xpeaks + npeaks);

    TString fitfuncname = "";
    for (Int_t ipeaks = 0; ipeaks < npeaks; ipeaks++) {
      fitfuncname += Form("gaus(%d) + ", 3 * ipeaks);
    }

    fitfuncname += Form("[%d]", 3 * npeaks);
    BOOST_LOG_TRIVIAL(debug) << "Fitting function name : " << fitfuncname;
    TF1 *fitfunc = new TF1("fitfunc", fitfuncname);
    for (Int_t ipeaks = 0; ipeaks < npeaks; ipeaks++) {
      fitfunc->SetParameter(3 * ipeaks + 1, xpeaks[ipeaks]);
      fitfunc->SetParameter(3 * ipeaks + 2, 60);
    }
    fitfunc->SetParameter(3 * npeaks, 1e3);

    h_lt->Fit(fitfunc);
    c->Print(canvasname, "pdf");
    c->Print(canvasname + "]", "pdf");

    BOOST_LOG_TRIVIAL(info) << "Bunch leadtime : ( "
			    << fitfunc->GetParameter(1)  << ", "
			    << fitfunc->GetParameter(4)  << ", "
			    << fitfunc->GetParameter(7)  << ", "
			    << fitfunc->GetParameter(10) << ", "
			    << fitfunc->GetParameter(13) << ", "
			    << fitfunc->GetParameter(16) << " )";

    BOOST_LOG_TRIVIAL(info) << "Bunch difference : ( "
			    << fitfunc->GetParameter(4)  - fitfunc->GetParameter(1)  << ", " 
			    << fitfunc->GetParameter(7)  - fitfunc->GetParameter(4)  << ", "
			    << fitfunc->GetParameter(10) - fitfunc->GetParameter(7)  << ", "
			    << fitfunc->GetParameter(13) - fitfunc->GetParameter(10) << ", "
			    << fitfunc->GetParameter(16) - fitfunc->GetParameter(13) << " )";
     
    BOOST_LOG_TRIVIAL(info) << "Bunch width : ( " 
			    << 3 * fitfunc->GetParameter(2)  << ", "
			    << 3 * fitfunc->GetParameter(5)  << ", "
			    << 3 * fitfunc->GetParameter(8)  << ", "
			    << 3 * fitfunc->GetParameter(11) << ", "
			    << 3 * fitfunc->GetParameter(14) << ", "
			    << 3 * fitfunc->GetParameter(17) << " )";

  } catch (const std::runtime_error &error) {
    BOOST_LOG_TRIVIAL(fatal) << "Runtime error : " << error.what();
    std::exit(1);
  } catch (const std::invalid_argument &error) {
    BOOST_LOG_TRIVIAL(fatal) << "Invalid argument error : " << error.what();
    std::exit(1);
  }

  BOOST_LOG_TRIVIAL(info) << "==========Multi Hit TDC Evaluation Finish==========";
  std::exit(0);

}

