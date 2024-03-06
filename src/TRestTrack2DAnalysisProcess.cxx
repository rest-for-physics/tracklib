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
/// ### Observables
/// SetObservableValue("NTracksX", NTracksX);
/// SetObservableValue("NTracksY", NTracksY);
///
/// Map observables, there are the same observables for first and second tracks.
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
    if (LoadConfigFromFile(configFilename)) LoadDefaultConfig();
}

TRestTrack2DAnalysisProcess::~TRestTrack2DAnalysisProcess() { delete fOutputTrackEvent; }

void TRestTrack2DAnalysisProcess::LoadDefaultConfig() { SetTitle("Default config"); }

void TRestTrack2DAnalysisProcess::Initialize() {
    SetSectionName(this->ClassName());
    SetLibraryVersion(LIBRARY_VERSION);

    fInputTrackEvent = nullptr;
    fOutputTrackEvent = new TRestTrackEvent();
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

    /// Number of traks per type
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
    map<int, Double_t> XZ_YZ_SigmaZ;

    map<int, Double_t> XZ_YZ_SigmaXYBalance;
    map<int, Double_t> XZ_YZ_SigmaZBalance;

    map<int, Double_t> XZ_SkewZ;
    map<int, Double_t> YZ_SkewZ;

    /// Gauss sigma observables
    map<int, Double_t> XZ_GaussSigmaX;
    map<int, Double_t> XZ_GaussSigmaZ;
    map<int, Double_t> YZ_GaussSigmaY;
    map<int, Double_t> YZ_GaussSigmaZ;
    map<int, Double_t> XZ_YZ_GaussSigmaZ;

    map<int, Double_t> XZ_YZ_GaussSigmaXYBalance;
    map<int, Double_t> XZ_YZ_GaussSigmaZBalance;

    /// Espacial measuremets
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
    Double_t XZ_YZ_FirstSecondTracksDistanceSum;

    /// ---------------------------------------------------------- ///
    /// ------------------ COMPUTE OBSERVABLES ------------------- ///
    /// ---------------------------------------------------------- ///

    NTracksX = fTrackEvent->GetNumberOfTracks("X");
    NTracksY = fTrackEvent->GetNumberOfTracks("Y");

    /// Map oservables
    for (int tck = 0; tck < fTrackEvent->GetNumberOfTracks(); tck++) {
        if (!fTrackEvent->isTopLevel(tck)) continue;

        TRestTrack* t = fTrackEvent->GetTrack(tck);

        if (t->isXZ) {
            XZ_NHitsX[t->GetTrackID] = t->GetNumberOfHits();
            XZ_EnergyX[t->GetTrackID] = t->GetTrackEnergy();
            XZ_SigmaX[t->GetTrackID] = t->GetHits()->GetSigmaX();
            XZ_SigmaZ[t->GetTrackID] = t->GetHits()->GetSigmaZ2();
            XZ_GaussSigmaX[t->GetTrackID] = t->GetHits()->GetGaussSigmaX();
            XZ_GaussSigmaZ[t->GetTrackID] = t->GetHits()->GetGaussSigmaZ();
            XZ_LengthX[t->GetTrackID] = t->GetLength();
            XZ_VolumeX[t->GetTrackID] = t->GetVolume();
            XZ_MeanX[t->GetTrackID] = t->GetMeanPosition()->X();
            XZ_MeanZ[t->GetTrackID] = t->GetMeanPosition()->Z();
            XZ_SkewZ[t->GetTrackID] = t->GetHits()->GetSkewZ();

            YZ_NHitsY[t->GetTrackID] = 0;
            YZ_EnergyY[t->GetTrackID] = 0;
            YZ_SigmaY[t->GetTrackID] = 0;
            YZ_SigmaZ[t->GetTrackID] = 0;
            YZ_GaussSigmaY[t->GetTrackID] = 0;
            YZ_GaussSigmaZ[t->GetTrackID] = 0;
            YZ_LengthY[t->GetTrackID] = 0;
            YZ_VolumeY[t->GetTrackID] = 0;
            YZ_MeanY[t->GetTrackID] = 0;
            YZ_MeanZ[t->GetTrackID] = 0;
            YZ_SkewZ[t->GetTrackID] = 0;
        }

        if (t->isYZ) {
            XZ_NHitsX[t->GetTrackID] = 0;
            XZ_EnergyX[t->GetTrackID] = 0;
            XZ_SigmaX[t->GetTrackID] = 0;
            XZ_SigmaZ[t->GetTrackID] = 0;
            XZ_GaussSigmaX[t->GetTrackID] = 0;
            XZ_GaussSigmaZ[t->GetTrackID] = 0;
            XZ_LengthX[t->GetTrackID] = 0;
            XZ_VolumeX[t->GetTrackID] = 0;
            XZ_MeanX[t->GetTrackID] = 0;
            XZ_MeanZ[t->GetTrackID] = 0;
            XZ_SkewZ[t->GetTrackID] = 0;

            YZ_NHitsY[t->GetTrackID] = t->GetNumberOfHits();
            YZ_EnergyY[t->GetTrackID] = t->GetTrackEnergy();
            YZ_SigmaY[t->GetTrackID] = t->GetHits()->GetSigmaY();
            YZ_SigmaZ[t->GetTrackID] = t->GetHits()->GetSigmaZ2();
            YZ_GaussSigmaY[t->GetTrackID] = t->GetHits()->GetGaussSigmaY();
            YZ_GaussSigmaZ[t->GetTrackID] = t->GetHits()->GetGaussSigmaZ();
            YZ_LengthY[t->GetTrackID] = t->GetLength();
            YZ_VolumeY[t->GetTrackID] = t->GetVolume();
            YZ_MeanY[t->GetTrackID] = t->GetMeanPosition()->Y();
            YZ_MeanZ[t->GetTrackID] = t->GetMeanPosition()->Z();
            YZ_SkewZ[t->GetTrackID] = t->GetHits()->GetSkewZ();
        } else {
            XZ_EnergyX[t->GetTrackID] = 0;
            XZ_SigmaX[t->GetTrackID] = 0;
            XZ_SigmaZ[t->GetTrackID] = 0;
            XZ_GaussSigmaX[t->GetTrackID] = 0;
            XZ_GaussSigmaZ[t->GetTrackID] = 0;
            XZ_LengthX[t->GetTrackID] = 0;
            XZ_VolumeX[t->GetTrackID] = 0;
            XZ_MeanX[t->GetTrackID] = 0;
            XZ_MeanZ[t->GetTrackID] = 0;
            XZ_SkewZ[t->GetTrackID] = 0;

            YZ_EnergyY[t->GetTrackID] = 0;
            YZ_SigmaY[t->GetTrackID] = 0;
            YZ_SigmaZ[t->GetTrackID] = 0;
            YZ_GaussSigmaY[t->GetTrackID] = 0;
            YZ_GaussSigmaZ[t->GetTrackID] = 0;
            YZ_LengthY[t->GetTrackID] = 0;
            YZ_VolumeY[t->GetTrackID] = 0;
            YZ_MeanY[t->GetTrackID] = 0;
            YZ_MeanZ[t->GetTrackID] = 0;
            YZ_SkewZ[t->GetTrackID] = 0;
        }
    }

    /// Sort tracks by energy ///
    vector<pair<int, Double_t>> energiesX;
    vector<pair<int, Double_t>> energiesY;

    /// From map to vector
    map<int, Double_t>::iterator it;
    for (it = XZ_EnergyX.begin(); it != XZ_EnergyX.end(); it++) {
        energiesX.push_back(make_pair(it->first, it->second));
    }
    for (it = YZ_EnergyY.begin(); it != YZ_EnergyY.end(); it++) {
        energiesY.push_back(make_pair(it->first, it->second));
    }

    /// Sort the vector by decreasing order of its pair's second value
    sort(energiesX.begin(), energiesX.end(), sortByValueDescending);
    sort(energiesY.begin(), energiesY.end(), sortByValueDescending);

    /// Observables "balance", between max track in X and max track in Y
    for (int i = 0; i < min(NTracksX, NTracksY); i++) {
        XZ_YZ_SigmaXYBalance[i] =
            (XZ_SigmaX[energiesX[i].first()] - YZ_SigmaY[energiesY[i].first()]) /
            (XZ_SigmaX[energiesX[i].first()] + YZ_SigmaY[energiesY[i].first()]) XZ_YZ_SigmaZBalance[i] =
                (XZ_SigmaZ[energiesX[i].first()] - YZ_SigmaZ[energiesY[i].first()]) /
                (XZ_SigmaZ[energiesX[i].first()] + YZ_SigmaZ[energiesY[i].first()])
                    XZ_YZ_GaussSigmaXYBalance[i] =
                    (XZ_GaussSigmaX[energiesX[i].first()] - YZ_GaussSigmaY[energiesY[i].first()]) /
                    (XZ_GaussSigmaX[energiesX[i].first()] + YZ_GaussSigmaY[energiesY[i].first()])
                        XZ_YZ_GaussSigmaZBalance[i] =
                        (XZ_GaussSigmaZ[energiesX[i].first()] - YZ_GaussSigmaZ[energiesY[i].first()]) /
                        (XZ_GaussSigmaZ[energiesX[i].first()] + YZ_GaussSigmaZ[energiesY[i].first()])
    }

    /// ------------------------------------------------------------- ///
    /// ------------------ SET OBSERVABLES VALUES ------------------- ///
    /// ------------------------------------------------------------- ///

    SetObservableValue("NTracksX", NTracksX);
    SetObservableValue("NTracksY", NTracksY);

    // --- Map observables --- //
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
    SetObservableValue("MaxTrack_XZ_EnergyX", XZ_EnergyX[energiesX[0].first()]);
    SetObservableValue("MaxTrack_XZ_SigmaX", XZ_SigmaX[energiesX[0].first()]);
    SetObservableValue("MaxTrack_XZ_SigmaZ", XZ_SigmaZ[energiesX[0].first()]);
    SetObservableValue("MaxTrack_XZ_GaussSigmaX", XZ_GaussSigmaX[energiesX[0].first()]);
    SetObservableValue("MaxTrack_XZ_GaussSigmaZ", XZ_GaussSigmaZ[energiesX[0].first()]);
    SetObservableValue("MaxTrack_XZ_LengthX", XZ_LengthX[energiesX[0].first()]);
    SetObservableValue("MaxTrack_XZ_VolumeX", XZ_VolumeX[energiesX[0].first()]);
    SetObservableValue("MaxTrack_XZ_MeanX", XZ_MeanX[energiesX[0].first()]);
    SetObservableValue("MaxTrack_XZ_MeanZ", XZ_MeanZ[energiesX[0].first()]);
    SetObservableValue("MaxTrack_XZ_SkewZ", XZ_SkewZ[energiesX[0].first()]);

    SetObservableValue("MaxTrack_YZ_EnergyY", YZ_EnergyY[energiesY[0].first()]);
    SetObservableValue("MaxTrack_YZ_SigmaY", YZ_SigmaY[energiesY[0].first()]);
    SetObservableValue("MaxTrack_YZ_SigmaZ", YZ_SigmaZ[energiesY[0].first()]);
    SetObservableValue("MaxTrack_YZ_GaussSigmaY", YZ_GaussSigmaY[energiesY[0].first()]);
    SetObservableValue("MaxTrack_YZ_GaussSigmaZ", YZ_GaussSigmaZ[energiesY[0].first()]);
    SetObservableValue("MaxTrack_YZ_LengthY", YZ_LengthY[energiesY[0].first()]);
    SetObservableValue("MaxTrack_YZ_VolumeY", YZ_VolumeY[energiesY[0].first()]);
    SetObservableValue("MaxTrack_YZ_MeanY", YZ_MeanY[energiesY[0].first()]);
    SetObservableValue("MaxTrack_YZ_MeanZ", YZ_MeanZ[energiesY[0].first()]);
    SetObservableValue("MaxTrack_YZ_SkewZ", YZ_SkewZ[energiesY[0].first()]);

    SetObservableValue("MaxTrack_XZ_YZ_SigmaXYBalance", XZ_YZ_SigmaXYBalance[energiesY[0].first()]);
    SetObservableValue("MaxTrack_XZ_YZ_SigmaZBalance", XZ_YZ_SigmaZBalance[energiesY[0].first()]);
    SetObservableValue("MaxTrack_XZ_YZ_GaussSigmaXYBalance", XZ_YZ_GaussSigmaXYBalance[energiesY[0].first()]);
    SetObservableValue("MaxTrack_XZ_YZ_GaussSigmaZBalance", XZ_YZ_GaussSigmaZBalance[energiesY[0].first()]);

    // --- Second max track observables --- //
    SetObservableValue("SecondMaxTrack_XZ_EnergyX", XZ_EnergyX[energiesX[1].first()]);
    SetObservableValue("SecondMaxTrack_XZ_SigmaX", XZ_SigmaX[energiesX[1].first()]);
    SetObservableValue("SecondMaxTrack_XZ_SigmaZ", XZ_SigmaZ[energiesX[1].first()]);
    SetObservableValue("SecondMaxTrack_XZ_GaussSigmaX", XZ_GaussSigmaX[energiesX[1].first()]);
    SetObservableValue("SecondMaxTrack_XZ_GaussSigmaZ", XZ_GaussSigmaZ[energiesX[1].first()]);
    SetObservableValue("SecondMaxTrack_XZ_LengthX", XZ_LengthX[energiesX[1].first()]);
    SetObservableValue("SecondMaxTrack_XZ_VolumeX", XZ_VolumeX[energiesX[1].first()]);
    SetObservableValue("SecondMaxTrack_XZ_MeanX", XZ_MeanX[energiesX[1].first()]);
    SetObservableValue("SecondMaxTrack_XZ_MeanZ", XZ_MeanZ[energiesX[1].first()]);
    SetObservableValue("SecondMaxTrack_XZ_SkewZ", XZ_SkewZ[energiesX[1].first()]);

    SetObservableValue("SecondMaxTrack_YZ_EnergyY", YZ_EnergyY[energiesY[1].first()]);
    SetObservableValue("SecondMaxTrack_YZ_SigmaY", YZ_SigmaY[energiesY[1].first()]);
    SetObservableValue("SecondMaxTrack_YZ_SigmaZ", YZ_SigmaZ[energiesY[1].first()]);
    SetObservableValue("SecondMaxTrack_YZ_GaussSigmaY", YZ_GaussSigmaY[energiesY[1].first()]);
    SetObservableValue("SecondMaxTrack_YZ_GaussSigmaZ", YZ_GaussSigmaZ[energiesY[1].first()]);
    SetObservableValue("SecondMaxTrack_YZ_LengthY", YZ_LengthY[energiesY[1].first()]);
    SetObservableValue("SecondMaxTrack_YZ_VolumeY", YZ_VolumeY[energiesY[1].first()]);
    SetObservableValue("SecondMaxTrack_YZ_MeanY", YZ_MeanY[energiesY[1].first()]);
    SetObservableValue("SecondMaxTrack_YZ_MeanZ", YZ_MeanZ[energiesY[1].first()]);
    SetObservableValue("SecondMaxTrack_YZ_SkewZ", YZ_SkewZ[energiesY[1].first()]);

    SetObservableValue("SecondMaxTrack_XZ_YZ_SigmaXYBalance", XZ_YZ_SigmaXYBalance[energiesY[1].first()]);
    SetObservableValue("SecondMaxTrack_XZ_YZ_SigmaZBalance", XZ_YZ_SigmaZBalance[energiesY[1].first()]);
    SetObservableValue("SecondMaxTrack_XZ_YZ_GaussSigmaXYBalance",
                       XZ_YZ_GaussSigmaXYBalance[energiesY[1].first()]);
    SetObservableValue("SecondMaxTrack_XZ_YZ_GaussSigmaZBalance",
                       XZ_YZ_GaussSigmaZBalance[energiesY[1].first()]);

    return fTrackEvent;
}

void TRestTrack2DAnalysisProcess::EndProcess() {
    // Function to be executed once at the end of the process
    // (after all events have been processed)

    // Start by calling the EndProcess function of the abstract class.
    // Comment this if you don't want it.
    // TRestEventProcess::EndProcess();
}

void TRestTrack2DAnalysisProcess::InitFromConfigFile() {}