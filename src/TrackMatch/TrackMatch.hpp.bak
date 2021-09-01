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
 * Create NINJA tracker clusters
 * @param ninja_hits NINJA Hit summary vector
 * @param ninja_clusters NTBMSummary for the spill (x/y separated and only NINJA tracker data)
 */
void CreateNinjaCluster(std::vector<const B2HitSummary*> ninja_hits, NTBMSummary* ninja_clusters);

/**
 * Original function instead of B2HitsSet::HasDetector() as reconstructed track summary
 * does not have detector information on its own
 * @param track reconstructed B2TrackSummary object
 * @param det detector id
 * @return true if the object has hits inside the detector
 */
bool MyHasDetector(const B2TrackSummary *track, B2Detector det);

/**
 * Get number of hits in each view of i-th plane of B2TrackSummary
 * @param track reconstructed B2TrackSummary object
 * @param view view
 * @param plane plane
 * @return number of hits
 */
int GetBabyMindPlaneHits(const B2TrackSummary *track, int view, int plane);

/**
 * Get the averaged x(y)/z position vector in Baby MIND in Baby MIND coordinate
 * @param track reconstructed B2TrackSummary object
 * @param view view in interest
 * @param plane plane in interest
 * @return at(0) means x or y averaged position, at(1) does z
 */
std::vector<double> GetBabyMindPlanePosition(const B2TrackSummary *track, int view, int plane);

/**
 * Get positional error vector in Baby MIND
 * @param track reconstructed B2TrackSummary object
 * @param view view in interest
 * @param plane plane in interest
 * @return at(0) means x or y positional error, at(1) does z
 */
std::vector<double> GetBabyMindPlanePositionError(const B2TrackSummary *track, int view, int plane);

/**
 * Fit Baby MIND
 * @param track reconstructed B2TrackSummary object
 * @param c TCanvas for drawing
 * @param entry entry for canvas title
 * @param draw true if draw result pdf is required
 * @param view view
 * @return at(0) means intercept and at(1) does slope in Baby MIND coordinate
 */
std::vector<double> FitBabyMind(const B2TrackSummary *track, TCanvas *c, int entry, bool draw, int view);

/**
 * Get Baby MIND initial direction
 * @param track reconstructed B2TrackSummary object
 * @param c TCanvas for drawing
 * @param entry entry for canvas title
 * @return at(0) means y and at(1) does x directions
 */
std::vector<double> GetBabyMindInitialDirection(const B2TrackSummary *track, TCanvas *c, int entry);

/**
 * Get Baby MIND initial position
 * @param track reconstructed B2TrackSummary object
 * @param view view
 * @param c TCanvas for drawing
 * @param entry entry for canvas title
 * @param draw true if draw result pdf is required
 * @return at(0) means x or y and at(1) does z
 */
std::vector<double> GetBabyMindInitialPosition(const B2TrackSummary *track, int view, TCanvas *c, int entry, bool draw);

/**
 * Calculate hit expected position on the NINJA tracker position
 * @param track reconstructed B2TrackSumamry object
 * @return at(0) means y and at(1) means x
 */
std::vector<double> CalculateExpectedPosition(const B2TrackSummary *track);

/**
 * Check if the reconstructed track summary expected to have hits
 * in the NINJA tracker
 * @param track B2TrackSummary object of Reconstructed Baby MIND track
 * @return true if the track expected to have hits else false
 */
bool NinjaHitExpected(const B2TrackSummary *track, TCanvas *c, int entry);

/**
 * Track matching between Baby MIND and NINJA tracker using x/y separated NTBMSummary
 * and Baby MIND B2TrackSummary
 * @param track B2TrackSummary object of Baby MIND track
 * @param baby_mind_track_id Baby MIND track id (incremented from 0 NINJA internally)
 * @param ntbm_in NTBMSummary object created in the CreateNinjaCluster function
 */
bool MatchBabyMindTrack(const B2TrackSummary *track, int baby_mind_track_id, NTBMSummary *ntbm_in, int &bunch_diff);

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
 * Get boolean if range [min, max] does not make hit in a scintillator bar
 * @param min minimum value of the range
 * @param max maximum value of the range
 * @param view pln slot detector ids of the scintillator bar
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
 * @param ntbm_summary NTBMSummary object
 * @param cluster cluster
 * @param view view
 * @param plane plane
 * @return number of hits in the plane
 */
int GetNinjaTrackerPlaneHits(NTBMSummary *ntbm_summary, int cluster, int view, int plane);

/**
 * Use Baby MIND information, reconstruct tangent for matching
 * between NINJA tracker and Emulsion shifter
 * @param ntbmsummary NTBMSummary object after the MatchBabyMindTrack function
 */
void ReconstructNinjaTangent(NTBMSummary* ntbm_summary);

/**
 * Use Baby MIND information, reconstruct position for matching
 * between NINJA tracker and Emulsion shifter
 * @param ntbmsummary NTBMSummary object after the MatchBabyMindTrack function
 */
void ReconstructNinjaPosition(NTBMSummary* ntbm_summary);

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
 */
void TransferBeamInfo(const B2SpillSummary& spill_summary, NTBMSummary* ntbm_summary);

/**
 * Original function instead of B2HitsSet::GetBunch() as reconstructed track summary
 * does not have bunch information on its own
 * @param track reconstructed B2TrackSummary object
 * @return bunch
 */
int MyGetBunch(const B2TrackSummary *track);

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
