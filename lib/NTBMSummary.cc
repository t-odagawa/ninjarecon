#include "NTBMSummary.hh"
#include "NTBMConst.hh"

#include <ostream>

NTBMSummary::NTBMSummary() {
  NTBMSummary::Clear("C");
}

void NTBMSummary::Clear(Option_t *option) {
  spill_pot_ = -1.;
  for ( double &i : bunch_pot_ )
    i = -1.;
  bsd_spill_number_ = 0;
  timestamp_ = 0.;
  bsd_good_spill_flag_ = -1;
  wagasci_good_spill_flag_ = -1;
  for (int i = 0; i < 8; i++) 
    detector_flags_[i] = -1;
  number_of_tracks_ = 0;
  ninja_track_type_.clear();
  momentum_type_.clear();
  momentum_.clear();
  baby_mind_position_.clear();
  baby_mind_tangent_.clear();
  baby_mind_maximum_plane_.clear();
  charge_.clear();
  direction_.clear();
  bunch_.clear();
  number_of_ninja_clusters_ = 0;
  baby_mind_track_id_.clear();
  number_of_hits_.clear();
  plane_.clear();
  slot_.clear();
  pe_.clear();
  bunch_difference_.clear();
  ninja_position_.clear();
  ninja_tangent_.clear();
  normalization_ = 1.;
  total_cross_section_ = 1.;
  number_of_true_particles_.clear();
  true_particle_id_.clear();
  true_position_.clear();
  true_tangent_.clear();
  TObject::Clear(option);
}

std::ostream &operator<<(std::ostream &os, const NTBMSummary &obj) {
  os << "Total POT of this spill = " << obj.spill_pot_ << "\n"
     << "POT for each bunch = ";
  for (int i = 0; i < NUMBER_OF_BUNCHES; i++) {
    os << i + 1 << " : " << obj.bunch_pot_[i];
    if (i != NUMBER_OF_BUNCHES - 1) os << ", ";
  }
  os << "\n"
     << "Timestamp = " << obj.timestamp_ << "\n"
     << "BSD good spill flag (good : 1, bad : 0) = " << obj.bsd_good_spill_flag_ << "\n"
     << "WAGASCI good spill flag (good : 1, bad : 0) = " << obj.wagasci_good_spill_flag_ << "\n"
     << "Detector flags (good : 1, bad : 0) = ";
  for (int i = 0; i < 8; i++) {
    os << i+1 << " : " << obj.detector_flags_[i];
    if (i != 7) os << ", ";
  }
  os << "\n" 
     << "Number of Baby MIND tracks = " << obj.number_of_tracks_ << "\n"
     << "Baby MIND track type (ECC cand. : 0, Sand cand. : 1) = ";
  for (int i = 0; i < obj.number_of_tracks_; i++) {
    os << i + 1 << " : " << obj.ninja_track_type_.at(i);
    if(i != obj.number_of_tracks_ - 1) os << ", ";
  }
  os << "\n"
     << "Momentum measurement type (Baby MIND range : 0, curvature : 1) = ";
  for (int i = 0; i < obj.number_of_tracks_; i++) {
    os << i + 1 << " : " << obj.momentum_type_.at(i);
    if(i != obj.number_of_tracks_ - 1) os << ", ";
  }
  os << "\n"
     << "Absolute momentum = ";
  for (int i = 0; i < obj.number_of_tracks_; i++) {
    os << i + 1 << " : " << obj.momentum_.at(i) << " +/- "
       << obj.momentum_error_.at(i);
    if(i != obj.number_of_tracks_ - 1) os << ", ";
  }
  os << "\n"
     << "Baby MIND initial position = ";
  for (int i = 0; i < obj.number_of_tracks_; i++) {
    os << i + 1 << " : ( "
       << obj.baby_mind_position_.at(i).at(0) << " +/- "
       << obj.baby_mind_position_error_.at(i).at(0) << ", "
       << obj.baby_mind_position_.at(i).at(1) << "+/-"
       << obj.baby_mind_position_error_.at(i).at(1) << " )\n";
  }
  os << "\n"
     << "Baby MIND initial tangent = ";
  for (int i = 0; i < obj.number_of_tracks_; i++) {
    os << i + 1 << " : ( "
       << obj.baby_mind_tangent_.at(i).at(0) << " +/- "
       << obj.baby_mind_tangent_error_.at(i).at(0) << ", "
       << obj.baby_mind_tangent_.at(i).at(1) << " +/- "
       << obj.baby_mind_tangent_error_.at(i).at(1) <<" )\n";
  }
  os << "\n"
     << "Baby MIND maximum plane = ";
  for (int i = 0; i < obj.number_of_tracks_; i++) {
    os << i + 1 << " : " << obj.baby_mind_maximum_plane_.at(i);
    if(i != obj.number_of_tracks_ - 1) os << ", ";
  }  
  os << "\n"
     << "Track length total = ";
  for (int i = 0; i < obj.number_of_tracks_; i++) {
    os << i + 1 << " : " << obj.track_length_total_.at(i);
    if(i != obj.number_of_tracks_ - 1) os << ", ";
  }
  os << "\n"
     << "Charge = ";
  for (int i = 0; i < obj.number_of_tracks_; i++) {
    os << i + 1 << " : " << obj.charge_.at(i);
    if(i != obj.number_of_tracks_ - 1) os << ", ";
  }
  os << "\n"
     << "Direction = ";
  for (int i = 0; i < obj.number_of_tracks_; i++) {
    os << i + 1 << " : " << obj.direction_.at(i);
    if(i != obj.number_of_tracks_ - 1) os << ", ";
  }
  os << "\n"
     << "Bunch = ";
  for (int i = 0; i < obj.number_of_tracks_; i++) {
    os << i + 1 << " : " << obj.bunch_.at(i);
    if(i != obj.number_of_tracks_ - 1) os << ", ";
  }
  os << "\n"
     << "Number of NINJA clusters = " << obj.number_of_ninja_clusters_ << "\n"
     << "Corresponding Baby MIND track ID = ";
  for (int i = 0; i < obj.number_of_ninja_clusters_; i++) {
    os << i + 1 << " : " << obj.baby_mind_track_id_.at(i);
    if(i != obj.number_of_ninja_clusters_ - 1) os << ", ";
  }
  os << "\n"
     << "Number of hits in each cluster = ";
  for (int i = 0; i < obj.number_of_ninja_clusters_; i++) {
    os << i + 1 << " : ( "
       << obj.number_of_hits_.at(i).at(0) << ", "
       << obj.number_of_hits_.at(i).at(1) << " )\n";
  }
  os << "\n"
     << "Plane list = ";
  for (int i = 0; i < obj.number_of_ninja_clusters_; i++) {
    os << i + 1 << " : Y : ( ";
    for (int j = 0; j < obj.number_of_hits_.at(i).at(0); j++) {
      os << obj.plane_.at(i).at(0).at(j);
      if (j != obj.number_of_hits_.at(i).at(0) - 1) os << ", ";
    }
    os << " ), X : ( ";
    for (int j = 0; j < obj.number_of_hits_.at(i).at(1); j++) {
      os << obj.plane_.at(i).at(1).at(j);
      if (j != obj.number_of_hits_.at(i).at(1) - 1) os << ", ";
    }
    os << " )\n";
  }
  os << "\n"
     << "Slot list = ";
  for (int i = 0; i < obj.number_of_ninja_clusters_; i++) {
    os << i + 1 << " : Y : ( ";
    for (int j = 0; j < obj.number_of_hits_.at(i).at(0); j++) {
      os << obj.slot_.at(i).at(0).at(j);
      if (j != obj.number_of_hits_.at(i).at(0) - 1) os << ", ";
    }
    os << " ), X : ( ";
    for (int j = 0; j < obj.number_of_hits_.at(i).at(1); j++) {
      os << obj.slot_.at(i).at(1).at(j);
      if (j != obj.number_of_hits_.at(i).at(1) - 1) os << ", ";
    }
    os << " )\n";   
  }
  os << "\n"
     << "Photoelectrons or Time over Threshold list = ";
  for (int i = 0; i < obj.number_of_ninja_clusters_; i++) {
    os << i + 1 << " : Y : ( ";
    for (int j = 0; j < obj.number_of_hits_.at(i).at(0); j++) {
      os << obj.pe_.at(i).at(0).at(j);
      if (j != obj.number_of_hits_.at(i).at(0) - 1) os << ", ";
    }
    os << " ), X : ( ";
    for (int j = 0; j < obj.number_of_hits_.at(i).at(1); j++) {
      os << obj.pe_.at(i).at(1).at(j);
      if (j != obj.number_of_hits_.at(i).at(1) - 1) os << ", ";
    }
    os << " )\n";
  }
  os << "\n"
     << "Bunch difference from the first matched bunch = ";
  for (int i = 0; i < obj.number_of_ninja_clusters_; i++) {
    os << i + 1 << " : " << obj.bunch_difference_.at(i);
    if (i != obj.number_of_ninja_clusters_ - 1) os << ", ";
  }
  os << "\n"
     << "NINJA tracker reconstructed position = ";
  for (int i = 0; i < obj.number_of_ninja_clusters_; i++) {
    os << i + 1 << " : ( "
       << obj.ninja_position_.at(i).at(0) << " +/- "
       << obj.ninja_position_error_.at(i).at(0) << ", "
       << obj.ninja_position_.at(i).at(1) << " +/- "
       << obj.ninja_position_error_.at(i).at(1) << " )\n";
  }
  os << "\n"
     << "NINJA tracker reconstructed tangent = ";
  for (int i = 0; i < obj.number_of_ninja_clusters_; i++) {
    os << i + 1 << " : ( "
       << obj.ninja_tangent_.at(i).at(0) << " +/- "
       << obj.ninja_tangent_error_.at(i).at(0) << ", "
       << obj.ninja_tangent_.at(i).at(1) << " +/- "
       << obj.ninja_tangent_error_.at(i).at(1) << " )\n";
  }
  os << "\n"
     << "Normalization factor = " << obj.normalization_ << "\n"
     << "Total cross section = " << obj.total_cross_section_ << "\n";
  
  return os;
}

void NTBMSummary::SetSpillPot(double spill_pot) {
  spill_pot_ = spill_pot;
}

double NTBMSummary::GetSpillPot() const {
  return spill_pot_;
}

void NTBMSummary::SetBunchPot(int bunch, double bunch_pot) {
  bunch_pot_[bunch] = bunch_pot;
}

double NTBMSummary::GetBunchPot(int bunch) const {
  if(bunch >= 0 && bunch < NUMBER_OF_BUNCHES)
    return bunch_pot_[bunch];
  else
    throw std::out_of_range("Bunch number out of range");
}

void NTBMSummary::SetBsdSpillNumber(int bsd_spill_number) {
  bsd_spill_number_ = bsd_spill_number;
}

int NTBMSummary::GetBsdSpillNumber() const {
  return bsd_spill_number_;
}

void NTBMSummary::SetTimestamp(double timestamp) {
  timestamp_ = timestamp;
}

double NTBMSummary::GetTimestamp() const {
  return timestamp_;
}

void NTBMSummary::SetBsdGoodSpillFlag(int bsd_good_spill_flag) {
  bsd_good_spill_flag_ = bsd_good_spill_flag;
}

int NTBMSummary::GetBsdGoodSpillFlag() const {
  return bsd_good_spill_flag_;
}

void NTBMSummary::SetWagasciGoodSpillFlag(int wagasci_good_spill_flag) {
  wagasci_good_spill_flag_ = wagasci_good_spill_flag;
}

int NTBMSummary::GetWagasciGoodSpillFlag() const {
  return wagasci_good_spill_flag_;
}

void NTBMSummary::SetDetectorFlags(int detector, int detector_flag) {
  if (detector >= 0 && detector < 8)
    detector_flags_[detector] = detector_flag;
  else 
    throw std::out_of_range("Detector id out of range");
}

int NTBMSummary::GetDetectorFlags(int detector) const {
  if (detector >= 0 && detector < 8)
    return detector_flags_[detector];
  else 
    throw std::out_of_range("Detector id out of range");
}

void NTBMSummary::SetNumberOfTracks(int number_of_tracks) {
  number_of_tracks_ = number_of_tracks;
  // Always set number of tracks before set other elements
  // related to Baby MIND
  ninja_track_type_.resize(number_of_tracks_);
  momentum_type_.resize(number_of_tracks_);
  momentum_.resize(number_of_tracks_);
  momentum_error_.resize(number_of_tracks_);
  baby_mind_position_.resize(number_of_tracks_);
  baby_mind_position_error_.resize(number_of_tracks_);
  baby_mind_tangent_.resize(number_of_tracks_);
  baby_mind_tangent_error_.resize(number_of_tracks_);
  for(int i = 0; i < number_of_tracks_; i++) {
    baby_mind_position_.at(i).resize(2);
    baby_mind_position_error_.at(i).resize(2);
    baby_mind_tangent_.at(i).resize(2);
    baby_mind_tangent_error_.at(i).resize(2);
  }
  baby_mind_maximum_plane_.resize(number_of_tracks_);
  track_length_total_.resize(number_of_tracks_);
  charge_.resize(number_of_tracks_);
  direction_.resize(number_of_tracks_);
  bunch_.resize(number_of_tracks_);
}

int NTBMSummary::GetNumberOfTracks() const {
  return number_of_tracks_;
}

void NTBMSummary::SetNinjaTrackType(int track, int ninja_track_type) {
  ninja_track_type_.at(track) = ninja_track_type;
}

int NTBMSummary::GetNinjaTrackType(int track) const {
  if (track > number_of_tracks_)
    throw std::out_of_range("Number of track out of range");
  return ninja_track_type_.at(track);
}

void NTBMSummary::SetMomentumType(int track, int momentum_type) {
  momentum_type_.at(track) = momentum_type;
}

int NTBMSummary::GetMomentumType(int track) const {
  if (track >= number_of_tracks_)
    throw std::out_of_range("Number of track out of range");
  return momentum_type_.at(track);
}

void NTBMSummary::SetMomentum(int track, double momentum) {
  momentum_.at(track) = momentum;
}

double NTBMSummary::GetMomentum(int track) const {
  if (track >= number_of_tracks_)
    throw std::out_of_range("Number of track out of range");
  return momentum_.at(track);
}

void NTBMSummary::SetMomentumError(int track, double momentum_error) {
  momentum_error_.at(track) = momentum_error;
}

double NTBMSummary::GetMomentumError(int track) const {
  if (track >= number_of_tracks_)
    throw std::out_of_range("Number of track out of range");
  return momentum_error_.at(track);
}

void NTBMSummary::SetBabyMindPosition(int track, int view, double baby_mind_position) {
  if (view >= NUMBER_OF_VIEWS)
    throw std::out_of_range("View out of range");
  baby_mind_position_.at(track).at(view) = baby_mind_position;
}

void NTBMSummary::SetBabyMindPosition(int track, std::vector<double> baby_mind_position) {
  for (std::size_t view = 0; view < NUMBER_OF_VIEWS; view++)
    SetBabyMindPosition(track, view, baby_mind_position.at(view));
}

std::vector<double> NTBMSummary::GetBabyMindPosition(int track) const {
  if (track >= number_of_tracks_)
    throw std::out_of_range("Number of track out of range");
  return baby_mind_position_.at(track);
}

double NTBMSummary::GetBabyMindPosition(int track, int view) const {
  if (view >= NUMBER_OF_VIEWS)
    throw std::out_of_range("View out of range");
  return GetBabyMindPosition(track).at(view);
}

void NTBMSummary::SetBabyMindPositionError(int track, int view, double baby_mind_position_error) {
  if (view >= NUMBER_OF_VIEWS)
    throw std::out_of_range("View out of range");
  baby_mind_position_error_.at(track).at(view) = baby_mind_position_error;
}

void NTBMSummary::SetBabyMindPositionError(int track, std::vector<double> baby_mind_position_error) {
  for (std::size_t view = 0; view < NUMBER_OF_VIEWS; view++)
    SetBabyMindPositionError(track, view, baby_mind_position_error.at(view));
}

std::vector<double> NTBMSummary::GetBabyMindPositionError(int track) const {
  if (track >= number_of_tracks_)
    throw std::out_of_range("Number of track out of range");
  return baby_mind_position_error_.at(track);
}

double NTBMSummary::GetBabyMindPositionError(int track, int view) const {
  if (view >= NUMBER_OF_VIEWS)
    throw std::out_of_range("View out of range");
  return GetBabyMindPositionError(track).at(view);
}

void NTBMSummary::SetBabyMindTangent(int track, int view, double baby_mind_tangent) {
  if (view >= NUMBER_OF_VIEWS)
    throw std::out_of_range("View out of range");
  baby_mind_tangent_.at(track).at(view) = baby_mind_tangent;
}

void NTBMSummary::SetBabyMindTangent(int track, std::vector<double> baby_mind_tangent) {
  for(std::size_t view = 0; view < NUMBER_OF_VIEWS; view++)
    SetBabyMindTangent(track, view, baby_mind_tangent.at(view));
}

std::vector<double> NTBMSummary::GetBabyMindTangent(int track) const {
  if (track >= number_of_tracks_)
    throw std::out_of_range("Number of track out of range");
  return baby_mind_tangent_.at(track);
}

double NTBMSummary::GetBabyMindTangent(int track, int view) const {
  if (view > NUMBER_OF_VIEWS)
    throw std::out_of_range("View our of range");
  return GetBabyMindTangent(track).at(view);
}

void NTBMSummary::SetBabyMindTangentError(int track, int view, double baby_mind_tangent_error) {
  if (view >= NUMBER_OF_VIEWS)
    throw std::out_of_range("View out of range");
  baby_mind_tangent_error_.at(track).at(view) = baby_mind_tangent_error;
}

void NTBMSummary::SetBabyMindTangentError(int track, std::vector<double> baby_mind_tangent_error) {
  for(std::size_t view = 0; view < NUMBER_OF_VIEWS; view++)
    SetBabyMindTangentError(track, view, baby_mind_tangent_error.at(view));
}

std::vector<double> NTBMSummary::GetBabyMindTangentError(int track) const {
  if (track >= number_of_tracks_)
    throw std::out_of_range("Number of track out of range");
  return baby_mind_tangent_error_.at(track);
}

double NTBMSummary::GetBabyMindTangentError(int track, int view) const {
  if (view > NUMBER_OF_VIEWS)
    throw std::out_of_range("View our of range");
  return GetBabyMindTangentError(track).at(view);
}

void NTBMSummary::SetBabyMindMaximumPlane(int track, int baby_mind_maximum_plane) {
  baby_mind_maximum_plane_.at(track) = baby_mind_maximum_plane;
}

int NTBMSummary::GetBabyMindMaximumPlane(int track) const {
  if (track >= number_of_tracks_)
    throw std::out_of_range("Number of track out of range");
  return baby_mind_maximum_plane_.at(track);
}

void NTBMSummary::SetTrackLengthTotal(int track, double track_length_total) {
  track_length_total_.at(track) = track_length_total;
}

double NTBMSummary::GetTrackLengthTotal(int track) const {
  if (track >= number_of_tracks_)
    throw std::out_of_range("Number of track out of range");
  return track_length_total_.at(track);
}

void NTBMSummary::SetCharge(int track, int charge) {
  charge_.at(track) = charge;
}

int NTBMSummary::GetCharge(int track) const {
  if (track >= number_of_tracks_)
    throw std::out_of_range("Number of track out of range");
  return charge_.at(track);
}

void NTBMSummary::SetDirection(int track, int direction) {
  direction_.at(track) = direction;
}

int NTBMSummary::GetDirection(int track) const {
  if (track >= number_of_tracks_)
    throw std::out_of_range("Number of track out of range");
  return direction_.at(track);
}

void NTBMSummary::SetBunch(int track, int bunch) {
  bunch_.at(track) = bunch;
}

int NTBMSummary::GetBunch(int track) const {
  if (track >= number_of_tracks_)
    throw std::out_of_range("Number of track out of range");
  return bunch_.at(track);
}

void NTBMSummary::SetNumberOfNinjaClusters(int number_of_ninja_clusters) {
  number_of_ninja_clusters_ = number_of_ninja_clusters;
  // Always set number of clusters before set other elements
  // related to NINJA cluster
  baby_mind_track_id_.resize(number_of_ninja_clusters_);
  number_of_hits_.resize(number_of_ninja_clusters_);
  plane_.resize(number_of_ninja_clusters_);
  slot_.resize(number_of_ninja_clusters_);
  pe_.resize(number_of_ninja_clusters_);
  bunch_difference_.resize(number_of_ninja_clusters_);
  ninja_position_.resize(number_of_ninja_clusters_);
  ninja_position_error_.resize(number_of_ninja_clusters_);
  ninja_tangent_.resize(number_of_ninja_clusters);
  ninja_tangent_error_.resize(number_of_ninja_clusters_);
  for(int i = 0; i < number_of_ninja_clusters_; i++) {
    number_of_hits_.at(i).resize(2);
    plane_.at(i).resize(2);
    slot_.at(i).resize(2);
    pe_.at(i).resize(2);
    ninja_position_.at(i).resize(2);
    ninja_position_error_.at(i).resize(2);
    ninja_tangent_.at(i).resize(2);
    ninja_tangent_error_.at(i).resize(2);
  }
}

int NTBMSummary::GetNumberOfNinjaClusters() const {
  return number_of_ninja_clusters_;
}

void NTBMSummary::SetBabyMindTrackId(int cluster, int baby_mind_track_id) {
  baby_mind_track_id_.at(cluster) = baby_mind_track_id;
}

int NTBMSummary::GetBabyMindTrackId(int cluster) const {
  if (cluster >= number_of_ninja_clusters_)
    throw std::out_of_range("Number of cluster out of range");
  return baby_mind_track_id_.at(cluster);
}

void NTBMSummary::SetNumberOfHits(int cluster, int view, int number_of_hits) {
  if (view >= NUMBER_OF_VIEWS)
    throw std::out_of_range("View out of range");
  number_of_hits_.at(cluster).at(view) = number_of_hits;
  // Always set number of hits before set other elements
  // related to NINJA tracker hits
  plane_.at(cluster).at(view).resize(number_of_hits_.at(cluster).at(view));
  slot_.at(cluster).at(view).resize(number_of_hits_.at(cluster).at(view));
  pe_.at(cluster).at(view).resize(number_of_hits_.at(cluster).at(view));
}

void NTBMSummary::SetNumberOfHits(int cluster, std::vector<int> number_of_hits) {
  for(std::size_t view = 0; view < NUMBER_OF_VIEWS; view++)
    SetNumberOfHits(cluster, view, number_of_hits.at(view));
}

std::vector<int> NTBMSummary::GetNumberOfHits(int cluster) const {
  if (cluster >= number_of_ninja_clusters_)
    throw std::out_of_range("Number of cluster out of range");
  return number_of_hits_.at(cluster);
}

int NTBMSummary::GetNumberOfHits(int cluster, int view) const {
  if(view >= NUMBER_OF_VIEWS)
    throw std::out_of_range("View out of range");
  return GetNumberOfHits(cluster).at(view);
}

void NTBMSummary::SetPlane(int cluster, int view, int hit, int plane) {
  if(view >= NUMBER_OF_VIEWS)
    throw std::out_of_range("View out of range");
  if (plane >= NUMBER_OF_PLANES)
    throw std::out_of_range("Plane out of range");
  plane_.at(cluster).at(view).at(hit) = plane;
}

void NTBMSummary::SetPlane(int cluster, int view, std::vector<int> plane) {
  for (std::size_t hit = 0; hit < plane.size(); hit++)
    SetPlane(cluster, view, hit, plane.at(hit));
}

void NTBMSummary::SetPlane(int cluster, std::vector<std::vector<int>> plane) {
  for(std::size_t view = 0; view < NUMBER_OF_VIEWS; view++)
    SetPlane(cluster, view, plane.at(view));
}

std::vector<std::vector<int>> NTBMSummary::GetPlane(int cluster) const {
  if (cluster >= number_of_ninja_clusters_)
    throw std::out_of_range("Number of cluster out of range");
  return plane_.at(cluster);
}

std::vector<int> NTBMSummary::GetPlane(int cluster, int view) const {
  if (view >= NUMBER_OF_VIEWS)
    throw std::out_of_range("View our of range");
  return GetPlane(cluster).at(view);
}

int NTBMSummary::GetPlane(int cluster, int view, int hit) const {
  if (hit >= number_of_hits_.at(cluster).at(view))
    throw std::out_of_range("Number of hit out of range");
  return GetPlane(cluster, view).at(hit);
}

void NTBMSummary::SetSlot(int cluster, int view, int hit, int slot) {
  if(view >= NUMBER_OF_VIEWS)
    throw std::out_of_range("View out of range");
  if(slot >= NUMBER_OF_SLOTS_IN_PLANE)
    throw std::out_of_range("Slot out of range");
  slot_.at(cluster).at(view).at(hit) = slot;
}

void NTBMSummary::SetSlot(int cluster, int view, std::vector<int> slot) {
  for (std::size_t hit = 0; hit < slot.size(); hit++)
    SetSlot(cluster, view, hit, slot.at(hit));
}

void NTBMSummary::SetSlot(int cluster, std::vector<std::vector<int>> slot) {
  for(std::size_t view = 0; view < NUMBER_OF_VIEWS; view++)
    SetSlot(cluster, view, slot.at(view));
}

std::vector<std::vector<int>> NTBMSummary::GetSlot(int cluster) const {
  if (cluster >= number_of_ninja_clusters_)
    throw std::out_of_range("Number of cluster out of range");
  return slot_.at(cluster);
}

std::vector<int> NTBMSummary::GetSlot(int cluster, int view) const {
  if (view >= NUMBER_OF_VIEWS)
    throw std::out_of_range("View out of range");
  return GetSlot(cluster).at(view);
}

int NTBMSummary::GetSlot(int cluster, int view, int hit) const {
  if (hit >= number_of_hits_.at(cluster).at(view))
    throw std::out_of_range("Number of hit out of range");
  return GetSlot(cluster, view).at(hit);
}

void NTBMSummary::SetPe(int cluster, int view, int hit, double pe) {
  if(view >= NUMBER_OF_VIEWS)
    throw std::out_of_range("View out of range");
  pe_.at(cluster).at(view).at(hit) = pe;
}

void NTBMSummary::SetPe(int cluster, int view, std::vector<double> pe) {
  for (std::size_t hit = 0; hit < pe.size(); hit++)
    SetPe(cluster, view, hit, pe.at(hit));
}

void NTBMSummary::SetPe(int cluster, std::vector<std::vector<double>> pe) {
  for(std::size_t view = 0; view < NUMBER_OF_VIEWS; view++)
    SetPe(cluster, view, pe.at(view));
}

std::vector<std::vector<double>> NTBMSummary::GetPe(int cluster) const {
  if (cluster >= number_of_ninja_clusters_)
    throw std::out_of_range("Number of cluster out of range");
  return pe_.at(cluster);
}

std::vector<double> NTBMSummary::GetPe(int cluster, int view) const {
  if (view >= NUMBER_OF_VIEWS)
    throw std::out_of_range("View our of range");
  return GetPe(cluster).at(view);
}

double NTBMSummary::GetPe(int cluster, int view, int hit) const {
  if (hit >= number_of_hits_.at(cluster).at(view))
    throw std::out_of_range("Number of hit out of range");
  return GetPe(cluster, view).at(hit);
}

void NTBMSummary::SetBunchDifference(int cluster, int bunch_difference) {
  bunch_difference_.at(cluster) = bunch_difference;
}

int NTBMSummary::GetBunchDifference(int cluster) const {
  if (cluster > number_of_ninja_clusters_)
    throw std::out_of_range("Number of cluster out of range");
  return bunch_difference_.at(cluster);
}

void NTBMSummary::SetNinjaPosition(int cluster, int view, double ninja_position) {
  if (view >= NUMBER_OF_VIEWS)
    throw std::out_of_range("View out of range");
  ninja_position_.at(cluster).at(view) = ninja_position;
}

void NTBMSummary::SetNinjaPosition(int cluster, std::vector<double> ninja_position) {
  for(std::size_t view = 0; view < NUMBER_OF_VIEWS; view++)
    SetNinjaPosition(cluster, view, ninja_position.at(view));
}

std::vector<double> NTBMSummary::GetNinjaPosition(int cluster) const {
  if (cluster >= number_of_ninja_clusters_)
    throw std::out_of_range("Number of cluster out of range");
  return ninja_position_.at(cluster);
}

double NTBMSummary::GetNinjaPosition(int cluster, int view) const {
  if (view >= NUMBER_OF_VIEWS)
    throw std::out_of_range("View out of range");
  return GetNinjaPosition(cluster).at(view);
}

void NTBMSummary::SetNinjaPositionError(int cluster, int view, double ninja_position_error) {
  if (view >= NUMBER_OF_VIEWS)
    throw std::out_of_range("View out of range");
  ninja_position_error_.at(cluster).at(view) = ninja_position_error;
}

void NTBMSummary::SetNinjaPositionError(int cluster, std::vector<double> ninja_position_error) {
  for(std::size_t view = 0; view < NUMBER_OF_VIEWS; view++)
    SetNinjaPositionError(cluster, view, ninja_position_error.at(view));
}

std::vector<double> NTBMSummary::GetNinjaPositionError(int cluster) const {
  if (cluster >= number_of_ninja_clusters_)
    throw std::out_of_range("Number of cluster out of range");
  return ninja_position_error_.at(cluster);
}

double NTBMSummary::GetNinjaPositionError(int cluster, int view) const {
  if (view >= NUMBER_OF_VIEWS)
    throw std::out_of_range("View out of range");
  return GetNinjaPositionError(cluster).at(view);
}

void NTBMSummary::SetNinjaTangent(int cluster, int view, double ninja_tangent) {
  if (view >= NUMBER_OF_VIEWS)
    throw std::out_of_range("View out of range");
  ninja_tangent_.at(cluster).at(view) = ninja_tangent;
}

void NTBMSummary::SetNinjaTangent(int cluster, std::vector<double> ninja_tangent) {
  for(std::size_t view = 0; view < NUMBER_OF_VIEWS; view++)
    SetNinjaTangent(cluster, view, ninja_tangent.at(view));
}

std::vector<double> NTBMSummary::GetNinjaTangent(int cluster) const {
  if (cluster >= number_of_ninja_clusters_)
    throw std::out_of_range("Number of cluster out of range");
  return ninja_tangent_.at(cluster);
}

double NTBMSummary::GetNinjaTangent(int cluster, int view) const {
  if (view >= NUMBER_OF_VIEWS)
    throw std::out_of_range("View out of range");
  return GetNinjaTangent(cluster).at(view);
}

void NTBMSummary::SetNinjaTangentError(int cluster, int view, double ninja_tangent_error) {
  if (view >= NUMBER_OF_VIEWS)
    throw std::out_of_range("View out of range");
  ninja_tangent_error_.at(cluster).at(view) = ninja_tangent_error;
}

void NTBMSummary::SetNinjaTangentError(int cluster, std::vector<double> ninja_tangent_error) {
  for(std::size_t view = 0; view < NUMBER_OF_VIEWS; view++)
    SetNinjaTangentError(cluster, view, ninja_tangent_error.at(view));
}

std::vector<double> NTBMSummary::GetNinjaTangentError(int cluster) const {
  if (cluster >= number_of_ninja_clusters_)
    throw std::out_of_range("Number of cluster out of range");
  return ninja_tangent_error_.at(cluster);
}

double NTBMSummary::GetNinjaTangentError(int cluster, int view) const {
  if (view >= NUMBER_OF_VIEWS)
    throw std::out_of_range("View out of range");
  return GetNinjaTangentError(cluster).at(view);
}

void NTBMSummary::SetNumberOfTrueParticles(int cluster, int number_of_true_particles) {
  number_of_true_particles_.at(cluster) = number_of_true_particles;
  for (int itrue = 0; itrue < number_of_true_particles_.at(cluster); itrue++) {
    true_particle_id_.at(cluster).resize(number_of_true_particles_.at(cluster));
    true_position_.at(cluster).resize(number_of_true_particles_.at(cluster));
    true_tangent_.at(cluster).resize(number_of_true_particles_.at(cluster));
  }
}

int NTBMSummary::GetNumberOfTrueParticles(int cluster) const {
  if (cluster >= number_of_ninja_clusters_)
    throw std::out_of_range("Number of cluster out of range");
  return number_of_true_particles_.at(cluster);
}

void NTBMSummary::SetTrueParticleId(int cluster, int particle, int true_particle_id) {
  true_particle_id_.at(cluster).at(particle) = true_particle_id;
}

void NTBMSummary::SetTrueParticleId(int cluster, std::vector<int> true_particle_id) {
  for (int particle = 0; particle < number_of_true_particles_.at(cluster); particle++)
    SetTrueParticleId(cluster, particle, true_particle_id.at(particle));
}

std::vector<int> NTBMSummary::GetTrueParticleId(int cluster) const {
  return true_particle_id_.at(cluster);
}

int NTBMSummary::GetTrueParticleId(int cluster, int particle) const {
  return GetTrueParticleId(cluster).at(particle);
}

void NTBMSummary::SetNormalization(double normalization) {
  normalization_ = normalization;
}

double NTBMSummary::GetNormalization() const {
  return normalization_;
}

void NTBMSummary::SetTotalCrossSection(double total_cross_section) {
  total_cross_section_ = total_cross_section;
}

double NTBMSummary::GetTotalCrossSection() const {
  return total_cross_section_;
}

void NTBMSummary::SetTruePosition(int cluster, int particle, int view, double true_position) {
  if (view >= NUMBER_OF_VIEWS)
    throw std::out_of_range("View out of range");
  true_position_.at(cluster).at(particle).at(view) = true_position;
}

void NTBMSummary::SetTruePosition(int cluster, int particle, std::vector<double> true_position) {
  for(std::size_t view = 0; view < NUMBER_OF_VIEWS; view++)
    SetTruePosition(cluster, particle, view, true_position.at(view));
}

void NTBMSummary::SetTruePosition(int cluster, std::vector<std::vector<double>> true_position) {
  for(int particle = 0; particle < number_of_true_particles_.at(cluster); particle++)
    SetTruePosition(cluster, particle, true_position.at(particle));
}

std::vector<std::vector<double>> NTBMSummary::GetTruePosition(int cluster) const {
  if (cluster >= number_of_ninja_clusters_)
    throw std::out_of_range("Number of cluster out of range");
  return true_position_.at(cluster);
}

std::vector<double> NTBMSummary::GetTruePosition(int cluster, int particle) const {
  if (particle >= number_of_true_particles_.at(cluster))
    throw std::out_of_range("Number of true particles our of range");
  return GetTruePosition(cluster).at(particle);
}

double NTBMSummary::GetTruePosition(int cluster, int particle, int view) const {
  if (view >= NUMBER_OF_VIEWS)
    throw std::out_of_range("View out of range");
  return GetTruePosition(cluster, particle).at(view);
}

void NTBMSummary::SetTrueTangent(int cluster, int particle, int view, double true_tangent) {
  if (view >= NUMBER_OF_VIEWS)
    throw std::out_of_range("View ourt of range");
  true_tangent_.at(cluster).at(particle).at(view) = true_tangent;
}

void NTBMSummary::SetTrueTangent(int cluster, int particle, std::vector<double> true_tangent) {
  for (std::size_t view = 0; view < NUMBER_OF_VIEWS; view++)
    SetTrueTangent(cluster, particle, view, true_tangent.at(view));
}

void NTBMSummary::SetTrueTangent(int cluster, std::vector<std::vector<double>> true_tangent) {
  for (int particle = 0; particle < number_of_true_particles_.at(cluster); particle++)
    SetTrueTangent(cluster, particle, true_tangent.at(particle));
}

std::vector<std::vector<double>> NTBMSummary::GetTrueTangent(int cluster) const {
  if (cluster >= number_of_ninja_clusters_)
    throw std::out_of_range("Number of cluster out of range");
  return true_tangent_.at(cluster);
}

std::vector<double> NTBMSummary::GetTrueTangent(int cluster, int particle) const {
  if (particle >= number_of_true_particles_.at(cluster))
    throw std::out_of_range("Number of true particles out of range");
  return GetTrueTangent(cluster).at(particle);
}

double NTBMSummary::GetTrueTangent(int cluster, int particle, int view) const {
  if (view >= NUMBER_OF_VIEWS)
    throw std::out_of_range("View out of range");
  return GetTrueTangent(cluster, particle).at(view);
}

ClassImp(NTBMSummary)
