#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>

#include <TFile.h>
#include <TTree.h>

#include <B2Enum.hh>

#include <NTBMConst.hh>
#include <NTBMSummary.hh>

namespace logging = boost::log;

int main(int argc, char* argv[]) {

  logging::core::get()->set_filter
    (
     logging::trivial::severity >= logging::trivial::info
     );

  if ( argc < 4 ) {
    BOOST_LOG_TRIVIAL(error) << "Usage : " << argv[0]
			     << " <NTBM file name> <number of input texts> <text file names> <output file name> <position allowance>";
    std::exit(1);
  }

  std::string filename_match = argv[1];
  TFile *file_match = new TFile(filename_match.c_str(), "read");
  TTree *tree = (TTree*)file_match->Get("tree");
  NTBMSummary *ntbm = nullptr;
  tree->SetBranchAddress("NTBMSummary", &ntbm);

  int num_text_file = std::atoi(argv[2]);

  if ( argc != 3 + num_text_file + 2 ) {
    BOOST_LOG_TRIVIAL(error) << "Number of input text files are inconsistent";
    std::exit(1);
  }

  std::vector<std::string > input_vec;
  for ( int itext = 0; itext < num_text_file; itext++ ) {
    input_vec.push_back(argv[3 + itext]);
  }

  std::ofstream ofs(argv[3 + num_text_file]);

  const double allowance = std::atof(argv[3 + num_text_file + 1]);

  std::vector<double> x_vec;
  std::vector<double> y_vec;
  std::vector<double> x_nt_vec;
  std::vector<double> y_nt_vec;
  std::vector<double> x_diff_vec;
  std::vector<double> y_diff_vec;
  std::vector<bool> x_find_vec;
  std::vector<bool> y_find_vec;
  x_vec.reserve(num_text_file);
  y_vec.reserve(num_text_file);
  x_nt_vec.reserve(num_text_file);
  y_nt_vec.reserve(num_text_file);
  x_diff_vec.reserve(num_text_file);
  y_diff_vec.reserve(num_text_file);
  x_find_vec.reserve(num_text_file);
  y_find_vec.reserve(num_text_file);

  int spot, start, stop;

  for ( auto filename : input_vec ) {

    std::ifstream ifs(filename);

    int event_disp_id;
    double bt_ax, bt_ay, bt_x, bt_y;
    int pl, rawid;
    int year, month, day;
    double ex_x, ex_y;
    
    while( ifs >> event_disp_id >> bt_ax >> bt_ay >> bt_x >> bt_y
	   >> pl >> rawid
	   >> spot >> start >> stop 
	   >> year >> month >> day >> year >> month >> day
	   >> ex_x >> ex_y ) {
      x_vec.push_back(ex_x);
      y_vec.push_back(ex_y);
      x_nt_vec.push_back(-9999.);
      y_nt_vec.push_back(-9999.);
      x_diff_vec.push_back(-9999.);
      y_diff_vec.push_back(-9999.);
      x_find_vec.push_back(false);
      y_find_vec.push_back(false);
      BOOST_LOG_TRIVIAL(info) << "X : " << ex_x << ", "
			      << "Y : " << ex_y;
    }

  }

  for ( int ientry = 0; ientry < tree->GetEntries(); ientry++ ) {
    
    tree->GetEntry(ientry);
    BOOST_LOG_TRIVIAL(debug) << "Entry : " << ientry;

    if ( ntbm->GetTimestamp() < start ) continue;
    if ( ntbm->GetTimestamp() > stop ) break;

    if ( ntbm->GetNumberOfNinjaClusters() < 2 ) continue;
   
    for ( int bunch_diff = 0; bunch_diff < 7; bunch_diff++ ) {

      for ( int i = 0; i < num_text_file; i++ ) {
	x_find_vec.at(i) = false;
	y_find_vec.at(i) = false;
      }

      for ( int icluster = 0; icluster < ntbm->GetNumberOfNinjaClusters(); icluster++ ) {
	if ( ntbm->GetBunchDifference(icluster) != bunch_diff ) continue; // only in the same bunch difference
	if ( ntbm->GetBabyMindTrackId(icluster) != -1 ) continue; // only 2d cluster

	if ( ntbm->GetNumberOfHits(icluster, B2View::kTopView) > 0 ) { // top view (x)
	  double x = ntbm->GetNinjaPosition(icluster, B2View::kTopView);
	  for ( int ix = 0; ix < x_vec.size(); ix++ ) {
	    if ( std::fabs(x - x_vec.at(ix)) < allowance ) {
	      x_find_vec.at(ix) = true;
	      x_nt_vec.at(ix) = x;
	      x_diff_vec.at(ix) = x - x_vec.at(ix);
	    }
	  }
	}
	else if ( ntbm->GetNumberOfHits(icluster, B2View::kSideView) > 0 ) { // side view (y)
	  double y = ntbm->GetNinjaPosition(icluster, B2View::kSideView);
	  for ( int iy = 0; iy < y_vec.size(); iy++ ) {
	    if ( std::fabs(y - y_vec.at(iy)) < allowance ) {
	      y_find_vec.at(iy) = true;
	      y_nt_vec.at(iy) = y;
	      y_diff_vec.at(iy) = y - y_vec.at(iy);
	    }
	  }
	}
      } // icluster

      bool find_event_flag = true;
      for ( int i = 0; i < num_text_file; i++ ) {
	find_event_flag = find_event_flag && x_find_vec.at(i);
	find_event_flag = find_event_flag && y_find_vec.at(i);
      }

      if ( find_event_flag ) {
	BOOST_LOG_TRIVIAL(info) << "Found event candidate : " << (int)ntbm->GetTimestamp()
				<< ", " << ntbm->GetEntryInDailyFile();
	for ( int i = 0; i < num_text_file; i++ ) {
	  BOOST_LOG_TRIVIAL(info) << "X : " << x_vec.at(i) << ", " << x_nt_vec.at(i) << ", Y : " << y_vec.at(i) << ", " << y_nt_vec.at(i);
	}
	ofs << (int)ntbm->GetTimestamp() << " " << ntbm->GetEntryInDailyFile() << std::endl;
	break;
      }

    } // bunch diff

  } // entry

  ofs.close();

  std::exit(0);

}
