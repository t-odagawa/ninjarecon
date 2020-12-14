#ifndef NTBMCONST_HH
#define NTBMCONST_HH

///> Default non initialized value
static const int NTBM_NON_INITIALIZED_VALUE = -2;

///> Number of J-PARC neutrino beam bunches
static const std::size_t NUMBER_OF_BUNCHES = 8;

///> Number of views
static const std::size_t NUMBER_OF_VIEWS = 2;
///> Number of planes in the NINJA tracker
static const std::size_t NUMBER_OF_PLANES = 4;
///> Number of slots in the NINJA tracker
static const std::size_t NUMBER_OF_SLOTS = 31;

///> Photoelectron threshold for the NINJA tracker
static const double PE_THRESHOLD = 2.5;
///> Time over threshold threshold for the NINJA tracker
static const double TOT_THRESHOLD = 0.;

#endif
