/*************************************************************************
 * This file is part of the REST software framework.                     *
 *                                                                       *
 * Copyright (C) 2016 GIFNA/TREX (University of Zaragoza)                *
 * For more information see http://gifna.unizar.es/trex                  *
 *                                                                       *
 * REST is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation, either version 3 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * REST is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the          *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have a copy of the GNU General Public License along with   *
 * REST in $REST_PATH/LICENSE.                                           *
 * If not, see http://www.gnu.org/licenses/.                             *
 * For the list of contributors see $REST_PATH/CREDITS.                  *
 *************************************************************************/

/////////////////////////////////////////////////////////////////////////////
///
/// Analysis process for 2D tracks, XZ-YZ.
/// For other types of tracks all observables are 0.
/// XZ_YZ means values from both types of tracks are involved.
///
/// ### Observables
/// "NTracks", fTrackEvent->GetNumberOfTracks());
/// "NTracksX", Number of XZ tracks
/// "NTracksY", Number of YZ tracks
/// "TotalEnergy", fTrackEvent->GetEnergy());
/// "XZ_TotalEnergyX", Energy in all XZ tracks
/// "YZ_TotalEnergyY", Energy in all YZ tracks
/// "XZ_YZ_MaxTrackEnergy", energiesX[0].second + energiesY[0].second (first tracks in XZ and YZ are added)
/// "XZ_YZ_MaxTrackEnergyPercentage", Percentage of first track (XZ YZ added) energy from total energy
/// "XZ_YZ_MaxTrackEnergyBalanceXY", Energy balance between X and Y in first track
/// "XZ_YZ_SecondMaxTrackEnergy", energiesX[1].second + energiesY[1].second (second tracks in XZ and YZ are
/// added)
/// "XZ_YZ_SecondMaxTrackEnergyPercentage", Percentage of second track (XZ YZ added) energy from total
/// energy
/// "XZ_YZ_SecondMaxTrackEnergyBalanceXY", Energy balance between X and Y in first track
///
/// "XZ_FirstSecondTracksDistanceXZ", Distance in XZ plane of mean positions of first two XZ tracks
/// "YZ_FirstSecondTracksDistanceYZ", Distance in YZ plane of mean positions of first two YZ tracks
/// "XZ_YZ_FirstSecondTracksDistanceSum", Root of squared sum of previous two
///
/// Map observables, there are the same observables for first and second tracks.
/// MaxTrack_... and SecondMaxTrack_...
/// Map observables have values for each track of each observable.
/// If XZ observable in a YZ track -> = 0 and viceversa.
///
/// SetObservableValue("Map_XZ_EnergyX", XZ_EnergyX);
/// SetObservableValue("Map_XZ_SigmaX", XZ_SigmaX);
/// SetObservableValue("Map_XZ_SigmaZ", XZ_SigmaZ);
/// SetObservableValue("Map_XZ_GaussSigmaX", XZ_GaussSigmaX);
/// SetObservableValue("Map_XZ_GaussSigmaZ", XZ_GaussSigmaZ);
/// SetObservableValue("Map_XZ_LengthX", XZ_LengthX);
/// SetObservableValue("Map_XZ_VolumeX", XZ_VolumeX);
/// SetObservableValue("Map_XZ_MeanX", XZ_MeanX);
/// SetObservableValue("Map_XZ_MeanZ", XZ_MeanZ);
/// SetObservableValue("Map_XZ_SkewZ", XZ_SkewZ);
///
/// SetObservableValue("Map_YZ_EnergyY", YZ_EnergyY);
/// SetObservableValue("Map_YZ_SigmaY", YZ_SigmaY);
/// SetObservableValue("Map_YZ_SigmaZ", YZ_SigmaZ);
/// SetObservableValue("Map_YZ_GaussSigmaY", YZ_GaussSigmaY);
/// SetObservableValue("Map_YZ_GaussSigmaZ", YZ_GaussSigmaZ);
/// SetObservableValue("Map_YZ_LengthY", YZ_LengthY);
/// SetObservableValue("Map_YZ_VolumeY", YZ_VolumeY);
/// SetObservableValue("Map_YZ_MeanY", YZ_MeanY);
/// SetObservableValue("Map_YZ_MeanZ", YZ_MeanZ);
/// SetObservableValue("Map_YZ_SkewZ", YZ_SkewZ);
///
/// SetObservableValue("Map_XZ_YZ_SigmaXYBalance", XZ_YZ_SigmaXYBalance);
/// SetObservableValue("Map_XZ_YZ_SigmaZBalance", XZ_YZ_SigmaZBalance);
/// SetObservableValue("Map_XZ_YZ_GaussSigmaXYBalance", XZ_YZ_GaussSigmaXYBalance);
/// SetObservableValue("Map_XZ_YZ_GaussSigmaZBalance", XZ_YZ_GaussSigmaZBalance);
///
///
///
///______________________________________________________________________________
///
/// RESTsoft - Software for Rare Event Searches with TPCs
///
/// History of developments:
///
/// 2024-March: Analysis for 2D tracks (XZ-YZ)
///             David Diez Ibanez
///
///
/// \class      TRestTrack2DAnalysisProcess
/// \author     David Diez Ibanez
///
///______________________________________________________________________________
///
/////////////////////////////////////////////////////////////////////////////

#include "TRestTrack2DAnalysisProcess.h"

using namespace std;

// Comparator function for sorting in descending order
bool sortByValueDescending(const std::pair<int, Double_t>& a, const std::pair<int, Double_t>& b) {
    return a.second > b.second;  // Change to < for ascending order
}

ClassImp(TRestTrack2DAnalysisProcess);

TRestTrack2DAnalysisProcess::TRestTrack2DAnalysisProcess() { Initialize(); }

TRestTrack2DAnalysisProcess::TRestTrack2DAnalysisProcess(const char* configFilename) {
    Initialize();
    if (LoadConfigFromFile(configFilename)) {
        LoadDefaultConfig();
    }
}

TRestTrack2DAnalysisProcess::~TRestTrack2DAnalysisProcess() { delete fTrackEvent; }

void TRestTrack2DAnalysisProcess::LoadDefaultConfig() { SetTitle("Default config"); }

void TRestTrack2DAnalysisProcess::Initialize() {
    SetSectionName(this->ClassName());
    SetLibraryVersion(LIBRARY_VERSION);

    fTrackEvent = new TRestTrackEvent();
}

void TRestTrack2DAnalysisProcess::LoadConfig(const string& configFilename, const string& name) {
    if (LoadConfigFromFile(configFilename, name)) LoadDefaultConfig();
}

void TRestTrack2DAnalysisProcess::InitProcess() {}

TRestEvent* TRestTrack2DAnalysisProcess::ProcessEvent(TRestEvent* inputEvent) {
    fTrackEvent = (TRestTrackEvent*)inputEvent;

    /// -------------------------------------------------- ///
    /// ------------------ OBSERVABLES ------------------- ///
    /// -------------------------------------------------- ///

    /// Energy by X and Y
    Double_t XZ_TotalEnergyX;
    Double_t YZ_TotalEnergyY;

    /// Number of tracks per type
    int NTracksX;
    int NTracksY;

    ////  ------ MAP OBSERVABLES ------ ////
    /// Energy per type of track
    map<int, Double_t> XZ_EnergyX;
    map<int, Double_t> YZ_EnergyY;

    /// Number of hits in the track
    map<int, int> XZ_NHitsX;
    map<int, int> YZ_NHitsY;

    /// Standard deviation observables
    map<int, Double_t> XZ_SigmaX;
    map<int, Double_t> XZ_SigmaZ;
    map<int, Double_t> YZ_SigmaY;
    map<int, Double_t> YZ_SigmaZ;

    map<int, Double_t> XZ_YZ_SigmaXYBalance;
    map<int, Double_t> XZ_YZ_SigmaZBalance;

    map<int, Double_t> XZ_SkewZ;
    map<int, Double_t> YZ_SkewZ;

    /// Gauss sigma observables
    map<int, Double_t> XZ_GaussSigmaX;
    map<int, Double_t> XZ_GaussSigmaZ;
    map<int, Double_t> YZ_GaussSigmaY;
    map<int, Double_t> YZ_GaussSigmaZ;

    map<int, Double_t> XZ_YZ_GaussSigmaXYBalance;
    map<int, Double_t> XZ_YZ_GaussSigmaZBalance;

    /// Spatial measurements
    map<int, Double_t> XZ_LengthX;
    map<int, Double_t> YZ_LengthY;

    map<int, Double_t> XZ_VolumeX;
    map<int, Double_t> YZ_VolumeY;

    map<int, Double_t> XZ_MeanX;
    map<int, Double_t> XZ_MeanZ;
    map<int, Double_t> YZ_MeanY;
    map<int, Double_t> YZ_MeanZ;

    /// Distances between biggest two tracks in energy
    Double_t XZ_FirstSecondTracksDistanceXZ;
    Double_t YZ_FirstSecondTracksDistanceYZ;

    /// Skew combining XZ and YZ max tracks
    Double_t MaxTrack_XZ_YZ_SigmaZ;
    Double_t MaxTrack_XZ_YZ_GaussSigmaZ;
    Double_t MaxTrack_XZ_YZ_SkewXY;
    Double_t MaxTrack_XZ_YZ_SkewZ;

    /// ---------------------------------------------------------- ///
    /// ------------------ COMPUTE OBSERVABLES ------------------- ///
    /// ---------------------------------------------------------- ///

    NTracksX = fTrackEvent->GetNumberOfTracks("X");
    NTracksY = fTrackEvent->GetNumberOfTracks("Y");

    /// Map observables
    for (int tck = 0; tck < fTrackEvent->GetNumberOfTracks(); tck++) {
        if (!fTrackEvent->isTopLevel(tck)) continue;

        TRestTrack* t = fTrackEvent->GetTrack(tck);

        if (t->isXZ()) {
            XZ_NHitsX[t->GetTrackID()] = t->GetNumberOfHits();
            XZ_EnergyX[t->GetTrackID()] = t->GetTrackEnergy();
            XZ_SigmaX[t->GetTrackID()] = t->GetHits()->GetSigmaX();
            XZ_SigmaZ[t->GetTrackID()] = t->GetHits()->GetSigmaZ2();
            XZ_GaussSigmaX[t->GetTrackID()] = t->GetHits()->GetGaussSigmaX();
            XZ_GaussSigmaZ[t->GetTrackID()] = t->GetHits()->GetGaussSigmaZ();
            XZ_LengthX[t->GetTrackID()] = t->GetLength();
            XZ_VolumeX[t->GetTrackID()] = t->GetVolume();
            XZ_MeanX[t->GetTrackID()] = t->GetMeanPosition().X();
            XZ_MeanZ[t->GetTrackID()] = t->GetMeanPosition().Z();
            XZ_SkewZ[t->GetTrackID()] = t->GetHits()->GetSkewZ();

            YZ_NHitsY[t->GetTrackID()] = 0;
            YZ_EnergyY[t->GetTrackID()] = 0;
            YZ_SigmaY[t->GetTrackID()] = 0;
            YZ_SigmaZ[t->GetTrackID()] = 0;
            YZ_GaussSigmaY[t->GetTrackID()] = 0;
            YZ_GaussSigmaZ[t->GetTrackID()] = 0;
            YZ_LengthY[t->GetTrackID()] = 0;
            YZ_VolumeY[t->GetTrackID()] = 0;
            YZ_MeanY[t->GetTrackID()] = 0;
            YZ_MeanZ[t->GetTrackID()] = 0;
            YZ_SkewZ[t->GetTrackID()] = 0;
        } else if (t->isYZ()) {
            XZ_NHitsX[t->GetTrackID()] = 0;
            XZ_EnergyX[t->GetTrackID()] = 0;
            XZ_SigmaX[t->GetTrackID()] = 0;
            XZ_SigmaZ[t->GetTrackID()] = 0;
            XZ_GaussSigmaX[t->GetTrackID()] = 0;
            XZ_GaussSigmaZ[t->GetTrackID()] = 0;
            XZ_LengthX[t->GetTrackID()] = 0;
            XZ_VolumeX[t->GetTrackID()] = 0;
            XZ_MeanX[t->GetTrackID()] = 0;
            XZ_MeanZ[t->GetTrackID()] = 0;
            XZ_SkewZ[t->GetTrackID()] = 0;

            YZ_NHitsY[t->GetTrackID()] = t->GetNumberOfHits();
            YZ_EnergyY[t->GetTrackID()] = t->GetTrackEnergy();
            YZ_SigmaY[t->GetTrackID()] = t->GetHits()->GetSigmaY();
            YZ_SigmaZ[t->GetTrackID()] = t->GetHits()->GetSigmaZ2();
            YZ_GaussSigmaY[t->GetTrackID()] = t->GetHits()->GetGaussSigmaY();
            YZ_GaussSigmaZ[t->GetTrackID()] = t->GetHits()->GetGaussSigmaZ();
            YZ_LengthY[t->GetTrackID()] = t->GetLength();
            YZ_VolumeY[t->GetTrackID()] = t->GetVolume();
            YZ_MeanY[t->GetTrackID()] = t->GetMeanPosition().Y();
            YZ_MeanZ[t->GetTrackID()] = t->GetMeanPosition().Z();
            YZ_SkewZ[t->GetTrackID()] = t->GetHits()->GetSkewZ();
        } else {
            XZ_EnergyX[t->GetTrackID()] = 0;
            XZ_SigmaX[t->GetTrackID()] = 0;
            XZ_SigmaZ[t->GetTrackID()] = 0;
            XZ_GaussSigmaX[t->GetTrackID()] = 0;
            XZ_GaussSigmaZ[t->GetTrackID()] = 0;
            XZ_LengthX[t->GetTrackID()] = 0;
            XZ_VolumeX[t->GetTrackID()] = 0;
            XZ_MeanX[t->GetTrackID()] = 0;
            XZ_MeanZ[t->GetTrackID()] = 0;
            XZ_SkewZ[t->GetTrackID()] = 0;

            YZ_EnergyY[t->GetTrackID()] = 0;
            YZ_SigmaY[t->GetTrackID()] = 0;
            YZ_SigmaZ[t->GetTrackID()] = 0;
            YZ_GaussSigmaY[t->GetTrackID()] = 0;
            YZ_GaussSigmaZ[t->GetTrackID()] = 0;
            YZ_LengthY[t->GetTrackID()] = 0;
            YZ_VolumeY[t->GetTrackID()] = 0;
            YZ_MeanY[t->GetTrackID()] = 0;
            YZ_MeanZ[t->GetTrackID()] = 0;
            YZ_SkewZ[t->GetTrackID()] = 0;
        }
    }

    /// Sort tracks by energy ///
    vector<pair<int, Double_t>> energiesX;
    vector<pair<int, Double_t>> energiesY;

    /// From map to vector
    map<int, Double_t>::iterator it;
    for (it = XZ_EnergyX.begin(); it != XZ_EnergyX.end(); it++) {
        energiesX.emplace_back(it->first, it->second);
    }
    for (it = YZ_EnergyY.begin(); it != YZ_EnergyY.end(); it++) {
        energiesY.emplace_back(it->first, it->second);
    }

    /// Sort the vector by decreasing order of its pair's second value
    sort(energiesX.begin(), energiesX.end(), sortByValueDescending);
    sort(energiesY.begin(), energiesY.end(), sortByValueDescending);

    /// Observables "balance", between max track in X and max track in Y
    for (int i = 0; i < min(NTracksX, NTracksY); i++) {
        XZ_YZ_SigmaXYBalance[i] = (XZ_SigmaX[energiesX[i].first] - YZ_SigmaY[energiesY[i].first]) /
                                  (XZ_SigmaX[energiesX[i].first] + YZ_SigmaY[energiesY[i].first]);
        XZ_YZ_SigmaZBalance[i] = (XZ_SigmaZ[energiesX[i].first] - YZ_SigmaZ[energiesY[i].first]) /
                                 (XZ_SigmaZ[energiesX[i].first] + YZ_SigmaZ[energiesY[i].first]);
        XZ_YZ_GaussSigmaXYBalance[i] =
            (XZ_GaussSigmaX[energiesX[i].first] - YZ_GaussSigmaY[energiesY[i].first]) /
            (XZ_GaussSigmaX[energiesX[i].first] + YZ_GaussSigmaY[energiesY[i].first]);
        XZ_YZ_GaussSigmaZBalance[i] =
            (XZ_GaussSigmaZ[energiesX[i].first] - YZ_GaussSigmaZ[energiesY[i].first]) /
            (XZ_GaussSigmaZ[energiesX[i].first] + YZ_GaussSigmaZ[energiesY[i].first]);
    }

    /// Distance between first two tracks
    if (fTrackEvent->GetNumberOfTracks() > 1) {
        Double_t dXz = 0, dxZ = 0, dYz = 0, dyZ = 0;

        dXz = abs(XZ_MeanX[energiesX[0].first] - XZ_MeanX[energiesX[1].first]);
        dxZ = abs(XZ_MeanZ[energiesX[0].first] - XZ_MeanZ[energiesX[1].first]);

        dYz = abs(YZ_MeanY[energiesY[0].first] - YZ_MeanY[energiesY[1].first]);
        dyZ = abs(YZ_MeanZ[energiesY[0].first] - YZ_MeanZ[energiesY[1].first]);

        XZ_FirstSecondTracksDistanceXZ = TMath::Sqrt(dXz * dXz + dxZ * dxZ);
        YZ_FirstSecondTracksDistanceYZ = TMath::Sqrt(dYz * dYz + dyZ * dyZ);
    } else {
        XZ_FirstSecondTracksDistanceXZ = 0;
        YZ_FirstSecondTracksDistanceYZ = 0;
    }

    /// Energy observables
    XZ_TotalEnergyX = 0;
    YZ_TotalEnergyY = 0;

    for (auto pair : energiesX) {
        XZ_TotalEnergyX += pair.second;
    }
    for (auto pair : energiesY) {
        YZ_TotalEnergyY += pair.second;
    }

    /// Skew and sigma Z observables combining max tracks in XZ and YZ
    TRestHits hits;
    TRestHits* hitsXZ = nullptr;
    TRestHits* hitsYZ = nullptr;
    if (fTrackEvent->GetMaxEnergyTrack("X")) hitsXZ = fTrackEvent->GetMaxEnergyTrack("X")->GetHits();
    if (fTrackEvent->GetMaxEnergyTrack("Y")) hitsYZ = fTrackEvent->GetMaxEnergyTrack("Y")->GetHits();

    auto hitsBoth = {hitsXZ, hitsYZ};

    for (auto arg : hitsBoth) {
        if (arg == nullptr) {
            continue;
        }
        for (int n = 0; n < int(arg->GetNumberOfHits()); n++) {
            // your code in the existing loop, replacing `hits` by `arg`
            Double_t eDep = arg->GetEnergy(n);
            Double_t x = arg->GetX(n);
            Double_t y = arg->GetY(n);
            Double_t z = arg->GetZ(n);
            auto time = arg->GetTime(n);
            auto type = arg->GetType(n);

            hits.AddHit({x, y, z}, eDep, time, type);
        }
    }

    MaxTrack_XZ_YZ_SigmaZ = hits.GetSigmaZ2();
    MaxTrack_XZ_YZ_GaussSigmaZ = hits.GetGaussSigmaZ();
    MaxTrack_XZ_YZ_SkewXY = hits.GetSkewXY();
    MaxTrack_XZ_YZ_SkewZ = hits.GetSkewZ();

    /// ------------------------------------------------------------- ///
    /// ------------------ SET OBSERVABLES VALUES ------------------- ///
    /// ------------------------------------------------------------- ///

    // --- Number of tracks and energy --- //
    SetObservableValue("NTracks", fTrackEvent->GetNumberOfTracks());
    SetObservableValue("NTracksX", NTracksX);
    SetObservableValue("NTracksY", NTracksY);
    SetObservableValue("TotalEnergy", fTrackEvent->GetEnergy());
    SetObservableValue("XZ_TotalEnergyX", XZ_TotalEnergyX);
    SetObservableValue("YZ_TotalEnergyY", YZ_TotalEnergyY);

    // --- Map observables --- //
    SetObservableValue("Map_XZ_NHitsX", XZ_NHitsX);
    SetObservableValue("Map_XZ_EnergyX", XZ_EnergyX);
    SetObservableValue("Map_XZ_SigmaX", XZ_SigmaX);
    SetObservableValue("Map_XZ_SigmaZ", XZ_SigmaZ);
    SetObservableValue("Map_XZ_GaussSigmaX", XZ_GaussSigmaX);
    SetObservableValue("Map_XZ_GaussSigmaZ", XZ_GaussSigmaZ);
    SetObservableValue("Map_XZ_LengthX", XZ_LengthX);
    SetObservableValue("Map_XZ_VolumeX", XZ_VolumeX);
    SetObservableValue("Map_XZ_MeanX", XZ_MeanX);
    SetObservableValue("Map_XZ_MeanZ", XZ_MeanZ);
    SetObservableValue("Map_XZ_SkewZ", XZ_SkewZ);

    SetObservableValue("Map_YZ_NHitsY", YZ_NHitsY);
    SetObservableValue("Map_YZ_EnergyY", YZ_EnergyY);
    SetObservableValue("Map_YZ_SigmaY", YZ_SigmaY);
    SetObservableValue("Map_YZ_SigmaZ", YZ_SigmaZ);
    SetObservableValue("Map_YZ_GaussSigmaY", YZ_GaussSigmaY);
    SetObservableValue("Map_YZ_GaussSigmaZ", YZ_GaussSigmaZ);
    SetObservableValue("Map_YZ_LengthY", YZ_LengthY);
    SetObservableValue("Map_YZ_VolumeY", YZ_VolumeY);
    SetObservableValue("Map_YZ_MeanY", YZ_MeanY);
    SetObservableValue("Map_YZ_MeanZ", YZ_MeanZ);
    SetObservableValue("Map_YZ_SkewZ", YZ_SkewZ);

    SetObservableValue("Map_XZ_YZ_SigmaXYBalance", XZ_YZ_SigmaXYBalance);
    SetObservableValue("Map_XZ_YZ_SigmaZBalance", XZ_YZ_SigmaZBalance);
    SetObservableValue("Map_XZ_YZ_GaussSigmaXYBalance", XZ_YZ_GaussSigmaXYBalance);
    SetObservableValue("Map_XZ_YZ_GaussSigmaZBalance", XZ_YZ_GaussSigmaZBalance);

    // --- Max track observables --- //

    // Copy the MaxTrack keys immediately after checking the vector

    SetObservableValue("MaxTrack_XZ_OK", !energiesX.empty());

    if (!energiesX.empty()) {
        int energiesX0FirstKey =
            energiesX[0].first;  // Declare Keys outside to avoid error when accessing "energiesX[0].first"...

        SetObservableValue("MaxTrack_XZ_NHitsX", XZ_NHitsX[energiesX0FirstKey]);
        SetObservableValue("MaxTrack_XZ_EnergyX", XZ_EnergyX[energiesX0FirstKey]);
        SetObservableValue("MaxTrack_XZ_SigmaX", XZ_SigmaX[energiesX0FirstKey]);
        SetObservableValue("MaxTrack_XZ_SigmaZ", XZ_SigmaZ[energiesX0FirstKey]);
        SetObservableValue("MaxTrack_XZ_GaussSigmaX", XZ_GaussSigmaX[energiesX0FirstKey]);
        SetObservableValue("MaxTrack_XZ_GaussSigmaZ", XZ_GaussSigmaZ[energiesX0FirstKey]);
        SetObservableValue("MaxTrack_XZ_LengthX", XZ_LengthX[energiesX0FirstKey]);
        SetObservableValue("MaxTrack_XZ_VolumeX", XZ_VolumeX[energiesX0FirstKey]);
        SetObservableValue("MaxTrack_XZ_MeanX", XZ_MeanX[energiesX0FirstKey]);
        SetObservableValue("MaxTrack_XZ_MeanZ", XZ_MeanZ[energiesX0FirstKey]);
        SetObservableValue("MaxTrack_XZ_SkewZ", XZ_SkewZ[energiesX0FirstKey]);
    } else {
        SetObservableValue("MaxTrack_XZ_NHitsX", 0.);
        SetObservableValue("MaxTrack_XZ_EnergyX", 0.);
        SetObservableValue("MaxTrack_XZ_SigmaX", 0.);
        SetObservableValue("MaxTrack_XZ_SigmaZ", 0.);
        SetObservableValue("MaxTrack_XZ_GaussSigmaX", 0.);
        SetObservableValue("MaxTrack_XZ_GaussSigmaZ", 0.);
        SetObservableValue("MaxTrack_XZ_LengthX", 0.);
        SetObservableValue("MaxTrack_XZ_VolumeX", 0.);
        SetObservableValue("MaxTrack_XZ_MeanX", 0.);
        SetObservableValue("MaxTrack_XZ_MeanZ", 0.);
        SetObservableValue("MaxTrack_XZ_SkewZ", 0.);
    }

    SetObservableValue("MaxTrack_YZ_OK", !energiesY.empty());

    if (!energiesY.empty()) {
        int energiesY0FirstKey = energiesY[0].first;

        SetObservableValue("MaxTrack_YZ_NHitsY", YZ_NHitsY[energiesY0FirstKey]);
        SetObservableValue("MaxTrack_YZ_EnergyY", YZ_EnergyY[energiesY0FirstKey]);
        SetObservableValue("MaxTrack_YZ_SigmaY", YZ_SigmaY[energiesY0FirstKey]);
        SetObservableValue("MaxTrack_YZ_SigmaZ", YZ_SigmaZ[energiesY0FirstKey]);
        SetObservableValue("MaxTrack_YZ_GaussSigmaY", YZ_GaussSigmaY[energiesY0FirstKey]);
        SetObservableValue("MaxTrack_YZ_GaussSigmaZ", YZ_GaussSigmaZ[energiesY0FirstKey]);
        SetObservableValue("MaxTrack_YZ_LengthY", YZ_LengthY[energiesY0FirstKey]);
        SetObservableValue("MaxTrack_YZ_VolumeY", YZ_VolumeY[energiesY0FirstKey]);
        SetObservableValue("MaxTrack_YZ_MeanY", YZ_MeanY[energiesY0FirstKey]);
        SetObservableValue("MaxTrack_YZ_MeanZ", YZ_MeanZ[energiesY0FirstKey]);
        SetObservableValue("MaxTrack_YZ_SkewZ", YZ_SkewZ[energiesY0FirstKey]);
    } else {
        SetObservableValue("MaxTrack_YZ_NHitsY", 0.);
        SetObservableValue("MaxTrack_YZ_EnergyY", 0.);
        SetObservableValue("MaxTrack_YZ_SigmaY", 0.);
        SetObservableValue("MaxTrack_YZ_SigmaZ", 0.);
        SetObservableValue("MaxTrack_YZ_GaussSigmaY", 0.);
        SetObservableValue("MaxTrack_YZ_GaussSigmaZ", 0.);
        SetObservableValue("MaxTrack_YZ_LengthY", 0.);
        SetObservableValue("MaxTrack_YZ_VolumeY", 0.);
        SetObservableValue("MaxTrack_YZ_MeanY", 0.);
        SetObservableValue("MaxTrack_YZ_MeanZ", 0.);
        SetObservableValue("MaxTrack_YZ_SkewZ", 0.);
    }

    SetObservableValue("MaxTrack_XZ_YZ_OK", !energiesY.empty() && !energiesX.empty());

    if (!energiesX.empty() && !energiesY.empty()) {
        Double_t energiesX0SecondKey = energiesX[0].second;
        Double_t energiesY0SecondKey = energiesY[0].second;

        SetObservableValue("MaxTrack_XZ_YZ_Energy", energiesX0SecondKey + energiesY0SecondKey);
        SetObservableValue("MaxTrack_XZ_YZ_MaxTrackEnergyPercentage",
                           (energiesX0SecondKey + energiesY0SecondKey) / fTrackEvent->GetEnergy());
        SetObservableValue("MaxTrack_XZ_YZ_EnergyBalanceXY", (energiesX0SecondKey - energiesY0SecondKey) /
                                                                 (energiesX0SecondKey + energiesY0SecondKey));

    } else {
        SetObservableValue("MaxTrack_XZ_YZ_Energy", 0.);
        SetObservableValue("MaxTrack_XZ_YZ_MaxTrackEnergyPercentage", 0.);
        SetObservableValue("MaxTrack_XZ_YZ_EnergyBalanceXY", 0.);
    }

    SetObservableValue("MaxTrack_XZ_YZ_SigmaXYBalance", XZ_YZ_SigmaXYBalance[0]);
    SetObservableValue("MaxTrack_XZ_YZ_SigmaZBalance", XZ_YZ_SigmaZBalance[0]);
    SetObservableValue("MaxTrack_XZ_YZ_GaussSigmaXYBalance", XZ_YZ_GaussSigmaXYBalance[0]);
    SetObservableValue("MaxTrack_XZ_YZ_GaussSigmaZBalance", XZ_YZ_GaussSigmaZBalance[0]);

    // --- Second max track observables --- //
    SetObservableValue("SecondMaxTrack_XZ_OK", energiesX.size() > 1);

    // Copy the SecondTrack keys immediately after checking the vector
    if (energiesX.size() > 1) {
        int energiesX1FirstKey =
            energiesX[1].first;  // Declare Keys outside to avoid error when accessing "energiesX[1].first"...

        SetObservableValue("SecondMaxTrack_XZ_NHitsX", XZ_NHitsX[energiesX1FirstKey]);
        SetObservableValue("SecondMaxTrack_XZ_EnergyX", XZ_EnergyX[energiesX1FirstKey]);
        SetObservableValue("SecondMaxTrack_XZ_SigmaX", XZ_SigmaX[energiesX1FirstKey]);
        SetObservableValue("SecondMaxTrack_XZ_SigmaZ", XZ_SigmaZ[energiesX1FirstKey]);
        SetObservableValue("SecondMaxTrack_XZ_GaussSigmaX", XZ_GaussSigmaX[energiesX1FirstKey]);
        SetObservableValue("SecondMaxTrack_XZ_GaussSigmaZ", XZ_GaussSigmaZ[energiesX1FirstKey]);
        SetObservableValue("SecondMaxTrack_XZ_LengthX", XZ_LengthX[energiesX1FirstKey]);
        SetObservableValue("SecondMaxTrack_XZ_VolumeX", XZ_VolumeX[energiesX1FirstKey]);
        SetObservableValue("SecondMaxTrack_XZ_MeanX", XZ_MeanX[energiesX1FirstKey]);
        SetObservableValue("SecondMaxTrack_XZ_MeanZ", XZ_MeanZ[energiesX1FirstKey]);
        SetObservableValue("SecondMaxTrack_XZ_SkewZ", XZ_SkewZ[energiesX1FirstKey]);
    } else {
        SetObservableValue("SecondMaxTrack_XZ_NHitsX", 0.);
        SetObservableValue("SecondMaxTrack_XZ_EnergyX", 0.);
        SetObservableValue("SecondMaxTrack_XZ_SigmaX", 0.);
        SetObservableValue("SecondMaxTrack_XZ_SigmaZ", 0.);
        SetObservableValue("SecondMaxTrack_XZ_GaussSigmaX", 0.);
        SetObservableValue("SecondMaxTrack_XZ_GaussSigmaZ", 0.);
        SetObservableValue("SecondMaxTrack_XZ_LengthX", 0.);
        SetObservableValue("SecondMaxTrack_XZ_VolumeX", 0.);
        SetObservableValue("SecondMaxTrack_XZ_MeanX", 0.);
        SetObservableValue("SecondMaxTrack_XZ_MeanZ", 0.);
        SetObservableValue("SecondMaxTrack_XZ_SkewZ", 0.);
    }

    SetObservableValue("SecondMaxTrack_YZ_OK", energiesY.size() > 1);

    if (!energiesY.empty()) {
        int energiesY1FirstKey = energiesY[1].first;

        SetObservableValue("SecondMaxTrack_YZ_NHitsY", YZ_NHitsY[energiesY1FirstKey]);
        SetObservableValue("SecondMaxTrack_YZ_EnergyY", YZ_EnergyY[energiesY1FirstKey]);
        SetObservableValue("SecondMaxTrack_YZ_SigmaY", YZ_SigmaY[energiesY1FirstKey]);
        SetObservableValue("SecondMaxTrack_YZ_SigmaZ", YZ_SigmaZ[energiesY1FirstKey]);
        SetObservableValue("SecondMaxTrack_YZ_GaussSigmaY", YZ_GaussSigmaY[energiesY1FirstKey]);
        SetObservableValue("SecondMaxTrack_YZ_GaussSigmaZ", YZ_GaussSigmaZ[energiesY1FirstKey]);
        SetObservableValue("SecondMaxTrack_YZ_LengthY", YZ_LengthY[energiesY1FirstKey]);
        SetObservableValue("SecondMaxTrack_YZ_VolumeY", YZ_VolumeY[energiesY1FirstKey]);
        SetObservableValue("SecondMaxTrack_YZ_MeanY", YZ_MeanY[energiesY1FirstKey]);
        SetObservableValue("SecondMaxTrack_YZ_MeanZ", YZ_MeanZ[energiesY1FirstKey]);
        SetObservableValue("SecondMaxTrack_YZ_SkewZ", YZ_SkewZ[energiesY1FirstKey]);
    } else {
        SetObservableValue("SecondMaxTrack_YZ_NHitsY", 0.);
        SetObservableValue("SecondMaxTrack_YZ_EnergyY", 0.);
        SetObservableValue("SecondMaxTrack_YZ_SigmaY", 0.);
        SetObservableValue("SecondMaxTrack_YZ_SigmaZ", 0.);
        SetObservableValue("SecondMaxTrack_YZ_GaussSigmaY", 0.);
        SetObservableValue("SecondMaxTrack_YZ_GaussSigmaZ", 0.);
        SetObservableValue("SecondMaxTrack_YZ_LengthY", 0.);
        SetObservableValue("SecondMaxTrack_YZ_VolumeY", 0.);
        SetObservableValue("SecondMaxTrack_YZ_MeanY", 0.);
        SetObservableValue("SecondMaxTrack_YZ_MeanZ", 0.);
        SetObservableValue("SecondMaxTrack_YZ_SkewZ", 0.);
    }

    SetObservableValue("SecondMaxTrack_XZ_YZ_SigmaXYBalance", XZ_YZ_SigmaXYBalance[1]);
    SetObservableValue("SecondMaxTrack_XZ_YZ_SigmaZBalance", XZ_YZ_SigmaZBalance[1]);
    SetObservableValue("SecondMaxTrack_XZ_YZ_GaussSigmaXYBalance", XZ_YZ_GaussSigmaXYBalance[1]);
    SetObservableValue("SecondMaxTrack_XZ_YZ_GaussSigmaZBalance", XZ_YZ_GaussSigmaZBalance[1]);

    SetObservableValue("SecondMaxTrack_XZ_YZ_OK", energiesY.size() > 1 && energiesX.size() > 1);

    if (energiesY.size() > 1 && energiesX.size() > 1) {
        Double_t energiesX1SecondKey = energiesX[1].second;
        Double_t energiesY1SecondKey = energiesY[1].second;

        if (fTrackEvent->GetNumberOfTracks() > 2) {
            SetObservableValue("SecondMaxTrack_XZ_YZ_Energy", energiesX1SecondKey + energiesY1SecondKey);
            SetObservableValue("SecondMaxTrack_XZ_YZ_EnergyPercentage",
                               (energiesX1SecondKey + energiesY1SecondKey) / fTrackEvent->GetEnergy());
            SetObservableValue(
                "SecondMaxTrack_XZ_YZ_EnergyBalanceXY",
                (energiesX1SecondKey - energiesY1SecondKey) / (energiesX1SecondKey + energiesY1SecondKey));
        } else {
            SetObservableValue("SecondMaxTrack_XZ_YZ_Energy", 0.);
            SetObservableValue("SecondMaxTrack_XZ_YZ_EnergyPercentage", 0.);
            SetObservableValue("SecondMaxTrack_XZ_YZ_EnergyBalanceXY", 0.);
        }
    } else {
        SetObservableValue("SecondMaxTrack_XZ_YZ_Energy", 0.);
        SetObservableValue("SecondMaxTrack_XZ_YZ_EnergyPercentage", 0.);
        SetObservableValue("SecondMaxTrack_XZ_YZ_EnergyBalanceXY", 0.);
    }

    // --- Distance observables between first two tracks --- //
    SetObservableValue("XZ_FirstSecondTracksDistanceXZ", XZ_FirstSecondTracksDistanceXZ);
    SetObservableValue("YZ_FirstSecondTracksDistanceYZ", YZ_FirstSecondTracksDistanceYZ);
    SetObservableValue("XZ_YZ_FirstSecondTracksDistanceSum",
                       TMath::Sqrt(XZ_FirstSecondTracksDistanceXZ * XZ_FirstSecondTracksDistanceXZ +
                                   YZ_FirstSecondTracksDistanceYZ * YZ_FirstSecondTracksDistanceYZ));

    // --- Observables merging max tracks XZ and YZ --- //
    SetObservableValue("MaxTrack_XZ_YZ_SigmaZ", MaxTrack_XZ_YZ_SigmaZ);
    SetObservableValue("MaxTrack_XZ_YZ_GaussSigmaZ", MaxTrack_XZ_YZ_GaussSigmaZ);
    SetObservableValue("MaxTrack_XZ_YZ_SkewXY", MaxTrack_XZ_YZ_SkewXY);
    SetObservableValue("MaxTrack_XZ_YZ_SkewZ", MaxTrack_XZ_YZ_SkewZ);

    return fTrackEvent;
}

void TRestTrack2DAnalysisProcess::EndProcess() {}

void TRestTrack2DAnalysisProcess::InitFromConfigFile() {}
