#ifndef NTBMCONST_HH
#define NTBMCONST_HH

///> Default non initialized value
static const int NTBM_NON_INITIALIZED_VALUE = -2;

///> Number of J-PARC neutrino beam bunches
static const int NUMBER_OF_BUNCHES = 8;

///> Number of views
static const int NUMBER_OF_VIEWS = 2;
///> Number of planes in the NINJA tracker
static const int NUMBER_OF_PLANES = 4;
///> Number of slots in the NINJA tracker
static const int NUMBER_OF_SLOTS_IN_PLANE = 31;
///> Number of slots in all EASIROC modules (includes two unused channels)
static const int NUMBER_OF_SLOTS_IN_TRACKER = 250;

///> Width of NINJA tracker scintillator bar
static const double NINJA_TRACKER_SCI_WIDTH = 24.;

///> Photoelectron threshold for the NINJA tracker
static const double PE_THRESHOLD = 2.5;
///> Time over threshold threshold for the NINJA tracker
static const double TOT_THRESHOLD = 0.;

#endif
