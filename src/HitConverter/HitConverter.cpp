// system includes
#include <vector>

// boost includes
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

// root includes
#include <TFile.h>
#include <TTree.h>

// B2 includes
#include "B2Reader.hh"
#include "B2Writer.hh"
#include "B2Dimension.hh"
#include "B2Enum.hh"
#include "B2Measurement.hh"
#include "B2HitSummary.hh"
#include "B2SpillSummary.hh"
#include "B2BeamSummary.hh"

#include "NTBMConst.hh"

namespace logging = boost::log;


/**
 * Get corresponding NINJA spill number (entry number in the NINJA tracker root file)
 * to the WAGASCI spill summary
 * @param output_spill_summary WAGASCI/BabyMIND spill summary
 * @param tree TTree in the NINJA tracker root file
 * @param start # of entry to start (not to consider already assigned entries)
 * @return corresponding entry number in the root file
 */
Int_t GetNinjaSpill(const B2SpillSummary &output_spill_summary, TTree *tree, Int_t start,
		    UInt_t ut[NUMBER_OF_SLOTS_IN_TRACKER]) {

  const Double_t wagasci_time = output_spill_summary.GetBeamSummary().GetTimestamp();

  BOOST_LOG_TRIVIAL(debug) << "Getting NINJA entry # for BSD unixtime :  "
			   << (Int_t) wagasci_time;

  for(int ientry = start; ientry < tree->GetEntries(); ientry++) {
    tree->GetEntry(ientry);
    if(std::fabs(((Int_t)ut[0] - (Int_t)wagasci_time)) < 2) { // Check proper cast TODO
      BOOST_LOG_TRIVIAL(debug) << "Found NINJA entry # " << ientry
			       << ": Unixtime : " << ut[0];
      return ientry;
    }
  }
  return B2_NON_INITIALIZED_VALUE;
}

bool IsUnusedSlot(Int_t slot) {
  if (slot == 50 || slot == 115) return true;
  else return false;
}

bool IsNoisySlot(Int_t slot) {
  if (slot == 150) return true;
  else return false;
}

bool IsDeadSlot(Int_t slot) {
  if (slot == 92 ||
      slot == 156 ||
      slot == 157 ||
      slot == 163 ||
      slot == 207) return true;
  else return false;
}

Double_t ConvertTotToPe(Int_t time_over_threshold) {
  return 20.635 * std::pow(time_over_threshold, 0.3654) + 10.347;
}

/**
 * Add NINJA tracker data as B2SpillSummary
 * @param output_spill_summary WAGASCI/BabyMIND spill summary where the hits added
 * @param array[NUMBER_OF_SLOTS_IN_TRACKER] NINJA tracker raw data
 */
void AddNinjaAsHitSummary(B2SpillSummary &output_spill_summary,
			  Int_t lt[NUMBER_OF_SLOTS_IN_TRACKER], Int_t tt[NUMBER_OF_SLOTS_IN_TRACKER], Float_t pe[NUMBER_OF_SLOTS_IN_TRACKER],
			  Int_t view[NUMBER_OF_SLOTS_IN_TRACKER], Int_t pln[NUMBER_OF_SLOTS_IN_TRACKER], Int_t ch[NUMBER_OF_SLOTS_IN_TRACKER],
			  Int_t subrunid) {

  for (int slot = 0; slot < NUMBER_OF_SLOTS_IN_TRACKER; slot++) {
    if (IsUnusedSlot(slot)) continue;
    if (IsDeadSlot(slot)) continue;
    bool is_first_hit = false;
    bool is_after_hit = false;
    int bunch_difference = 0;

    // slot 150 treatment? TODO
    if (IsNoisySlot(slot) && pe[slot] > 3.5) is_first_hit = true;
    else if (pe[slot] > 2.5) is_first_hit = true;
    else {
      for (int i_bunch_difference = 1; i_bunch_difference <= 6; i_bunch_difference++) {
	if ( ( ( subrunid == 0 &&
		 std::fabs(lt[slot] - LEADTIME_PEAK_2019[i_bunch_difference - 1]) < LEADTIME_HALF_WIDTH ) ||
	       ( subrunid == 1 &&
		 std::fabs(lt[slot] - LEADTIME_PEAK_2020[i_bunch_difference - 1]) < LEADTIME_HALF_WIDTH ) ) &&
	     lt[slot] - tt[slot] > TOT_MIN &&
	     lt[slot] - tt[slot] < TOT_MAX ) {
	  is_after_hit = true;
	  bunch_difference = i_bunch_difference;
	  break;
	}
      }
    }

    if ( !is_first_hit && !is_after_hit ) continue;
    
    auto &output_hit_summary = output_spill_summary.AddHit();
    output_hit_summary.SetBunch(bunch_difference); // not bunch but bunch difference from the first hits.
    output_hit_summary.SetDetector(B2Detector::kNinja);
    output_hit_summary.SetPlaneGrid(B2GridPlane::kPlaneScintillator);
    output_hit_summary.SetPlane(pln[slot]);
    output_hit_summary.SetTrueTimeNs(lt[slot]);

    B2View ninja_view = (view[slot] == B2View::kTopView) ?
      B2View::kTopView : B2View::kSideView;
    output_hit_summary.SetView(ninja_view);
    TVector3 pos, err;	
    B2Dimension::GetPosNinjaTracker(ninja_view, (UInt_t) pln[slot], (UInt_t) ch[slot], pos);
    B2Dimension::GetErrorNinja(ninja_view, err);
    output_hit_summary.SetScintillatorPosition(B2Position(pos, err));
    B2ScintillatorType scintillator_type = (view[slot] == B2View::kTopView) ?
      B2ScintillatorType::kVertical : B2ScintillatorType::kHorizontal;
    output_hit_summary.SetScintillatorType(scintillator_type);

    std::vector<B2Readout> readouts;
    readouts.push_back(detector_to_single_readout(B2Detector::kNinja, scintillator_type, pln[slot]));
    for (const auto &readout : readouts) {
      output_hit_summary.SetSlot(readout, ch[slot]);
      if (is_after_hit) pe[slot] = ConvertTotToPe(lt[slot] - tt[slot]); // for fancy event display
      output_hit_summary.SetHighGainPeu(readout, pe[slot]);
      output_hit_summary.SetTimeNs(readout, lt[slot]-tt[slot]); // Time over Threshold stored
    } 
  }
  
}

// main function
int main(int argc, char *argv[]) {

  logging::core::get()->set_filter
    (
     logging::trivial::severity >= logging::trivial::info
     );
  
  BOOST_LOG_TRIVIAL(info) << "==========NINJA Hit Converter Start==========";

  if (argc != 5) {
    BOOST_LOG_TRIVIAL(error) << "Usage : "<< argv[0]
			     << " <input wagasci file path> <input ninja file path> <output file path> <subrun 0(2019)/1(2020)>";
    std::exit(1);
  }

  try {
    B2Reader reader(argv[1]);
    TFile *ntfile = new TFile(argv[2], "read");
    B2Writer writer(argv[3], reader);
    Int_t subrunid = atoi(argv[4]);
    BOOST_LOG_TRIVIAL(info) << "-----Settings Summary-----";
    BOOST_LOG_TRIVIAL(info) << "Reader  file : " << argv[1];
    BOOST_LOG_TRIVIAL(info) << "Tracker file : " << argv[2];
    BOOST_LOG_TRIVIAL(info) << "Writer  file : " << argv[3];
    BOOST_LOG_TRIVIAL(info) << "Subrun id    : " << argv[4];

    // Tracker file settings
    BOOST_LOG_TRIVIAL(info) << "Tracker file tree setting...";
    TTree *nttree = (TTree*)ntfile->Get("tree");

    Int_t adc[NUMBER_OF_SLOTS_IN_TRACKER], tt[NUMBER_OF_SLOTS_IN_TRACKER], lt[NUMBER_OF_SLOTS_IN_TRACKER];
    UInt_t unixtime[NUMBER_OF_SLOTS_IN_TRACKER];
    Float_t pe[NUMBER_OF_SLOTS_IN_TRACKER];
    Int_t view[NUMBER_OF_SLOTS_IN_TRACKER], pln[NUMBER_OF_SLOTS_IN_TRACKER], ch[NUMBER_OF_SLOTS_IN_TRACKER];
    
    nttree->SetBranchAddress("ADC", adc);
    nttree->SetBranchAddress("LEADTIME", lt);
    nttree->SetBranchAddress("TRAILTIME", tt);
    nttree->SetBranchAddress("UNIXTIME", unixtime);
    nttree->SetBranchAddress("PE", pe);
    nttree->SetBranchAddress("VIEW", view);
    nttree->SetBranchAddress("PLN", pln);
    nttree->SetBranchAddress("CH", ch);
    BOOST_LOG_TRIVIAL(info) << "done!";

    Int_t ntentry = 0; // Tracker file entry
    const Int_t ntentry_max = nttree->GetEntries();

    while (reader.ReadNextSpill() > 0 && ntentry < ntentry_max) {

      auto &output_spill_summary = writer.GetSpillSummary();
      auto &beam_summary = output_spill_summary.GetBeamSummary();
      // Get corresponding NINJA entry
      Int_t ntentry_tmp = GetNinjaSpill(output_spill_summary, nttree, ntentry, unixtime);
      
      // Add NINJA entry as B2HitSummary
      if (ntentry_tmp > 0) {
	nttree->GetEntry(ntentry_tmp);
	ntentry = ntentry_tmp + 1;
	AddNinjaAsHitSummary(output_spill_summary, lt, tt, pe, view, pln, ch, subrunid);
	beam_summary.EnableDetector(B2Detector::kNinja);
      }
      else {
	beam_summary.DisableDetector(B2Detector::kNinja);
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
