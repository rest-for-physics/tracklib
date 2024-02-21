///______________________________________________________________________________
///______________________________________________________________________________
///______________________________________________________________________________
///
///
///             RESTSoft : Software for Rare Event Searches with TPCs
///
///             TRestTrackEvent.h
///
///             Event class to store DAQ events either from simulation and
///             acquisition
///
///             oct 2015:   First concept
///                 Created as part of the conceptualization of existing REST
///                 software.
///                 Javier Gracia
///
///
///_______________________________________________________________________________

#include "TRestTrackEvent.h"

#include "TRestRun.h"
#include "TRestTools.h"

using namespace std;

ClassImp(TRestTrackEvent);

TRestTrackEvent::TRestTrackEvent() {
    // TRestTrackEvent default constructor
    TRestEvent::Initialize();
    fTrack.clear();
    fXYHit = nullptr;
    fXZHit = nullptr;
    fYZHit = nullptr;
    fXYZHit = nullptr;
    fXYTrack = nullptr;
    fXZTrack = nullptr;
    fYZTrack = nullptr;
    fXYZTrack = nullptr;
    fPad = nullptr;
    fLevels = -1;

    fPrintHitsWarning = true;
}

TRestTrackEvent::~TRestTrackEvent() {
    // TRestTrackEvent destructor
}

void TRestTrackEvent::Initialize() {
    fNtracks = 0;
    fNtracksX = 0;
    fNtracksY = 0;
    fTrack.clear();
    TRestEvent::Initialize();
}

void TRestTrackEvent::AddTrack(TRestTrack* c) {
    if (c->isXZ()) fNtracksX++;
    if (c->isYZ()) fNtracksY++;
    fNtracks++;

    fTrack.push_back(*c);

    SetLevels();
}

void TRestTrackEvent::RemoveTrack(int n) {
    if (fTrack[n].isXZ()) fNtracksX--;
    if (fTrack[n].isYZ()) fNtracksY--;
    fNtracks--;

    fTrack.erase(fTrack.begin() + n);

    SetLevels();
}

Int_t TRestTrackEvent::GetNumberOfTracks(TString option) {
    if (option == "")
        return fNtracks;
    else {
        Int_t nT = 0;
        for (int n = 0; n < GetNumberOfTracks(); n++) {
            if (!this->isTopLevel(n)) continue;

            if (option == "X" && GetTrack(n)->isXZ())
                nT++;
            else if (option == "Y" && GetTrack(n)->isYZ())
                nT++;
            else if (option == "XYZ" && GetTrack(n)->isXYZ())
                nT++;
        }

        return nT;
    }

    return -1;
}

TRestTrack* TRestTrackEvent::GetTrackById(Int_t id) {
    for (int i = 0; i < GetNumberOfTracks(); i++)
        if (GetTrack(i)->GetTrackID() == id) return GetTrack(i);
    return nullptr;
}

TRestTrack* TRestTrackEvent::GetMaxEnergyTrackInX() {
    Int_t track = -1;
    Double_t maxEnergy = 0;
    for (int tck = 0; tck < GetNumberOfTracks(); tck++) {
        if (!this->isTopLevel(tck)) continue;

        TRestTrack* t = GetTrack(tck);
        if (t->isXZ()) {
            if (t->GetEnergy() > maxEnergy) {
                maxEnergy = t->GetEnergy();
                track = tck;
            }
        }
    }

    if (track == -1) return nullptr;

    return GetTrack(track);
}

TRestTrack* TRestTrackEvent::GetMaxEnergyTrackInY() {
    Int_t track = -1;
    Double_t maxEnergy = 0;
    for (int tck = 0; tck < GetNumberOfTracks(); tck++) {
        if (!this->isTopLevel(tck)) continue;

        TRestTrack* t = GetTrack(tck);
        if (t->isYZ()) {
            if (t->GetEnergy() > maxEnergy) {
                maxEnergy = t->GetEnergy();
                track = tck;
            }
        }
    }

    if (track == -1) return nullptr;

    return GetTrack(track);
}

TRestTrack* TRestTrackEvent::GetMaxEnergyTrack(TString option) {
    if (option == "X") return GetMaxEnergyTrackInX();
    if (option == "Y") return GetMaxEnergyTrackInY();

    Int_t track = -1;
    Double_t maxEnergy = 0;
    for (int tck = 0; tck < GetNumberOfTracks(); tck++) {
        if (!this->isTopLevel(tck)) continue;

        TRestTrack* t = GetTrack(tck);
        if (t->isXYZ()) {
            if (t->GetEnergy() > maxEnergy) {
                maxEnergy = t->GetEnergy();
                track = tck;
            }
        }
    }

    if (track == -1) return nullptr;

    return GetTrack(track);
}

TRestTrack* TRestTrackEvent::GetSecondMaxEnergyTrack(TString option) {
    if (GetMaxEnergyTrack(option) == nullptr) return nullptr;

    Int_t id = GetMaxEnergyTrack(option)->GetTrackID();

    Int_t track = -1;
    Double_t maxEnergy = 0;
    for (int tck = 0; tck < GetNumberOfTracks(); tck++) {
        if (!this->isTopLevel(tck)) continue;

        TRestTrack* t = GetTrack(tck);
        if (t->GetTrackID() == id) continue;

        Double_t en = t->GetEnergy();

        if (option == "X" && (t->isXZ() || t->isXYZ())) {
            if (en > maxEnergy) {
                maxEnergy = t->GetEnergy();
                track = tck;
            }
        } else if (option == "Y" && (t->isYZ() || t->isXYZ())) {
            if (t->GetEnergy() > maxEnergy) {
                maxEnergy = t->GetEnergy();
                track = tck;
            }
        } else if (t->isXYZ()) {
            if (t->GetEnergy() > maxEnergy) {
                maxEnergy = t->GetEnergy();
                track = tck;
            }
        }
    }

    if (track == -1) return nullptr;

    return GetTrack(track);
}

Double_t TRestTrackEvent::GetMaxEnergyTrackVolume(TString option) {
    if (this->GetMaxEnergyTrack(option)) return this->GetMaxEnergyTrack(option)->GetVolume();
    return 0;
}

Double_t TRestTrackEvent::GetMaxEnergyTrackLength(TString option) {
    if (this->GetMaxEnergyTrack(option)) return this->GetMaxEnergyTrack(option)->GetLength();
    return 0;
}

Double_t TRestTrackEvent::GetEnergy(TString option) {
    Double_t en = 0;
    for (int tck = 0; tck < this->GetNumberOfTracks(); tck++) {
        if (!this->isTopLevel(tck)) continue;

        TRestTrack* t = GetTrack(tck);

        if (option == "")
            en += t->GetEnergy();

        else if (option == "X" && (t->isXZ() || t->isXYZ()))
            en += t->GetEnergy();

        else if (option == "Y" && (t->isYZ() || t->isXYZ()))
            en += t->GetEnergy();

        else if (option == "XYZ" && t->isXYZ())
            en += t->GetEnergy();
    }

    return en;
}

Bool_t TRestTrackEvent::isXYZ() {
    for (int tck = 0; tck < GetNumberOfTracks(); tck++)
        if (!fTrack[tck].isXYZ()) return false;
    return true;
}

Int_t TRestTrackEvent::GetTotalHits() {
    Int_t totHits = 0;
    for (int tck = 0; tck < GetNumberOfTracks(); tck++) totHits += GetTrack(tck)->GetNumberOfHits();
    return totHits;
}

Int_t TRestTrackEvent::GetLevel(Int_t tck) {
    Int_t lvl = 1;
    Int_t parentTrackId = GetTrack(tck)->GetParentID();

    while (parentTrackId > 0) {
        lvl++;
        parentTrackId = GetTrackById(parentTrackId)->GetParentID();
    }
    return lvl;
}

Bool_t TRestTrackEvent::isTopLevel(Int_t tck) {
    if (GetLevels() == GetLevel(tck)) return true;
    return false;
}

Int_t TRestTrackEvent::GetOriginTrackID(Int_t tck) {
    Int_t originTrackID = tck;
    Int_t pID = GetTrack(tck)->GetParentID();

    while (pID != 0) {
        originTrackID = pID;
        pID = GetTrackById(originTrackID)->GetParentID();
    }

    return originTrackID;
}

TRestTrack* TRestTrackEvent::GetOriginTrack(Int_t tck) {
    Int_t originTrackID = GetTrack(tck)->GetTrackID();
    Int_t pID = GetTrackById(originTrackID)->GetParentID();

    while (pID != 0) {
        originTrackID = pID;
        pID = GetTrackById(originTrackID)->GetParentID();
    }

    return GetTrackById(originTrackID);
}

TRestTrack* TRestTrackEvent::GetOriginTrackById(Int_t tckId) {
    Int_t originTrackID = tckId;
    Int_t pID = GetTrackById(tckId)->GetParentID();

    while (pID != 0) {
        originTrackID = pID;
        pID = GetTrackById(originTrackID)->GetParentID();
    }

    return GetTrackById(originTrackID);
}

void TRestTrackEvent::SetLevels() {
    Int_t maxLevel = 0;

    for (int tck = 0; tck < GetNumberOfTracks(); tck++) {
        Int_t lvl = GetLevel(tck);
        if (maxLevel < lvl) maxLevel = lvl;
    }
    fLevels = maxLevel;
}

///////////////////////////////////////////////
/// \brief This function retrieves the origin and the end track positions
/// based after the reconstruction of a 3D track. It requires the track to have
/// the same number of hits in X and Y. Two different directions are scanned and
/// the one which maximizes the track length is retrieved. Afterwards the position
/// of the closer hit to the half integral of the track is obtained. Then, the origin
/// of the track is defined as the further edge to the half integral, while the track
/// end is defined as the closest edge.
///
TRestVolumeHits TRestTrackEvent::GetMaxTrackBoundaries3D(TVector3& orig, TVector3& end) {
    TRestTrack* tckX = GetMaxEnergyTrackInX();
    TRestTrack* tckY = GetMaxEnergyTrackInY();

    if (tckX == nullptr || tckY == nullptr) {
        RESTWarning << "Track is empty, skipping" << RESTendl;
        return {};
    }

    TRestVolumeHits hitsX = (TRestVolumeHits) * (tckX->GetVolumeHits());
    TRestVolumeHits hitsY = (TRestVolumeHits) * (tckY->GetVolumeHits());

    const int nHits = std::min(hitsX.GetNumberOfHits(), hitsY.GetNumberOfHits());
    TRestVolumeHits best3DHits, hits3D;

    for (int i = 0; i < nHits; i++) {
        double enX = hitsX.GetEnergy(i);
        double enY = hitsY.GetEnergy(i);
        double posXZ = hitsX.GetZ(i);
        double posYZ = hitsY.GetZ(i);
        double avgZ = (enX * posXZ + enY * posYZ) / (enX + enY);
        best3DHits.AddHit(hitsX.GetX(i), hitsY.GetY(i), avgZ, enX + enY, 0, XYZ, 0, 0, 0);
        const int j = nHits - i - 1;
        enY = hitsY.GetEnergy(j);
        posYZ = hitsY.GetZ(j);
        avgZ = (enX * posXZ + enY * posYZ) / (enX + enY);
        hits3D.AddHit(hitsX.GetX(i), hitsY.GetY(j), avgZ, enX + enY, 0, XYZ, 0, 0, 0);
    }

    double length = (best3DHits.GetPosition(0) - best3DHits.GetPosition(nHits - 1)).Mag();

    if ((hits3D.GetPosition(0) - hits3D.GetPosition(nHits - 1)).Mag() > length) {
        best3DHits = hits3D;
    }

    double totEn = 0;
    for (unsigned int i = 0; i < best3DHits.GetNumberOfHits(); i++) {
        totEn += best3DHits.GetEnergy(i);
    }

    const TVector3 pos0 = best3DHits.GetPosition(0);
    const TVector3 posE = best3DHits.GetPosition(best3DHits.GetNumberOfHits() - 1);

    double integ = 0;
    unsigned int pos = 0;
    for (pos = 0; pos < best3DHits.GetNumberOfHits(); pos++) {
        integ += best3DHits.GetEnergy(pos);
        if (integ > totEn / 2.) break;
    }

    auto intPos = best3DHits.GetPosition(pos);
    const double intToFirst = (pos0 - intPos).Mag();
    const double intToLast = (posE - intPos).Mag();

    RESTDebug << "Integ pos " << pos << " Pos to first " << intToFirst << " last " << intToLast << RESTendl;
    if (intToFirst < intToLast) {
        end = pos0;
        orig = posE;
    } else {
        orig = pos0;
        end = posE;
    }

    RESTDebug << "Origin " << orig.X() << " " << orig.Y() << " " << orig.Z() << RESTendl;
    RESTDebug << "End    " << end.X() << " " << end.Y() << " " << end.Z() << RESTendl;

    return best3DHits;
}

///////////////////////////////////////////////
/// \brief This function retreive the origin and the end of the track
/// based on the most energetic hit. The origin is defined as the further
/// hit deposition edge to the most energetic hit, while the track end is
/// defined as the closest edge to the most energetic hit.
///
void TRestTrackEvent::GetMaxTrackBoundaries(TVector3& orig, TVector3& end) {
    TRestTrack* tckX = GetMaxEnergyTrackInX();
    TRestTrack* tckY = GetMaxEnergyTrackInY();

    if (tckX == nullptr || tckY == nullptr) {
        RESTWarning << "Track is empty, skipping" << RESTendl;
        return;
    }

    TVector3 origX, endX;
    // Retreive origin and end of the track for the XZ projection
    tckX->GetBoundaries(origX, endX);
    TVector3 origY, endY;
    // Retreive origin and end of the track for the YZ projection
    tckY->GetBoundaries(origY, endY);

    double originZ = (origX.Z() + origY.Z()) / 2.;
    double endZ = (endX.Z() + endY.Z()) / 2.;

    orig = TVector3(origX.X(), origY.Y(), originZ);
    end = TVector3(endX.X(), endY.Y(), endZ);
}

///////////////////////////////////////////////
/// \brief Function to calculate the relative Z of the most energetic
/// track to crosscheck if the track is upwards or downwards
///
Double_t TRestTrackEvent::GetMaxTrackRelativeZ() {
    TRestTrack* tckX = GetMaxEnergyTrackInX();
    TRestTrack* tckY = GetMaxEnergyTrackInY();

    if (tckX == nullptr || tckY == nullptr) {
        RESTWarning << "Track is empty, skipping" << RESTendl;
        return -1;
    }

    std::vector<std::pair<double, double> > zEn;
    double totEn = 0;

    for (size_t i = 0; i < tckX->GetVolumeHits()->GetNumberOfHits(); i++) {
        double en = tckX->GetVolumeHits()->GetEnergy(i);
        double z = tckX->GetVolumeHits()->GetZ(i);
        zEn.push_back(std::make_pair(z, en));
        totEn += en;
    }

    for (size_t i = 0; i < tckY->GetVolumeHits()->GetNumberOfHits(); i++) {
        double en = tckY->GetVolumeHits()->GetEnergy(i);
        double z = tckY->GetVolumeHits()->GetZ(i);
        zEn.push_back(std::make_pair(z, en));
        totEn += en;
    }

    std::sort(zEn.begin(), zEn.end());

    double integ = 0;
    size_t pos = 0;
    for (pos = 0; pos < zEn.size(); pos++) {
        integ += zEn[pos].second;
        if (integ >= totEn / 2.) break;
    }

    double length = zEn.front().first - zEn.back().first;
    double diff = zEn.front().first - zEn[pos].first;

    if (length == 0)
        return 0;
    else
        return diff / length;
}

void TRestTrackEvent::PrintOnlyTracks() {
    cout << "TrackEvent " << GetID() << endl;
    cout << "-----------------------" << endl;
    for (int i = 0; i < GetNumberOfTracks(); i++) {
        cout << "Track " << i << " id : " << GetTrack(i)->GetTrackID()
             << " parent : " << GetTrack(i)->GetParentID() << endl;
    }
    cout << "-----------------------" << endl;
    cout << "Track levels : " << GetLevels() << endl;
}

void TRestTrackEvent::PrintEvent(Bool_t fullInfo) {
    TRestEvent::PrintEvent();

    cout << "Number of tracks : "
         << GetNumberOfTracks("XYZ") + GetNumberOfTracks("X") + GetNumberOfTracks("Y") << endl;
    cout << "Number of tracks XZ " << GetNumberOfTracks("X") << endl;
    cout << "Number of tracks YZ " << GetNumberOfTracks("Y") << endl;
    cout << "Track levels : " << GetLevels() << endl;
    cout << "+++++++++++++++++++++++++++++++++++" << endl;
    for (int i = 0; i < GetNumberOfTracks(); i++) this->GetTrack(i)->PrintTrack(fullInfo);
}

TPad* TRestTrackEvent::DrawHits() {
    if (fXZHits) {
        delete fXZHits;
        fXZHits = nullptr;
    }
    if (fYZHits) {
        delete fYZHits;
        fYZHits = nullptr;
    }
    if (fHitsPad) {
        delete fHitsPad;
        fHitsPad = nullptr;
    }

    std::vector<double> fX, fY, fZ;

    for (int t = 0; t < GetNumberOfTracks(); t++) {
        TRestTrack* tck = GetTrack(t);
        if (GetLevel(t) != 1) continue;
        TRestVolumeHits* hits = tck->GetVolumeHits();
        for (unsigned int i = 0; i < hits->GetNumberOfHits(); i++) {
            if (hits->GetType(i) % X == 0) fX.emplace_back(hits->GetX(i));
            if (hits->GetType(i) % Y == 0) fY.emplace_back(hits->GetY(i));
            if (hits->GetType(i) % Z == 0) fZ.emplace_back(hits->GetZ(i));
        }
    }

    double maxX, minX, maxY, minY, maxZ, minZ;
    int nBinsX, nBinsY, nBinsZ;
    TRestHits::GetBoundaries(fX, maxX, minX, nBinsX);
    TRestHits::GetBoundaries(fY, maxY, minY, nBinsY);
    TRestHits::GetBoundaries(fZ, maxZ, minZ, nBinsZ);

    fXZHits = new TH2F("TXZ", "TXZ", nBinsX, minX, maxX, nBinsZ, minZ, maxZ);
    fYZHits = new TH2F("TYZ", "TYZ", nBinsY, minY, maxY, nBinsZ, minZ, maxZ);

    for (int t = 0; t < GetNumberOfTracks(); t++) {
        TRestTrack* tck = GetTrack(t);
        if (GetLevel(t) != 1) continue;
        TRestVolumeHits* hits = tck->GetVolumeHits();
        for (unsigned int i = 0; i < hits->GetNumberOfHits(); i++) {
            if (hits->GetType(i) == XZ) fXZHits->Fill(hits->GetX(i), hits->GetZ(i), hits->GetEnergy(i));
            if (hits->GetType(i) == YZ) fYZHits->Fill(hits->GetY(i), hits->GetZ(i), hits->GetEnergy(i));
        }
    }

    fHitsPad = new TPad("TrackHits", "TrackHits", 0, 0, 1, 1);
    fHitsPad->Divide(2, 1);
    fHitsPad->Draw();

    fHitsPad->cd(1);
    fXZHits->GetXaxis()->SetTitle("X-axis (mm)");
    fXZHits->GetYaxis()->SetTitle("Z-axis (mm)");
    fXZHits->Draw("COLZ");

    fHitsPad->cd(2);
    fYZHits->GetXaxis()->SetTitle("Y-axis (mm)");
    fYZHits->GetYaxis()->SetTitle("Z-axis (mm)");
    fYZHits->Draw("COLZ");

    return fHitsPad;
}

// Draw current event in a Tpad
TPad* TRestTrackEvent::DrawEvent(const TString& option) {
    /* Not used for the moment
    Bool_t drawXZ = false;
    Bool_t drawYZ = false;
    Bool_t drawXY = false;
    Bool_t drawXYZ = false;
    Bool_t drawLines = false;
    */

    Int_t maxLevel = 0;
    Int_t minLevel = 0;

    vector<string> optList = TRestTools::GetOptions((string)option);

    for (unsigned int n = 0; n < optList.size(); n++) {
        if (optList[n] == "print") this->PrintEvent();
        if (optList[n] == "noWarning") fPrintHitsWarning = false;
    }

    optList.erase(std::remove(optList.begin(), optList.end(), "print"), optList.end());
    optList.erase(std::remove(optList.begin(), optList.end(), "noWarning"), optList.end());

    for (unsigned int n = 0; n < optList.size(); n++) {
        /* Not used for the moment
        if( optList[n] == "XZ" ) drawXZ = true;
        if( optList[n] == "YZ" ) drawYZ = true;
        if( optList[n] == "XY" ) drawXY = true;
        if( optList[n] == "XYZ" ) drawXYZ = true;
        if( optList[n] == "L" || optList[n] == "lines"  ) drawLines = true;
        */
        string opt = optList[n];

        if (opt.find("maxLevel=") != string::npos) maxLevel = stoi(opt.substr(9, opt.length()).c_str());

        if (opt.find("minLevel=") != string::npos) minLevel = stoi(opt.substr(9, opt.length()).c_str());
    }

    if (fXYHit != nullptr) {
        delete[] fXYHit;
        fXYHit = nullptr;
    }
    if (fXZHit != nullptr) {
        delete[] fXZHit;
        fXZHit = nullptr;
    }
    if (fYZHit != nullptr) {
        delete[] fYZHit;
        fYZHit = nullptr;
    }
    if (fXYZHit != nullptr) {
        delete[] fXYZHit;
        fXYZHit = nullptr;
    }
    if (fXYTrack != nullptr) {
        delete[] fXYTrack;
        fXYTrack = nullptr;
    }
    if (fXZTrack != nullptr) {
        delete[] fXZTrack;
        fXZTrack = nullptr;
    }
    if (fYZTrack != nullptr) {
        delete[] fYZTrack;
        fYZTrack = nullptr;
    }
    if (fXYZTrack != nullptr) {
        delete[] fXYZTrack;
        fXYZTrack = nullptr;
    }
    if (fPad != nullptr) {
        delete fPad;
        fPad = nullptr;
    }

    int nTracks = this->GetNumberOfTracks();

    if (nTracks == 0) {
        cout << "Empty event " << endl;
        return nullptr;
    }

    this->PrintEvent(false);

    double maxX = -1e10, minX = 1e10, maxZ = -1e10, minZ = 1e10, maxY = -1e10, minY = 1e10;

    Int_t nTotHits = GetTotalHits();

    if (fPrintHitsWarning && nTotHits > 5000) {
        cout << endl;
        cout << " REST WARNING. TRestTrackEvent::DrawEvent. Number of hits is too "
                "high."
             << endl;
        cout << " This drawing method is not properly optimized to draw events "
                "with a high number of hits."
             << endl;
        cout << " To remove this warning you may use the DrawEvent method option : "
                "noWarning "
             << endl;
        cout << endl;

        fPrintHitsWarning = false;
    }

    fXYHit = new TGraph[nTotHits];
    fXZHit = new TGraph[nTotHits];
    fYZHit = new TGraph[nTotHits];
    fXYZHit = new TGraph2D[nTotHits];
    fXYTrack = new TGraph[nTracks];
    fXZTrack = new TGraph[nTracks];
    fYZTrack = new TGraph[nTracks];
    fXYZTrack = new TGraph2D[nTracks];

    vector<Int_t> drawLinesXY(nTracks, 0);
    vector<Int_t> drawLinesXZ(nTracks, 0);
    vector<Int_t> drawLinesYZ(nTracks, 0);
    vector<Int_t> drawLinesXYZ(nTracks, 0);

    int countXY = 0, countYZ = 0, countXZ = 0, countXYZ = 0;
    int nTckXY = 0, nTckXZ = 0, nTckYZ = 0, nTckXYZ = 0;

    Double_t minRadiusSize = 0.4;
    Double_t maxRadiusSize = 2.;

    Int_t maxTrackHits = 0;

    Int_t tckColor = 1;

    for (int tck = 0; tck < nTracks; tck++) {
        TRestVolumeHits* hits = fTrack[tck].GetVolumeHits();

        Double_t maxHitEnergy = hits->GetMaximumHitEnergy();
        Double_t meanHitEnergy = hits->GetMeanHitEnergy();

        /*
        cout << "Max hit energy : " << maxHitEnergy << endl;
        cout << "Mean hit energy : " << meanHitEnergy << endl;
        cout << "Number of hits " << hits->GetNumberOfHits( ) <<endl;
        */

        Bool_t isTopLevel = this->isTopLevel(tck);
        if (isTopLevel) tckColor++;
        Int_t level = this->GetLevel(tck);

        if (!isTopLevel && maxLevel > 0 && level > maxLevel) continue;

        if (!isTopLevel && minLevel > 0 && level < minLevel) continue;

        int tckXY = 0, tckYZ = 0, tckXZ = 0, tckXYZ = 0;
        Double_t radius;

        for (unsigned int nhit = 0; nhit < hits->GetNumberOfHits(); nhit++) {
            if (hits->GetNumberOfHits() > (unsigned int)maxTrackHits) maxTrackHits = hits->GetNumberOfHits();

            if (hits->isNaN(nhit)) {
                cout << "REST Warning. TRestTrackEvent::Draw. Hit is not defined!!" << endl;
                getchar();
                continue;
            }

            Double_t x = hits->GetX(nhit);
            Double_t y = hits->GetY(nhit);
            Double_t z = hits->GetZ(nhit);
            Double_t en = hits->GetEnergy(nhit);
            auto type = hits->GetType(nhit);

            // cout << x << " " << y << " " << z << " " << type << endl;

            /* {{{ Hit size definition (radius) */
            Double_t m = (maxRadiusSize) / (maxHitEnergy - meanHitEnergy);
            Double_t n = (maxRadiusSize - minRadiusSize) - m * meanHitEnergy;

            if (isTopLevel) {
                radius = m * en + n;
                if (radius < 0.1) radius = 0.1;
                if (hits->GetNumberOfHits() == 1) radius = 2;
                if (hits->GetNumberOfHits() > 100) radius = 0.8;

            } else {
                radius = 1.5 * minRadiusSize * level;
            }
            /* }}} */

            if (this->isXYZ() && nhit > 1) {
                if (tckXYZ == 0) nTckXYZ++;
                fXYZTrack[nTckXYZ - 1].SetPoint(tckXYZ, x, y, z);

                if (isTopLevel) drawLinesXYZ[nTckXYZ - 1] = 1;

                if (isTopLevel) {
                    fXYZHit[countXYZ].SetPoint(0, x, y, z);
                    // If there is only one-point the TGraph2D does NOT draw the point!
                    fXYZHit[countXYZ].SetPoint(1, x + 0.001, y + 0.001, z + 0.001);

                    fXYZHit[countXYZ].SetMarkerColor(level + 11);

                    fXYZHit[countXYZ].SetMarkerSize(radius);
                    fXYZHit[countXYZ].SetMarkerStyle(20);
                    countXYZ++;
                }
                tckXYZ++;
            }

            if (type == XY) {
                if (tckXY == 0) nTckXY++;
                fXYTrack[nTckXY - 1].SetPoint(tckXY, x, y);
                if (isTopLevel) drawLinesXY[nTckXY - 1] = 1;
                fXYHit[countXY].SetPoint(0, x, y);

                if (!isTopLevel)
                    fXYHit[countXY].SetMarkerColor(level + 11);
                else
                    fXYHit[countXY].SetMarkerColor(tckColor);

                fXYHit[countXY].SetMarkerSize(radius);
                fXYHit[countXY].SetMarkerStyle(20);
                tckXY++;
                countXY++;
            }

            if (type == XZ) {
                if (tckXZ == 0) nTckXZ++;
                fXZTrack[nTckXZ - 1].SetPoint(tckXZ, x, z);
                if (isTopLevel) drawLinesXZ[nTckXZ - 1] = 1;
                fXZHit[countXZ].SetPoint(0, x, z);

                if (!isTopLevel)
                    fXZHit[countXZ].SetMarkerColor(level + 11);
                else
                    fXZHit[countXZ].SetMarkerColor(tckColor);

                fXZHit[countXZ].SetMarkerSize(radius);
                fXZHit[countXZ].SetMarkerStyle(20);
                tckXZ++;
                countXZ++;
            }

            if (type == YZ) {
                if (tckYZ == 0) nTckYZ++;
                fYZTrack[nTckYZ - 1].SetPoint(tckYZ, y, z);
                if (isTopLevel) drawLinesYZ[nTckYZ - 1] = 1;
                fYZHit[countYZ].SetPoint(0, y, z);

                if (!isTopLevel)
                    fYZHit[countYZ].SetMarkerColor(level + 11);
                else
                    fYZHit[countYZ].SetMarkerColor(tckColor);

                fYZHit[countYZ].SetMarkerSize(radius);
                fYZHit[countYZ].SetMarkerStyle(20);
                tckYZ++;
                countYZ++;
            }

            if (type % X == 0) {
                if (x > maxX) maxX = x;
                if (x < minX) minX = x;
            }

            if (type % Y == 0) {
                if (y > maxY) maxY = y;
                if (y < minY) minY = y;
            }

            if (type % Z == 0) {
                if (z > maxZ) maxZ = z;
                if (z < minZ) minZ = z;
            }
        }
    }

    fPad = new TPad(this->GetName(), " ", 0, 0, 1, 1);
    if (this->isXYZ())
        fPad->Divide(2, 2);
    else
        fPad->Divide(2, 1);
    fPad->Draw();

    char title[256];
    sprintf(title, "Event ID %d", this->GetID());

    TMultiGraph* mgXY = new TMultiGraph();
    TMultiGraph* mgXZ = new TMultiGraph();
    TMultiGraph* mgYZ = new TMultiGraph();

    fPad->cd(1)->DrawFrame(minX - 10, minZ - 10, maxX + 10, maxZ + 10, title);
    fPad->cd(2)->DrawFrame(minY - 10, minZ - 10, maxY + 10, maxZ + 10, title);

    if (this->isXYZ()) fPad->cd(3)->DrawFrame(minX - 10, minY - 10, maxX + 10, maxY + 10, title);

    for (int i = 0; i < countXZ; i++) mgXZ->Add(&fXZHit[i]);

    fPad->cd(1);
    mgXZ->GetXaxis()->SetTitle("X-axis (mm)");
    mgXZ->GetYaxis()->SetTitle("Z-axis (mm)");
    mgXZ->GetYaxis()->SetTitleOffset(1.75);
    mgXZ->GetYaxis()->SetTitleSize(1.4 * mgXZ->GetYaxis()->GetTitleSize());
    mgXZ->GetXaxis()->SetTitleSize(1.4 * mgXZ->GetXaxis()->GetTitleSize());
    mgXZ->GetYaxis()->SetLabelSize(1.25 * mgXZ->GetYaxis()->GetLabelSize());
    mgXZ->GetXaxis()->SetLabelSize(1.25 * mgXZ->GetXaxis()->GetLabelSize());
    mgXZ->Draw("P");

    for (int i = 0; i < countYZ; i++) mgYZ->Add(&fYZHit[i]);

    fPad->cd(2);
    mgYZ->GetXaxis()->SetTitle("Y-axis (mm)");
    mgYZ->GetYaxis()->SetTitle("Z-axis (mm)");
    mgYZ->GetYaxis()->SetTitleOffset(1.75);
    mgYZ->GetYaxis()->SetTitleSize(1.4 * mgYZ->GetYaxis()->GetTitleSize());
    mgYZ->GetXaxis()->SetTitleSize(1.4 * mgYZ->GetXaxis()->GetTitleSize());
    mgYZ->GetYaxis()->SetLabelSize(1.25 * mgYZ->GetYaxis()->GetLabelSize());
    mgYZ->GetXaxis()->SetLabelSize(1.25 * mgYZ->GetXaxis()->GetLabelSize());
    mgYZ->Draw("P");

    if (this->isXYZ()) {
        for (int i = 0; i < countXY; i++) mgXY->Add(&fXYHit[i]);

        fPad->cd(3);
        mgXY->GetXaxis()->SetTitle("X-axis (mm)");
        mgXY->GetYaxis()->SetTitle("Y-axis (mm)");
        mgXY->GetYaxis()->SetTitleOffset(1.75);
        mgXY->Draw("P");
        mgXY->GetYaxis()->SetTitleSize(1.4 * mgXY->GetYaxis()->GetTitleSize());
        mgXY->GetXaxis()->SetTitleSize(1.4 * mgXY->GetXaxis()->GetTitleSize());
        mgXY->GetYaxis()->SetLabelSize(1.25 * mgXY->GetYaxis()->GetLabelSize());
        mgXY->GetXaxis()->SetLabelSize(1.25 * mgXY->GetXaxis()->GetLabelSize());
    }

    for (int tck = 0; tck < nTckXZ; tck++) {
        fPad->cd(1);
        fXZTrack[tck].SetLineWidth(2.);
        if (fXZTrack[tck].GetN() < 100 && drawLinesXZ[tck] == 1) fXZTrack[tck].Draw("L");
    }

    for (int tck = 0; tck < nTckYZ; tck++) {
        fPad->cd(2);
        fYZTrack[tck].SetLineWidth(2.);
        if (fYZTrack[tck].GetN() < 100 && drawLinesYZ[tck] == 1) fYZTrack[tck].Draw("L");
    }

    if (this->isXYZ()) {
        for (int tck = 0; tck < nTckXY; tck++) {
            fPad->cd(3);
            fXYTrack[tck].SetLineWidth(2.);
            if (fXYTrack[tck].GetN() < 100 && drawLinesXY[tck] == 1) fXYTrack[tck].Draw("L");
        }

        fPad->cd(4);

        TString option = "P";
        for (int tck = 0; tck < nTckXYZ; tck++) {
            fXYZTrack[tck].SetLineWidth(2.);
            if (fXZTrack[tck].GetN() < 100 && drawLinesXYZ[tck] == 1) {
                fXYZTrack[tck].Draw("LINE");
                option = "same P";
            }
        }

        for (int i = 0; i < countXYZ; i++) {
            if (i > 0) option = "same P";
            fXYZHit[i].Draw(option);
        }
    }

    return fPad;
}

///////////////////////////////////////////////
/// \brief Retreive origin and end of the track and store in a TGraph
/// and legend
///
void TRestTrackEvent::GetOriginEnd(std::vector<TGraph*>& originGr, std::vector<TGraph*>& endGr,
                                   std::vector<TLegend*>& leg) {
    if (originGr.size() != 2 || endGr.size() != 2 || leg.size() != 2) return;

    for (auto gr : originGr)
        if (gr) delete gr;

    for (auto gr : endGr)
        if (gr) delete gr;

    for (auto l : leg)
        if (l) delete l;

    TVector3 orig, end;
    GetMaxTrackBoundaries(orig, end);

    for (int i = 0; i < 2; i++) {
        originGr[i] = new TGraph();
        originGr[i]->SetPoint(0, orig[i], orig[2]);
        originGr[i]->SetMarkerColor(kRed);
        originGr[i]->SetMarkerStyle(20);
        endGr[i] = new TGraph();
        endGr[i]->SetPoint(0, end[i], end[2]);
        endGr[i]->SetMarkerColor(kBlack);
        endGr[i]->SetMarkerStyle(20);
        leg[i] = new TLegend(0.7, 0.7, 0.9, 0.9);
        leg[i]->AddEntry(originGr[i], "Origin", "p");
        leg[i]->AddEntry(endGr[i], "End", "p");
    }
}

///////////////////////////////////////////////
/// \brief Draw origin and end of the track in a pad passed to the function
/// Note that GetOriginEnd has to be issued in advance
///
void TRestTrackEvent::DrawOriginEnd(TPad* pad, std::vector<TGraph*>& originGr, std::vector<TGraph*>& endGr,
                                    std::vector<TLegend*>& leg) {
    if (originGr.size() != 2 || endGr.size() != 2 || leg.size() != 2) return;

    for (int i = 0; i < 2; i++) {
        pad->cd(i + 1);
        if (originGr[i]) originGr[i]->Draw("LP");
        if (endGr[i]) endGr[i]->Draw("LP");
        if (leg[i]) leg[i]->Draw();
        pad->Update();
    }
}
