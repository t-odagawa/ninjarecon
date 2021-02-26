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
 * Get incidenct track position on the upstream surface of Baby MIND
 * calculation is changed depending on the track type
 * @param track B2TrackSummary object
 * @return track position on the upstream surface of Baby MIND
 */
TVector3 CalculateTrackInitialPosition(const B2TrackSummary *track);

int GetBabyMindPlaneHits(const B2TrackSummary *track, int view, int plane);

/**
 * Get the averaged x(y)/z position vector in Baby MIND
 * @param track reconstructed B2TrackSummary object
 * @param view view in interest
 * @param plane plane in interest
 * @return at(0) means x or y averaged position, at(1) does z
 */

std::vector<double> GetBabyMindPlanePosition(const B2TrackSummary *track, int view, int plane);

std::vector<double> GetBabyMindPlanePositionError(const B2TrackSummary *track, int view, int plane);

std::vector<double> FitBabyMindTopView(const B2TrackSummary *track, TCanvas *c, int entry, bool draw);

std::vector<double> GetBabyMindInitialDirection(const B2TrackSummary *track, int view, TCanvas *c, int entry, bool draw);

std::vector<double> GetBabyMindInitialPosition(const B2TrackSummary *track, int view, TCanvas *c, int entry, bool draw);

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
 * @param ntbm_in NTBMSummary object created in the CreateNinjaCluster function
 * @param ntbm_out NTBMSummary object for 3D track matching
 */
void MatchBabyMindTrack(const B2TrackSummary *track, NTBMSummary *ntbm_in, NTBMSummary *ntbm_out);

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
 * Use Baby MIND information, reconstruct tangent for matching
 * between NINJA tracker and Emulsion shifter
 * @param ntbmsummary NTBMSummary object after the MatchBabyMindTrack function
 */
void ReconstructNinjaTangent(NTBMSummary* ntbmsummary);

/**
 * Use Baby MIND information, reconstruct position for matching
 * between NINJA tracker and Emulsion shifter
 * @param ntbmsummary NTBMSummary object after the MatchBabyMindTrack function
 */
void ReconstructNinjaPosition(NTBMSummary* ntbmsummary);

void SetTruePositionAngle(const B2SpillSummary& spill_summary, NTBMSummary* ntbmsummary);

void TransferBeamInfo(const B2SpillSummary& spill_summary, NTBMSummary* ntbmsummary);
#endif
