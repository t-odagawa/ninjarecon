#ifndef NINJARECON_TRACKMATCH_HPP
#define NINJARECON_TRACKMATCH_HPP

#include <vector>

#include <TCanvas.h>

#include "B2SpillSummary.hh"
#include "B2HitSummary.hh"
#include "B2BeamSummary.hh"
#include "B2TrackSummary.hh"
#include "NTBMSummary.hh"

/**
 * Comparator for B2HitSummary vector sort
 * @param lhs left hand side object
 * @param rhs right hand side object
 * @return true if the objects should not be swapped
 */
bool CompareNinjaHits(const B2HitSummary* lhs, const B2HitSummary* rhs);

/**
 * Comparator for B2HitSummary vector sort in one Baby MIND B2TrackSummary
 * @param lhs left hand side object
 * @param rhs right hand side object
 * @return true if the objects should not be swapped
 */
bool CompareBabyMindHitInOneTrack(const B2HitSummary* lhs, const B2HitSummary *rhs);

/**
 * Create NINJA tracker clusters
 * @param ninja_hits NINJA Hit summary vector
 * @param ninja_clusters NTBMSummary for the spill (x/y separated and only NINJA tracker data)
 */
void CreateNinjaCluster(std::vector<const B2HitSummary*> ninja_hits, NTBMSummary* ninja_clusters);


/**
 * Get position and error for one Baby MIND plane
 * @param position position list of Baby MIND hits
 * @param view view
 * @return position and error for one Baby MIND plane
 */
std::vector<std::vector<double> > CalcMergedOnePlanePositionAndError(std::vector<std::vector<double> > position, int view);

/**
 * Get position and error for Baby MIND planes
 * @param hits vector of Baby MIND B2HitSummary objects
 * @param datatype MC or real data
 * @return Baby MIND position and errors
 */
std::vector<std::vector<std::vector<std::vector<double> > > > GenerateMergedPositionAndErrors(std::vector<const B2HitSummary* > hits, int datatype);

/**
 * Fit Baby MIND
 * @param track reconstructed B2TrackSummary object
 * @param datatype MC or real data
 * @return at(0) means intercept and at(1) does slope in Baby MIND coordinate
 */
std::vector<std::vector<double> > FitBabyMind(const B2TrackSummary *track, int datatype);

/**
 * Get Baby MIND initial direction
 * @param track reconstructed B2TrackSummary object
 * @return at(0) means y and at(1) does x directions 
 * at(2) means y and at(3) does x positions
 */
std::vector<double> GetBabyMindInitialDirectionAndPosition(const B2TrackSummary *track);

/**
 * Calculate hit expected position on the NINJA tracker position
 * @param ntbm NTBMSummary object of the spill in interest
 * @param itrack Baby MIND track id (incremented from 0 NINJA internally)
 * @param z_shift Difference of z distance from nominal
 * @return at(0) means y and at(1) means x
 */
std::vector<double> CalculateExpectedPosition(NTBMSummary *ntbm, int itrack, double z_shift);

/**
 * Check if the Baby MIND reconstructed track expected to have hits
 * in the NINJA tracker
 * @param ntbm NTBMSummary object of the spill in interest
 * @param itrack Baby MIND track id (incremented from 0 NINJA internally)
 * @param z_shift Difference of z distance from nominal
 * @return true if the track expected to have hits else false
 */
bool NinjaHitExpected(NTBMSummary *ntbm, int itrack, double z_shift);

/**
 * Track matching between Baby MIND and NINJA tracker using x/y separated NTBMSummary
 * and Baby MIND B2TrackSummary
 * @param ntbm NTBMSummary object created in the CreateNinjaCluster function
 * @param itrack Baby MIND track id (incremented from 0 NINJA internally)
 * @param bunch_diff Bunch difference b/w NINJA and Baby MIND
 * @param z_shift Difference of z distance from nominal
 * @return true if the track is matched to NINJA cluster
 */
bool MatchBabyMindTrack(NTBMSummary *ntbm, int itrack, int &bunch_diff, double z_shift);

/**
 * Get boolean if the value is in range [min, max]
 * @param pos position to be evaluated
 * @param min minimum value of the range
 * @param max maximum value of the range
 */
bool IsInRange(double pos, double min, double max);

/**
 * Get boolean if range [min, max] makes hit in a scintillator bar
 * @param min minimum value of the range
 * @param max maximum value of the range
 * @param view pln slot detector ids of the scintillator bar
 */
bool IsMakeHit(double min, double max, int view, int plane, int slot);

/**
 * Get boolean if range [min, max] is between i and i+1-th scintillator bars
 * @param min minimum value of the range
 * @param max maximum value of the range
 * @param view pln slot detector ids of the scintillator bar including slot = -1
 */
bool IsInGap(double min, double max, int view, int plane, int slot);

/**
 * Get the minimum value of the position where the line intercepts
 * @param pos track start position
 * @param tangent track tangent
 * @param iplane plane id of the track starting scintillator bar
 * @param jplane plane id of the intercept evaluated scintillator bar
 * @param vertex vertex position of the track starting point
 */
double GetTrackAreaMin(double pos, double tangent, int iplane, int jplane, int vertex);

/**
 * Get the maximum value of the position where the line intercepts
 * @param pos track start position
 * @param tangent track tangent
 * @param iplane plane id of the track starting scintillator bar
 * @param jplane plane id of the intercept evaluated scintillator bar
 * @param vertex vertex position of the track starting point
 */
double GetTrackAreMax(double pos, double tangent, int iplane, int jplane, int vertex);

/**
 * Get boolean if the normal track analysis is possible
 * @param condition four element array of boolean
 */
bool IsGoodTrack(bool *condition);

/**
 * Get number of hits in NINJA tracker one plane
 * @param ntbm NTBMSummary object
 * @param cluster cluster
 * @return number of hits in the plane
 */
std::vector<std::vector<int> > GetNinjaTrackerNumberOfPlaneHits(NTBMSummary* ntbm, int cluster);

/**
 * Use Baby MIND information, reconstruct tangent for matching
 * between NINJA tracker and Emulsion shifter
 * @param ntbm NTBMSummary object after the MatchBabyMindTrack function
 */
void ReconstructNinjaTangent(NTBMSummary* ntbm);

/**
 * Use Baby MIND information, reconstruct position for matching
 * between NINJA tracker and Emulsion shifter
 * @param ntbm NTBMSummary object after the MatchBabyMindTrack function
 */
void ReconstructNinjaPosition(NTBMSummary* ntbm);

/**
 * Set TSS info as true position/angle information to evaluate
 * tracker performance with MC
 * @param spill_summary B2SpillSummary object
 * @param ntbm_summary NTBMSummary object
 */
void SetTruePositionAngle(const B2SpillSummary& spill_summary, NTBMSummary* ntbm_summary);

/**
 * Transfer Beam information from B2BeamSummary to NTBMSummary
 * @param spill_summary B2SpillSummary object
 * @param ntbm_summary NTBMSummary object
 * @param datatype MC or real data
 */
void TransferBeamInfo(const B2SpillSummary& spill_summary, NTBMSummary* ntbm_summary, int datatype);

/**
 * Transfer Baby MIND track info from B2TrackSummary to NTBMSummary
 * @param spill_summary B2SpillSummary object
 * @param ntbm_summary NTBMSummary object
 */
void TransferBabyMindTrackInfo(const B2SpillSummary& spill_summary, NTBMSummary ntbm_summary);

/**
 * Transfer MC normalization info from B2EventSummary to NTBMSummary
 * @param spill_summary B2SpillSummary object
 * @param ntbmsummary NTBMSummary object
 */
void TransferMCInfo(const B2SpillSummary& spill_summary, NTBMSummary ntbm_summary);

#endif
