// system includes
#include <string>
#include <vector>

// booost includes
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

// root includes

// B2 includes
#include "B2Reader.hh"
#include "B2Writer.hh"
#include "B2HitSummary.hh"

namespace logging = boost::log;

int main(int argc, char *argv[]) {

  logging::core::get()->set_filter
    (
     logging::trivial::severity >= logging::trivial::info
     );

  BOOST_LOG_TRIVIAL(info) << "==========NINJA Track Matching Start==========";

  if (argc != 2) {
    BOOST_LOG_TRIVIAL(error) << "Usage : " << argv[0]
			     << " <input B2 file path> <output B2 file path>";
    std::exit(1);
  }

  try {
    B2Reader reader(argv[1]);
    B2Writer writer(argv[2]);

    while (reader.ReadNextSpill() > 0) {
      auto &input_spill_summary = reader.GetSpillSummary();

      // Create NINJA tracker hit cluster
      auto it_hit = input_spill_summary.BeginHit();
      std::vector<const B2HitSummary* > ninja_hits;
      while (const auto *ninja_hit = it_hit.Next()) {
	if(ninja_hit->GetDetectorId() == B2Detector::kNinja)
	  ninja_hits.push_back(ninja_hit);
      }
      // CreateNinjaCluster(ninja_hits);

      // Extrapolate BabyMIND cluster to the NINJA position
      // and get the best cluster to match each NINJA cluster
      // auto it_cluster = input_spill_summary.BeginCluter();
      

      // Create output file only includes NINJA related items
      // writer.Fill();

    }
    
  } catch (const std::runtime_error &error) {
    BOOST_LOG_TRIVIAL(fatal) << "Runtime error : " << error.what();
  } catch (const std::invalid_argument &error) {
    BOOST_LOG_TRIVIAL(fatal) << "Invalid argument error : " << error.what();
    std::exit(1);
  }
  
  BOOST_LOG_TRIVIAL(info) << "==========NINJA Track Matching Finish==========";
  std::exit(0);

}