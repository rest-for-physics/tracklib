//////////////////////////////////////////////////////////////////////////
///
///             RESTSoft : Software for Rare Event Searches with TPCs
///
///             TRestTrackReductionProcess.cxx
///
///             Jan 2016:   First concept (Javier Galan)
///
//////////////////////////////////////////////////////////////////////////

#include "TRestTrackReductionProcess.h"

using namespace std;

ClassImp(TRestTrackReductionProcess);

TRestTrackReductionProcess::TRestTrackReductionProcess() { Initialize(); }

TRestTrackReductionProcess::~TRestTrackReductionProcess() { delete fOutputTrackEvent; }

void TRestTrackReductionProcess::Initialize() {
    SetSectionName(this->ClassName());
    SetLibraryVersion(LIBRARY_VERSION);

    fInputTrackEvent = nullptr;
    fOutputTrackEvent = new TRestTrackEvent();
}

void TRestTrackReductionProcess::InitProcess() {}

TRestEvent* TRestTrackReductionProcess::ProcessEvent(TRestEvent* evInput) {
    fInputTrackEvent = (TRestTrackEvent*)evInput;
    fOutputTrackEvent->SetEventInfo(fInputTrackEvent);

    // Copying the input tracks to the output track
    for (int tck = 0; tck < fInputTrackEvent->GetNumberOfTracks(); tck++)
        fOutputTrackEvent->AddTrack(fInputTrackEvent->GetTrack(tck));

    if (this->GetVerboseLevel() >= REST_Debug) fInputTrackEvent->PrintOnlyTracks();

    // Reducing the hits inside each track
    for (int tck = 0; tck < fInputTrackEvent->GetNumberOfTracks(); tck++) {
        if (!fInputTrackEvent->isTopLevel(tck)) continue;

        TRestTrack* track = fInputTrackEvent->GetTrack(tck);
        TRestVolumeHits* hits = track->GetVolumeHits();

        if (this->GetVerboseLevel() >= REST_Debug)
            cout << "TRestTrackReductionProcess. Reducing hits in track id : " << track->GetTrackID() << endl;

        TRestVolumeHits vHits = (TRestVolumeHits)(*hits);
        getHitsMerged(vHits);
        if (fKmeans) {
            TRestVolumeHits::kMeansClustering(hits, vHits, fMaxIt);
            int nHitsBefore;
            int nHitsAfter;
            do {
                nHitsBefore = vHits.GetNumberOfHits();
                getHitsMerged(vHits);
                TRestVolumeHits::kMeansClustering(hits, vHits, fMaxIt);
                nHitsAfter = vHits.GetNumberOfHits();
            } while (nHitsBefore != nHitsAfter);
        }
        TRestTrack newTrack;
        newTrack.SetVolumeHits(vHits);
        newTrack.SetParentID(track->GetTrackID());
        newTrack.SetTrackID(fOutputTrackEvent->GetNumberOfTracks() + 1);
        fOutputTrackEvent->AddTrack(&newTrack);
    }

    fOutputTrackEvent->SetLevels();
    return fOutputTrackEvent;
}


void TRestTrackReductionProcess::getHitsMerged(TRestVolumeHits& hits) {
    Double_t distance = fStartingDistance;
    while (distance < fMinimumDistance || hits.GetNumberOfHits() > fMaxNodes) {
        if (this->GetVerboseLevel() >= REST_Debug) {
            cout << "TRestTrackReductionProcess. Merging track hits within a "
                 << "distance : " << distance << " mm" << endl;
            cout << "TRestTrackReductionProcess. Hits now : " << hits.GetNumberOfHits() << endl;
        }

        Int_t mergedHits = 0;

        Bool_t merged = true;
        while (merged) {
            merged = false;
            for (int i = 0; i < hits.GetNumberOfHits(); i++) {
                for (int j = i + 1; j < hits.GetNumberOfHits(); j++) {
                    if (hits.GetDistance2(i, j) < distance * distance) {
                        mergedHits++;
                        hits.MergeHits(i, j);
                        merged = true;
                    }
                }
            }
        }

        debug << "TRestTrackReductionProcess. Number of hits merged : " << mergedHits << endl;
        mergedHits = 0;

        distance *= fDistanceStepFactor;
    }
}

void TRestTrackReductionProcess::EndProcess() {}
