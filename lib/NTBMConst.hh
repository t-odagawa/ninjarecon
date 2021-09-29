#ifndef NTBMCONST_HH
#define NTBMCONST_HH

#include <B2Const.hh>

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
///> Gap width of NINJA tracker scintillator bars
static const double NINJA_TRACKER_GAP = 25. * 4 / 3 - 24.;
///> Thickness of NINJA tracker scintillator bar
static const double NINJA_TRACKER_SCI_THICK = 3.;

///> Offset from the 4th layer in NINJA tracker in X/Y direction
static const double NINJA_TRACKER_OFFSET_XY[NUMBER_OF_PLANES] = {25. * 5 / 6, 25. / 6, 25. * 2 / 3, 0.};
///> Offset from the 1st layer in NINJA tracker in Z direction
static const double NINJA_TRACKER_OFFSET_Z[NUMBER_OF_PLANES] = {0., 6., 9., 15.};
///> Difference of NINJA tracker absolute position and reconstruction coordinate
static const double NINJA_SCI_DIFF = 448.; // mm
///> Baby MIND vertical scintillator overlap
static const double BM_VERTICAL_SCINTI_OVERLAP = 35.5; // mm
// Nominal position of the 2nd layer in the BM coordinate
// Offset + Scin_Mod_position.txt(2) + 5 cm (?) + 1.5 scintillators
static const double BM_SECOND_LAYER_POS = BABYMIND_OFFSET_Z + 183. + 50. + 15.; // mm
// Scintillator position used in the WAGASCI is not well corrected for our usage
// plane more than 2nd should be much corrected.
static const double BM_SCI_CORRECTION = -31.5; // mm
static const double TEMPORAL_ALLOWANCE[2] = {250., 400.}; // mm

///> Photoelectron threshold for the NINJA tracker
static const double PE_THRESHOLD = 2.5;
///> Time over threshold lower limit for the NINJA tracker
static const double TOT_MIN = 5.; // ns
///> Time over threshold upper limit for the NINJA tracker
static const double TOT_MAX = 150.; // ns
///> Leadtime peak values for the NINJA tracker for Physics run a-1
static const double LEADTIME_PEAK_2019[6] = {3500., 2916., 2333., 1750., 1168., 588.}; // ns
///> Leadtime peak values for the NINJA tracker for Physics run a-2
static const double LEADTIME_PEAK_2020[6] = {3503., 2924., 2345., 1765., 1182., 602.}; // ns
///> Leadtime 3sigma
static const double LEADTIME_HALF_WIDTH = 130.; // ns

#endif
