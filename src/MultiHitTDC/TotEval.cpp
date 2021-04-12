// boost includes
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

// root includes
#include <TF1.h>
#include <TH2D.h>
#include <TCanvas.h>

// B2 includes
#include <B2Reader.hh>
#include <B2Enum.hh>
#include <B2Dimension.hh>
#include <B2SpillSummary.hh>
#include <B2HitSummary.hh>

namespace logging = boost::log;

int main (int argc, char*argv[]) {

  logging::core::get()->set_filter
    (
     logging::trivial::severity >= logging::trivial::info
     );

  BOOST_LOG_TRIVIAL(info) << "==========ToT Evaluation Start==========";

  if (argc != 3) {
    BOOST_LOG_TRIVIAL(error) << "Usage : " << argv[0]
			     << " <input B2 file path> <output pdf file path>";
    std::exit(1);
  } 

  try {

    B2Reader reader(argv[1]);
    Double_t peu, tot;
    TCanvas *c = new TCanvas("c", "c");
    TH2D *hist = new TH2D("hist", "PE vs ToT;PE;ToT [ns]",200,0,100,100,0,200);

    while (reader.ReadNextSpill() > 0) {
      auto &spill_summary = reader.GetSpillSummary();

      // Get NINJA hits
      std::vector<const B2HitSummary*> hits;
      auto it_hit = spill_summary.BeginHit();
      while (const auto *hit = it_hit.Next()) {
	if (hit->GetDetectorId() != B2Detector::kNinja) continue;
	hits.push_back(hit);
      }
      BOOST_LOG_TRIVIAL(debug) << "# of NINJA hits : " << hits.size();

      // Fill histrogram
      if (hits.size() > 0) {
	for (auto hit : hits) {
	  std::vector<B2Readout> readouts;
	  readouts.push_back(hit->GetSingleReadout());
	  for (const auto &readout : readouts) {
	    peu = hit->GetHighGainPeu().GetValue(readout);
	    tot = hit->GetTimeNs().GetValue(readout);
	    hist->Fill(peu, tot);
	    BOOST_LOG_TRIVIAL(debug) << "peu : " << peu
				     << " tot : " << tot;
	  } // readout
	} // hit
      } // fi
    } // while

    c->cd();
    hist->Draw("colz");
    c->Print(argv[2], "pdf");

    delete c;

  } catch (const std::runtime_error &error) {
    BOOST_LOG_TRIVIAL(fatal) << "Runtime error : " << error.what();
    std::exit(1);
  } catch (const std::invalid_argument &error) {
    BOOST_LOG_TRIVIAL(fatal) << "Invalid argument error : " << error.what();
    std::exit(1);
  }

  BOOST_LOG_TRIVIAL(info) << "==========ToT EValuation Finish==========";
  std::exit(0);

}
