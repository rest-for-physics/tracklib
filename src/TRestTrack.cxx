///______________________________________________________________________________
///______________________________________________________________________________
///______________________________________________________________________________
///
///
///             RESTSoft : Software for Rare Event Searches with TPCs
///
///             TRestTrack.h
///
///             Event class to store signals form simulation and acquisition
///             events
///
///             oct 2015:   First concept
///                 Created as part of the conceptualization of existing REST
///                 software.
///                 Javier Gracia
///_______________________________________________________________________________

#include "TRestTrack.h"

using namespace std;

ClassImp(TRestTrack);

TRestTrack::TRestTrack() {
    // TRestTrack default constructor
}

TRestTrack::~TRestTrack() {
    // TRestTrack destructor
}

void TRestTrack::Initialize() {
    fTrackID = 0;
    fParentID = 0;
    fTrackEnergy = 0;
    fTrackLength = 0;
    fVolumeHits.RemoveHits();
}

void TRestTrack::SetVolumeHits(TRestVolumeHits hits) {
    fVolumeHits = hits;
    fTrackEnergy = hits.GetTotalEnergy();
    fTrackLength = hits.GetTotalDistance();
}

void TRestTrack::RemoveVolumeHits() {
    fVolumeHits.RemoveHits();
    fTrackEnergy = 0;
    fTrackLength = 0;
}

///////////////////////////////////////////////
/// \brief This function retreive the origin and the end of a single
/// track based on the most energetic hit. The origin is defined as the further
/// hit deposition edge to the most energetic hit, while the track end is defined
/// as the closest edge to the most energetic hit. The hits are expected to be
/// ordered following the minimim path.
///
void TRestTrack::GetBoundaries(TVector3& orig, TVector3& end) {
    const int nHits = fVolumeHits.GetNumberOfHits();
    int maxBin = 0;
    double maxEn = 0;

    for (int i = 0; i < nHits; i++) {
        double en = fVolumeHits.GetEnergy(i);
        if (en > maxEn) {
            maxEn = en;
            maxBin = i;
        }
    }

    TVector3 maxPos = fVolumeHits.GetPosition(maxBin);

    TVector3 pos0 = fVolumeHits.GetPosition(0);
    TVector3 posE = fVolumeHits.GetPosition(nHits - 1);

    const double maxToFirst = (pos0 - maxPos).Mag();
    const double maxToLast = (posE - maxPos).Mag();

    if (maxToFirst < maxToLast) {
        end = pos0;
        orig = posE;
    } else {
        orig = pos0;
        end = posE;
    }
}

void TRestTrack::PrintTrack(Bool_t fullInfo) {
    Double_t x = GetMeanPosition().X();
    Double_t y = GetMeanPosition().Y();
    Double_t z = GetMeanPosition().Z();

    cout << "Track ID : " << fTrackID << " Parent ID : " << fParentID;

    if (isXY()) cout << " is XY " << endl;
    if (isXZ()) cout << " is XZ " << endl;
    if (isYZ()) cout << " is YZ " << endl;
    if (isXYZ()) cout << " is XYZ " << endl;
    cout << "Energy : " << fTrackEnergy << endl;
    cout << "Length : " << fTrackLength << endl;
    cout << "Mean position : ( " << x << " , " << y << " , " << z << " ) " << endl;
    cout << "Number of track hits : " << fVolumeHits.GetNumberOfHits() << endl;
    cout << "----------------------------------------" << endl;

    if (fullInfo) {
        fVolumeHits.PrintHits();
        cout << "----------------------------------------" << endl;
    }
}
