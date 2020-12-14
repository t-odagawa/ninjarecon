// system includes
#include <string>
#include <vector>

// boost includes
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

// B2 includes
#include "B2Reader.hh"
#include "B2HitSummary.hh"
#include "B2BeamSummary.hh"

// program includes
#include "NTBMSummary.hh"

namespace logging = boost::log;

/**
 * Transfer beam information from B2BeamSummary to NTBMSummary
 * @param ntbm_summary destination object of the transfer
 * @param spill_summary source object of the transfer
 */
void TransferBeamInformation(NTBMSummary &ntbm_summary, const B2SpillSummary &spill_summary) {
  auto &beam_summary = spill_summary.GetBeamSummary();
  ntbm_summary.SetSpillPot(beam_summary.GetSpillPot());
  for(std::size_t bunch = 0; bunch < 8; bunch++)
    ntbm_summary.SetBunchPot(bunch, beam_summary.GetBunchPot(bunch));
  ntbm_summary.SetBsdSpillNumber(beam_summary.GetBsdSpillNumber());
  ntbm_summary.SetTimestamp(beam_summary.GetTimestamp());
  ntbm_summary.SetBsdGoodSpillFlag(beam_summary.GetBsdGoodSpillFlag());
  ntbm_summary.SetWagasciGoodSpillFlag(beam_summary.GetWagasciGoodSpillFlag());
}

/**
 * Transfer Baby MIND information from B2TrackSummary to NTBMSummary
 * @param ntbm_summary destination object of the transfer
 * @param spill_summary source object of the transfer
 */
void TransferBabyMindInformation(NTBMSummary &ntbm_summary, const B2SpillSummary &spill_summary) {
  //auto &it_track = spill_summary.BeginReconTrack();
  auto it_track = spill_summary.BeginTrueTrack();
  std::vector<const B2TrackSummary* > tracks;
  while (const auto *track = it_track.Next()) {

  }

}

/**
 * Transfer NINJA tracker information from input file to output file
 * @param destination destination object of the transfer
 * @param source source object of the transfer
 */
void TransferNinjaTrackerInformation(NTBMSummary &destination, NTBMSummary &source) {
  destination.SetNumberOfNinjaClusters(source.GetNumberOfNinjaClusters());
  for (int cluster = 0; cluster < destination.GetNumberOfNinjaClusters(); cluster++) {
    destination.SetBabyMindTrackId(cluster, source.GetBabyMindTrackId(cluster));
    destination.SetNumberOfHits(cluster, source.GetNumberOfHits(cluster));
    // Call number of hits function before plane/slot/pe!!
    destination.SetPlane(cluster, source.GetPlane(cluster));
    destination.SetSlot(cluster, source.GetSlot(cluster));
    destination.SetPe(cluster, source.GetPe(cluster));
    destination.SetBunchDifference(cluster, source.GetBunchDifference(cluster));
    destination.SetNinjaPosition(cluster, source.GetNinjaPosition(cluster));
    destination.SetNinjaPositionError(cluster, source.GetNinjaPositionError(cluster));
    destination.SetNinjaTangent(cluster, source.GetNinjaTangent(cluster));
    destination.SetNinjaTangentError(cluster, source.GetNinjaTangentError(cluster));
  }
}

int main(int argc, char *argv[]) {

  logging::core::get()->set_filter
    (
     logging::trivial::severity >= logging::trivial::info
     );

  BOOST_LOG_TRIVIAL(info) << "==========NINJA Emulsion Wrapper Start==========";

  if (argc != 3) {
    BOOST_LOG_TRIVIAL(error) << "Usage : " << argv[0]
			     << " <input b2 file path> <output wrapped file path>";
    std::exit(1);
  }

  try {

    B2Reader reader(argv[1]);
    TFile *ntfile = new TFile(argv[1], "read");
    TFile *outfile = new TFile(argv[2], "recreate");
    BOOST_LOG_TRIVIAL(info) << "-----Settings Summary-----";
    BOOST_LOG_TRIVIAL(info) << "Reader file : " << argv[1];
    BOOST_LOG_TRIVIAL(info) << "Output file : " << argv[2];

    // NINJA tracker branch setting
    // TTree *nttree = (TTree*)ntfile->Get("nttree");
    // NTBMSummary* nt_ntbm_summary;
    // nttree->SetBranchAddress("ntbmsummary", &nt_ntbm_summary);

    TTree *outtree = new TTree("tree", "tree");
    NTBMSummary output_ntbm_summary;
    outtree->Branch("ntbmsummary", &output_ntbm_summary);
    
    Int_t ntentry = 0;
    
    while (reader.ReadNextSpill() > 0) {
      // Beam and Baby MIND data extraction from B2SpillSummary
      auto &input_spill_summary = reader.GetSpillSummary();
      TransferBeamInformation(output_ntbm_summary, input_spill_summary);
      TransferBabyMindInformation(output_ntbm_summary, input_spill_summary);
      output_ntbm_summary.SetNumberOfTracks(1); // Test
      
      // NINJA tracker data extraction from NTBMSummary
      // nttree->GetEntry(ntentry);
      // TransferNinjaTrackerInformation(output_ntbm_summary, nt_ntbm_summary);
      ntentry++;

      outtree->Fill();

    }

    outtree->Write();
    outfile->Close();
    
  } catch (const std::runtime_error &error) {
    BOOST_LOG_TRIVIAL(fatal) << "Runtime error : " << error.what();
    std::exit(1);
  } catch (const std::invalid_argument &error) {
    BOOST_LOG_TRIVIAL(fatal) << "Invalid argument error : " << error.what();
    std::exit(1);
  }

  BOOST_LOG_TRIVIAL(info) << "==========NINJA Emulsion Wrapper Finish==========";
  std::exit(0);
  
}
