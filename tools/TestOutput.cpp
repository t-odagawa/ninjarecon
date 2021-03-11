// system includes
#include <vector>
#include <iostream>

// boost include
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

// root includes
#include <TFile.h>
#include <TTree.h>

#include "NTBMSummary.hh"

namespace logging = boost::log;

int main (int argc, char *argv[]) {

  logging::core::get()->set_filter
    (
     logging::trivial::severity >= logging::trivial::info
     );

  if (argc != 4) {
    BOOST_LOG_TRIVIAL(error) << "Usage : " << argv[0]
			     << "<input NTBM file path> <start entry> <end entry>";
    std::exit(1);
  }

  Int_t start = atoi(argv[2]);
  Int_t end = atoi(argv[3]);

  if (start < 0) {
    BOOST_LOG_TRIVIAL(error) << "Start entry shold be positive : " << argv[2];
    std::exit(1);
  }

  try {

    TFile *file = new TFile(argv[1], "read");
    TTree *tree = (TTree*)file->Get("tree");

    NTBMSummary *ntbm = new NTBMSummary();
    tree->SetBranchAddress("NTBMSummary", &ntbm);

    if (end >= tree->GetEntries()) {
      BOOST_LOG_TRIVIAL(error) << "End entry shold be less than # of entries in the file " << argv[1] << " : "
			       << tree->GetEntries() << " : " <<argv[3];
      std::exit(1);
    }

    for (int ientry = start; ientry < end; ientry++) {
      tree->GetEntry(ientry);
      BOOST_LOG_TRIVIAL(info) << "Timestamp : " << (int)ntbm->GetTimestamp();
      BOOST_LOG_TRIVIAL(info) << *ntbm;
    }

  } catch (const std::runtime_error &error) {
    BOOST_LOG_TRIVIAL(fatal) << "Runtime error : " << error.what();
    std::exit(1);
  } catch (const std::invalid_argument &error) {
    BOOST_LOG_TRIVIAL(fatal) << "Invalide argument error : " << error.what();
    std::exit(1);
  }

  std::exit(0);

}
