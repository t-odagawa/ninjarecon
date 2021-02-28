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
///> Distance between NINJA tracker and Baby MIND layer in interest
static const double NINJA_BABYMIND_DISTANCE[NUMBER_OF_VIEWS] = {810., 1040.};
///> Baby MIND vertical scintillator overlap
static const double BM_VERTICAL_SCINTI_OVERLAP = 35.5; // mm
///> Baby MIND second layer position in Baby MIND coordinate TODO
static const double BM_SECOND_LAYER_POS = -1767.; // mm

///> Photoelectron threshold for the NINJA tracker
static const double PE_THRESHOLD = 2.5;
///> Time over threshold threshold for the NINJA tracker
static const double TOT_THRESHOLD = 0.;

#endif
