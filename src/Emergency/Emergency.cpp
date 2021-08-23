#include <B2Reader.hh>
#include <B2SpillSummary.hh>
#include <B2TrackSummary.hh>
#include <B2ClusterSummary.hh>
#include <B2HitSummary.hh>

#include <NTBMSummary.hh>

bool MyHasDetector(const B2TrackSummary *track, B2Detector det) {

  bool ret = false;

  auto it_cluster = track->BeginCluster();
  while (const auto *cluster = it_cluster.Next()) {
    auto it_hit = cluster->BeginHit();
    while (const auto *hit = it_hit.Next()) {
      if (hit->GetDetectorId() == det) ret = true;
    }
  }
  return ret;
}


int MyGetBabyMindMaximumPlane(const B2TrackSummary *track) {

  int return_max_plane = -1;

  auto it_cluster = track->BeginCluster();
  while (const auto *cluster = it_cluster.Next()) {
    auto it_hit = cluster->BeginHit();
    while (const auto *hit = it_hit.Next()) {
      if (hit->GetDetectorId() == B2Detector::kBabyMind && 
	  hit->GetPlane() > return_max_plane)
	return_max_plane = hit->GetPlane();
    }
  }

  return return_max_plane;

}

int main (int argc, char *argv[]) {

  B2Reader reader(argv[1]);

  TFile *ifile = new TFile(argv[2], "read");
  TTree *itree = (TTree*)ifile->Get("tree");
  NTBMSummary *ntbm = nullptr;
  itree->SetBranchAddress("NTBMSummary", &ntbm);

  const Int_t n_itree_entry = itree->GetEntries();

  TFile *ofile = new TFile(argv[3], "recreate");
  TTree *otree = new TTree("tree", "tree");
  otree->Branch("NTBMSummary", &ntbm);

  Int_t i_itree = 0;

  while (reader.ReadNextSpill() > 0) {

    auto &input_spill_summary = reader.GetSpillSummary();

    itree->GetEntry(i_itree);

    std::vector<double> maximum_plate = {};

    auto it_track = input_spill_summary.BeginReconTrack();
    while (const auto *track = it_track.Next()) {
      if (MyHasDetector(track, B2Detector::kBabyMind)) {
	maximum_plate.push_back(MyGetBabyMindMaximumPlane(track));
      }
    }
    for (Int_t itrack = 0; itrack < maximum_plate.size(); itrack++) {
      ntbm->SetMomentum(itrack, maximum_plate.at(itrack));
    }

    otree->Fill();
    ntbm->Clear("C");
    i_itree++;

  }

  ofile->cd();
  otree->Write();
  ofile->Close();

}
