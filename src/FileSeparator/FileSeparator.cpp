// boost includes
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

// B2 includes
#include "B2Reader.hh"

namespace logging = boost::log;

///> number of slots used in the NINJA tracker
const int NUM_SLOTS = 250;

int main(int argc, char *argv[]) {

  logging::core::get()->set_filter
    (
     logging::trivial::severity >= logging::trivial::debug
     );

  BOOST_LOG_TRIVIAL(info) << "==========NINJA File Separator Start==========";

  if (argc != 4) {
    BOOST_LOG_TRIVIAL(error) << "Usage : " << argv[0]
			     << " <input wagasci file path> <input ninja file path> <output ninja file path>";
    std::exit(1);
  }

  try {

    B2Reader reader(argv[1]);
    TFile *input_nt_file = new TFile(argv[2], "read");
    TFile *output_nt_file = new TFile(argv[3], "recreate");
    BOOST_LOG_TRIVIAL(info) << "-----Settings Summary-----";
    BOOST_LOG_TRIVIAL(info) << "Reader file : " << argv[1];
    BOOST_LOG_TRIVIAL(info) << "Tracker input  file : " << argv[2];
    BOOST_LOG_TRIVIAL(info) << "Tracker output file : " << argv[3];

    // Tracker input file settings
    TTree *input_nt_tree = (TTree*)input_nt_file->Get("tree");

    Int_t adc[NUM_SLOTS], tt[NUM_SLOTS], lt[NUM_SLOTS];
    UInt_t unixtime[NUM_SLOTS];
    Float_t pe[NUM_SLOTS];
    Int_t view[NUM_SLOTS], pln[NUM_SLOTS], ch[NUM_SLOTS];

    input_nt_tree->SetBranchAddress("ADC", adc);
    input_nt_tree->SetBranchAddress("LEADTIME", lt);
    input_nt_tree->SetBranchAddress("TRAILTIME", tt);
    input_nt_tree->SetBranchAddress("UNIXTIME", unixtime);
    input_nt_tree->SetBranchAddress("PE", pe);
    input_nt_tree->SetBranchAddress("VIEW", view);
    input_nt_tree->SetBranchAddress("PLN", pln);
    input_nt_tree->SetBranchAddress("CH", ch);

    BOOST_LOG_TRIVIAL(debug) << "Tracker input file setting done";

    // Tracker output file settings
    TTree *output_nt_tree = new TTree("tree", "tree");
    
    output_nt_tree->Branch("ADC", adc, Form("ADC[%d]/I", NUM_SLOTS));
    output_nt_tree->Branch("LEADTIME", lt, Form("LEADTIME[%d]/I", NUM_SLOTS));
    output_nt_tree->Branch("TRAILTIME", tt, Form("TRAILTIME[%d]/I", NUM_SLOTS));
    output_nt_tree->Branch("UNIXTIME", unixtime, Form("UNIXTIME[%d]/i", NUM_SLOTS));
    output_nt_tree->Branch("PE", pe, Form("PE[%d]/F", NUM_SLOTS));
    output_nt_tree->Branch("VIEW", view, Form("VIEW[%d]/I", NUM_SLOTS));
    output_nt_tree->Branch("PLN", pln, Form("PLN[%d]/I", NUM_SLOTS));
    output_nt_tree->Branch("CH", ch, Form("CH[%d]/I", NUM_SLOTS));

    BOOST_LOG_TRIVIAL(debug) << "Tracker output file setting done";

    int start_time = 0, end_time = 0;


    while(reader.ReadNextSpill() > 0) {
      auto &spill_summary = reader.GetSpillSummary();
      if (reader.GetEntryNumber() == 1) {
	start_time = spill_summary.GetBeamSummary().GetTimestamp();
      }
      end_time = spill_summary.GetBeamSummary().GetTimestamp();
    }

    BOOST_LOG_TRIVIAL(debug) << "Start Unixtime : " << start_time;
    BOOST_LOG_TRIVIAL(debug) << "End Unixtime : " << end_time;

    if (start_time == 0 || end_time == 0) {
      BOOST_LOG_TRIVIAL(error) << "Start (End) Unixtime not set";
      BOOST_LOG_TRIVIAL(error) << "Start Unixtime : " << start_time;
      BOOST_LOG_TRIVIAL(error) << "End Unixtime : " << end_time;
      std::exit(1);
    }

    BOOST_LOG_TRIVIAL(info) << "-----NINJA tracker data extraction start-----";
    
    for (int nt_entry = 0; nt_entry < input_nt_tree->GetEntries(); nt_entry++) {

      input_nt_tree->GetEntry(nt_entry);
      
      if ((int)unixtime[0] < start_time - 1 ||
	  (int)unixtime[0] > end_time + 1) continue;
      
      output_nt_tree->Fill();
    }

    output_nt_file->cd();
    output_nt_tree->Write();
    output_nt_file->Close();

  } catch (const std::runtime_error &error) {
    BOOST_LOG_TRIVIAL(fatal) << "Runtime error : " << error.what();
    std::exit(1);
  } catch (const std::invalid_argument &error) {
    BOOST_LOG_TRIVIAL(fatal) << "Invalid argument error : " << error.what();
    std::exit(1);
  }

  BOOST_LOG_TRIVIAL(info) << "==========NINJA File Separator Finish==========";
  std::exit(0);
  
}
