// system includes
#include <string>

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

int main(int argc, char *argv[]) {

  logging::core::get()->set_filter
    (
     logging::trivial::severity >= logging::trivial::info
     );

  BOOST_LOG_TRIVIAL(info) << "==========NINJA Emulsion Wrapper Start==========";

  if (argc != 3) {
    BOOST_LOG_TRIVIAL(error) << "Usage : " << argv[0]
			     << "<input b2 file path> <output wrapped file path>";
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
      
      // NINJA tracker data extraction from NTBMSummary
      // nttree->GetEntry(ntentry);
      ntentry++;

      
    }
    
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
