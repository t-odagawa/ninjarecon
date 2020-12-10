// system includes
#include <string>
#include <vector>
#include <iostream>

// boost includes
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

// root includes
#include <TString.h>
#include <TFile.h>

// B2 includes
#include "B2Reader.hh"
#include "B2Writer.hh"
#include "B2HitSummary.hh"

namespace logging = boost::log;

///> number of slots used in the NINJA tracker
const Int_t NUM_SLOTS = 250;

/**
 * Get corresponding NINJA spill number (entry number in the NINJA tracker root file)
 * to the WAGASCI spill summary
 * @param input_spill_summary WAGASCI/BabyMIND spill summary
 * @param tree TTree in the NINJA tracker root file
 * @param start # of entry to start (not to consider already assigned entries)
 * @return corresponding entry number in the root file
 */
Int_t GetNinjaSpill(const B2SpillSummary &input_spill_summary, TTree *tree, Int_t &start,
		    UInt_t ut[NUM_SLOTS]) {
  
  BOOST_LOG_TRIVIAL(debug) << "Getting NINJA entry # for BSD spill" << input_spill_summary.GetBeamSummary().GetBsdSpillNumber();

  //const Double_t wagasci_time = spill_summary.GetBeamSummary().GetTimeStamp();
  const Double_t wagasci_time = 1555000000;
  
  for(int ientry = start; ientry < tree->GetEntries(); ientry++) {
    tree->GetEntry(ientry);
    break;
    if(ut[0] == wagasci_time) {
      return ientry;
    }
  }
  return B2_NON_INITIALIZED_VALUE;
}

// main function
int main(int argc, char *argv[]) {

  logging::core::get()->set_filter
    (
     logging::trivial::severity >= logging::trivial::info
     );
  
  BOOST_LOG_TRIVIAL(info) << "==========NINJA Hit Converter Start==========";

  if (argc != 4) {
    BOOST_LOG_TRIVIAL(error) << "Usage : "<< argv[0]
			     << " <input wagasci file path> <input ninja file path> <output file path>";
    std::exit(1);
  }

  try {
    B2Reader reader(argv[1]);
    TFile *ntfile = new TFile(argv[2], "read");
    B2Writer writer(argv[3]);
    BOOST_LOG_TRIVIAL(info) << "-----Settings Summary-----";
    BOOST_LOG_TRIVIAL(info) << "Reader  file : " << argv[1];
    BOOST_LOG_TRIVIAL(info) << "Tracker file : " << argv[2];
    BOOST_LOG_TRIVIAL(info) << "Writer  file : " << argv[3];

    // Tracker file settings
    BOOST_LOG_TRIVIAL(info) << "Tracker file tree setting...";
    TTree *nttree = (TTree*)ntfile->Get("tree");

    Int_t adc[NUM_SLOTS], tt[NUM_SLOTS], lt[NUM_SLOTS];
    UInt_t unixtime[NUM_SLOTS];
    Float_t pe[NUM_SLOTS];
    Int_t view[NUM_SLOTS], pln[NUM_SLOTS], ch[NUM_SLOTS];
    
    nttree->SetBranchAddress("ADC", adc);
    nttree->SetBranchAddress("LEADTIME", lt);
    nttree->SetBranchAddress("TRAILTIME", tt);
    nttree->SetBranchAddress("UNIXTIME", unixtime);
    nttree->SetBranchAddress("PE", pe);
    nttree->SetBranchAddress("VIEW", view);
    nttree->SetBranchAddress("PLN", pln);
    nttree->SetBranchAddress("CH", ch);
    BOOST_LOG_TRIVIAL(info) << "done!";

    Int_t ntentry = 1; // Tracker file entry
    const Int_t ntentry_max = nttree->GetEntries();

    while (reader.ReadNextSpill() > 0 && ntentry < ntentry_max) {

      auto &input_spill_summary = reader.GetSpillSummary();

      // Get corresponding NINJA entry
      ntentry = nttree->GetEntry(GetNinjaSpill(input_spill_summary, nttree, ntentry, unixtime));

      // Create output spill summary
      auto &output_spill_summary = writer.GetSpillSummary();      

      input_spill_summary.CloneRecon(output_spill_summary, kFALSE);
#ifdef MC
      input_spill_summary.CloneTrue(output_spill_summary, kFALSE);
      input_spill_summary.CloneEmulsions(output_spill_summary, kFALSE);
#endif
      // Add NINJA entry as B2HitSummary
      auto &output_hit_summary = output_spill_summary.AddHit();

      for(int slot = 0; slot < NUM_SLOTS; slot++) {
	if(pe[slot] < 2.5) continue;
	
	output_hit_summary.SetDetector(B2Detector::kNinja);
	//output_hit_summary.SetRelativePosition();
	//output_hit_summary.SetAbsolutePosition();
	//output_hit_summary.SetScintillatorPosition();
	output_hit_summary.SetPlane(pln[slot]);
	//output_hit_summary.SetSlot();
	output_hit_summary.SetTrueTimeNs(lt[slot]-tt[slot]); // Time over Threshold stored
	switch (view[slot]) {
	case B2View::kTopView :
	  output_hit_summary.SetView(B2View::kTopView);
	  output_hit_summary.SetScintillatorType(B2ScintillatorType::kVertical);
	  output_hit_summary.SetHighGainPeu(B2Readout::kTopReadout, pe[slot]);
	  break;
	case B2View::kSideView :
	  output_hit_summary.SetView(B2View::kSideView);
	  output_hit_summary.SetScintillatorType(B2ScintillatorType::kHorizontal);
	  output_hit_summary.SetHighGainPeu(B2Readout::kSideReadout, pe[slot]);
	  break;
	case B2View::kUnknownView :
	  BOOST_LOG_TRIVIAL(error) << "Unknown view";
	  break;
	}
	
      }
      
      writer.Fill();
      
    }
  
  } catch (const std::runtime_error &error) {
    BOOST_LOG_TRIVIAL(fatal) << "Runtime error : " << error.what();
    std::exit(1);
  } catch (const std::invalid_argument &error) {
    BOOST_LOG_TRIVIAL(fatal) << "Invalid argument error : " << error.what();
    std::exit(1);
  }

  BOOST_LOG_TRIVIAL(info) << "==========NINJA Hit Converter Finish==========";
  std::exit(0);
}
