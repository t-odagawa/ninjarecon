// system includes
#include <string>

// boost includes
#include <boost/log/trivial.hpp>

// root includes

// B2 includes
#include "B2Reader.hh"
#include "B2Writer.hh"
#include "B2HitSummary.hh"

int main(int argc, char *argv[]) {
  BOOST_LOG_TRIVIAL(info) << "==========NINJA Hit Converter Start==========";

  if (argc != 4) {
    BOOST_LOG_TRIVIAL(error) << "Usage : "<< argv[0]
			     << " <input wagasci file path> <input ninja file path> <output file path>";
    std::exit(1);
  }

  try {
    B2Reader reader(argv[1]);
    B2Writer writer(argv[3]);
    BOOST_LOG_TRIVIAL(info) << "-----Settings Summary-----";

    while (reader.ReadNextSpill() > 0) {

      auto &input_spill_summary = reader.GetSpillSummary();

      auto &output_spill_summary = writer.GetSpillSummary();

      input_spill_summary.CloneHits(output_spill_summary);
      input_spill_summary.CloneBSD(output_spill_summary);

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
