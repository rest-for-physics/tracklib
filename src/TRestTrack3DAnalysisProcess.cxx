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
/// Analysis process for 3D tracks, XYZ.
/// For other types of tracks all observables are 0.
///
/// ### Observables
/// "NTracks", fTrackEvent->GetNumberOfTracks());
/// "TotalEnergy", fTrackEvent->GetEnergy());
/// "XYZ_MaxTrackEnergyPercentage", Percentage of first track energy from total energy
/// "XYZ_SecondMaxTrackEnergyPercentage", Percentage of second track energy from total energy
/// "XYZ_FirstSecondTracksDistance", Distance in XYZ of mean positions of first two XYZ tracks
///
/// Map observables, there are the same observables for first and second tracks.
/// MaxTrack_... and SecondMaxTrack_...
/// Map observables have values for each track of each observable.
/// Only for XYZ tracks, if not = 0
///
/// SetObservableValue("Map_XYZ_NHits", XYZ_NHits);
/// SetObservableValue("Map_XYZ_Energy", XYZ_Energy);
/// SetObservableValue("Map_XYZ_SigmaX", XYZ_SigmaX);
/// SetObservableValue("Map_XYZ_SigmaY", XYZ_SigmaY);
/// SetObservableValue("Map_XYZ_SigmaZ", XYZ_SigmaZ);
/// SetObservableValue("Map_XYZ_GaussSigmaX", XYZ_GaussSigmaX);
/// SetObservableValue("Map_XYZ_GaussSigmaY", XYZ_GaussSigmaY);
/// SetObservableValue("Map_XYZ_GaussSigmaZ", XYZ_GaussSigmaZ);
/// SetObservableValue("Map_XYZ_Length", XYZ_Length);
/// SetObservableValue("Map_XYZ_Volume", XYZ_Volume);
/// SetObservableValue("Map_XYZ_MeanX", XYZ_MeanX);
/// SetObservableValue("Map_XYZ_MeanY", XYZ_MeanY);
/// SetObservableValue("Map_XYZ_MeanZ", XYZ_MeanZ);
/// SetObservableValue("Map_XYZ_SkewXY", XYZ_SkewXY);
/// SetObservableValue("Map_XYZ_SkewZ", XYZ_SkewZ);
///
///
///
///______________________________________________________________________________
///
/// RESTsoft - Software for Rare Event Searches with TPCs
///
/// History of developments:
///
/// 2024-March: Analysis for 3D tracks (XYZ)
///             David Diez Ibanez
///
///
/// \class      TRestTrack3DAnalysisProcess
/// \author     David Diez Ibanez
///
///______________________________________________________________________________
///
/////////////////////////////////////////////////////////////////////////////

#include "TRestTrack3DAnalysisProcess.h"
using namespace std;

// Comparator function for sorting in descending order
bool sortByValueDescending(const std::pair<int, Double_t>& a, const std::pair<int, Double_t>& b) {
    return a.second > b.second;  // Change to < for ascending order
}

ClassImp(TRestTrack3DAnalysisProcess);

TRestTrack3DAnalysisProcess::TRestTrack3DAnalysisProcess() { Initialize(); }

TRestTrack3DAnalysisProcess::TRestTrack3DAnalysisProcess(const char* configFilename) {
    Initialize();
    if (LoadConfigFromFile(configFilename)) LoadDefaultConfig();
}

TRestTrack3DAnalysisProcess::~TRestTrack3DAnalysisProcess() { delete fTrackEvent; }

void TRestTrack3DAnalysisProcess::LoadDefaultConfig() { SetTitle("Default config"); }

void TRestTrack3DAnalysisProcess::Initialize() {
    SetSectionName(this->ClassName());
    SetLibraryVersion(LIBRARY_VERSION);

    fTrackEvent = new TRestTrackEvent();
}

void TRestTrack3DAnalysisProcess::LoadConfig(const string& configFilename, const string& name) {
    if (LoadConfigFromFile(configFilename, name)) LoadDefaultConfig();
}

void TRestTrack3DAnalysisProcess::InitProcess() {}

TRestEvent* TRestTrack3DAnalysisProcess::ProcessEvent(TRestEvent* inputEvent) {
    fTrackEvent = (TRestTrackEvent*)inputEvent;

    /// -------------------------------------------------- ///
    /// ------------------ OBSERVABLES ------------------- ///
    /// -------------------------------------------------- ///

    ////  ------ MAP OBSERVABLES ------ ////
    /// Energy per track
    map<int, Double_t> XYZ_Energy;

    /// Number of hits in the track
    map<int, int> XYZ_NHits;

    /// Standard deviation observables
    map<int, Double_t> XYZ_SigmaX;
    map<int, Double_t> XYZ_SigmaY;
    map<int, Double_t> XYZ_SigmaZ;

    /// Skew observables
    map<int, Double_t> XYZ_SkewXY;
    map<int, Double_t> XYZ_SkewZ;

    /// Gauss sigma observables
    map<int, Double_t> XYZ_GaussSigmaX;
    map<int, Double_t> XYZ_GaussSigmaY;
    map<int, Double_t> XYZ_GaussSigmaZ;

    /// Espacial measuremets
    map<int, Double_t> XYZ_Length;

    map<int, Double_t> XYZ_Volume;

    map<int, Double_t> XYZ_MeanX;
    map<int, Double_t> XYZ_MeanY;
    map<int, Double_t> XYZ_MeanZ;

    /// Distance between biggest two tracks in energy
    Double_t XYZ_FirstSecondTracksDistance;

    /// ---------------------------------------------------------- ///
    /// ------------------ COMPUTE OBSERVABLES ------------------- ///
    /// ---------------------------------------------------------- ///

    /// Map oservables
    for (int tck = 0; tck < fTrackEvent->GetNumberOfTracks(); tck++) {
        if (!fTrackEvent->isTopLevel(tck)) continue;

        TRestTrack* t = fTrackEvent->GetTrack(tck);

        if (t->isXYZ()) {
            XYZ_NHits[t->GetTrackID()] = t->GetNumberOfHits();
            XYZ_Energy[t->GetTrackID()] = t->GetTrackEnergy();
            XYZ_SigmaX[t->GetTrackID()] = t->GetHits()->GetSigmaX();
            XYZ_SigmaY[t->GetTrackID()] = t->GetHits()->GetSigmaY();
            XYZ_SigmaZ[t->GetTrackID()] = t->GetHits()->GetSigmaZ2();
            XYZ_GaussSigmaX[t->GetTrackID()] = t->GetHits()->GetGaussSigmaX();
            XYZ_GaussSigmaY[t->GetTrackID()] = t->GetHits()->GetGaussSigmaY();
            XYZ_GaussSigmaZ[t->GetTrackID()] = t->GetHits()->GetGaussSigmaZ();
            XYZ_Length[t->GetTrackID()] = t->GetLength();
            XYZ_Volume[t->GetTrackID()] = t->GetVolume();
            XYZ_MeanX[t->GetTrackID()] = t->GetMeanPosition().X();
            XYZ_MeanY[t->GetTrackID()] = t->GetMeanPosition().Y();
            XYZ_MeanZ[t->GetTrackID()] = t->GetMeanPosition().Z();
            XYZ_SkewXY[t->GetTrackID()] = t->GetHits()->GetSkewXY();
            XYZ_SkewZ[t->GetTrackID()] = t->GetHits()->GetSkewZ();
        } else {
            XYZ_NHits[t->GetTrackID()] = 0;
            XYZ_Energy[t->GetTrackID()] = 0;
            XYZ_SigmaX[t->GetTrackID()] = 0;
            XYZ_SigmaY[t->GetTrackID()] = 0;
            XYZ_SigmaZ[t->GetTrackID()] = 0;
            XYZ_GaussSigmaX[t->GetTrackID()] = 0;
            XYZ_GaussSigmaY[t->GetTrackID()] = 0;
            XYZ_GaussSigmaZ[t->GetTrackID()] = 0;
            XYZ_Length[t->GetTrackID()] = 0;
            XYZ_Volume[t->GetTrackID()] = 0;
            XYZ_MeanX[t->GetTrackID()] = 0;
            XYZ_MeanY[t->GetTrackID()] = 0;
            XYZ_MeanZ[t->GetTrackID()] = 0;
            XYZ_SkewXY[t->GetTrackID()] = 0;
            XYZ_SkewZ[t->GetTrackID()] = 0;
        }
    }

    /// Sort tracks by energy ///
    vector<pair<int, Double_t>> energies;

    /// From map to vector
    map<int, Double_t>::iterator it;
    for (it = XYZ_Energy.begin(); it != XYZ_Energy.end(); it++) {
        energies.push_back(make_pair(it->first, it->second));
    }

    /// Sort the vector by decreasing order of its pair's second value
    sort(energies.begin(), energies.end(), sortByValueDescending);

    /// Distance between first two tracks
    Double_t dX = 0, dY = 0, dZ = 0;

    dX = abs(XYZ_MeanX[energies[0].first] - XYZ_MeanX[energiesX[1].first]);
    dY = abs(XYZ_MeanY[energies[0].first] - XYZ_MeanY[energiesX[1].first]);
    dZ = abs(XYZ_MeanZ[energies[0].first] - XYZ_MeanZ[energiesX[1].first]);

    XYZ_FirstSecondTracksDistance = TMath::Sqrt(dX * dX + dY * dY + dZ * dZ);

    /// ------------------------------------------------------------- ///
    /// ------------------ SET OBSERVABLES VALUES ------------------- ///
    /// ------------------------------------------------------------- ///

    // --- Number of tracks and energy --- //
    SetObservableValue("NTracks", fTrackEvent->GetNumberOfTracks());
    SetObservableValue("TotalEnergy", fTrackEvent->GetEnergy());

    // --- Map observables --- //
    SetObservableValue("Map_XYZ_NHits", XYZ_NHits);
    SetObservableValue("Map_XYZ_Energy", XYZ_Energy);
    SetObservableValue("Map_XYZ_SigmaX", XYZ_SigmaX);
    SetObservableValue("Map_XYZ_SigmaY", XYZ_SigmaY);
    SetObservableValue("Map_XYZ_SigmaZ", XYZ_SigmaZ);
    SetObservableValue("Map_XYZ_GaussSigmaX", XYZ_GaussSigmaX);
    SetObservableValue("Map_XYZ_GaussSigmaY", XYZ_GaussSigmaY);
    SetObservableValue("Map_XYZ_GaussSigmaZ", XYZ_GaussSigmaZ);
    SetObservableValue("Map_XYZ_Length", XYZ_Length);
    SetObservableValue("Map_XYZ_Volume", XYZ_Volume);
    SetObservableValue("Map_XYZ_MeanX", XYZ_MeanX);
    SetObservableValue("Map_XYZ_MeanY", XYZ_MeanY);
    SetObservableValue("Map_XYZ_MeanZ", XYZ_MeanZ);
    SetObservableValue("Map_XYZ_SkewXY", XYZ_SkewXY);
    SetObservableValue("Map_XYZ_SkewZ", XYZ_SkewZ);

    // --- Max track observables --- //
    SetObservableValue("MaxTrack_XYZ_NHits", XYZ_NHits[energies[0].first]);
    SetObservableValue("MaxTrack_XYZ_Energy", XYZ_EnergyX[energies[0].first]);
    SetObservableValue("MaxTrack_XYZ_SigmaX", XYZ_SigmaX[energies[0].first]);
    SetObservableValue("MaxTrack_XYZ_SigmaY", XYZ_SigmaY[energies[0].first]);
    SetObservableValue("MaxTrack_XYZ_SigmaZ", XYZ_SigmaZ[energies[0].first]);
    SetObservableValue("MaxTrack_XYZ_GaussSigmaX", XYZ_GaussSigmaX[energies[0].first]);
    SetObservableValue("MaxTrack_XYZ_GaussSigmaY", XYZ_GaussSigmaY[energies[0].first]);
    SetObservableValue("MaxTrack_XYZ_GaussSigmaZ", XYZ_GaussSigmaZ[energies[0].first]);
    SetObservableValue("MaxTrack_XYZ_Length", XYZ_Length[energies[0].first]);
    SetObservableValue("MaxTrack_XYZ_Volume", XYZ_Volume[energies[0].first]);
    SetObservableValue("MaxTrack_XYZ_MeanX", XYZ_MeanX[energies[0].first]);
    SetObservableValue("MaxTrack_XYZ_MeanY", XYZ_MeanY[energies[0].first]);
    SetObservableValue("MaxTrack_XYZ_MeanZ", XYZ_MeanZ[energies[0].first]);
    SetObservableValue("MaxTrack_XYZ_SkewZ", XYZ_SkewXY[energies[0].first]);
    SetObservableValue("MaxTrack_XYZ_SkewZ", XYZ_SkewZ[energies[0].first]);

    SetObservableValue("MaxTrack_XZ_YZ_MaxTrackEnergyPercentage",
                       (energies[0].second) / fTrackEvent->GetEnergy());

    // --- Second max track observables --- //
    SetObservableValue("SecondMaxTrack_XYZ_NHits", XYZ_NHits[energies[1].first]);
    SetObservableValue("SecondMaxTrack_XYZ_Energy", XYZ_EnergyX[energies[1].first]);
    SetObservableValue("SecondMaxTrack_XYZ_SigmaX", XYZ_SigmaX[energies[1].first]);
    SetObservableValue("SecondMaxTrack_XYZ_SigmaY", XYZ_SigmaY[energies[1].first]);
    SetObservableValue("SecondMaxTrack_XYZ_SigmaZ", XYZ_SigmaZ[energies[1].first]);
    SetObservableValue("SecondMaxTrack_XYZ_GaussSigmaX", XYZ_GaussSigmaX[energies[1].first]);
    SetObservableValue("SecondMaxTrack_XYZ_GaussSigmaY", XYZ_GaussSigmaY[energies[1].first]);
    SetObservableValue("SecondMaxTrack_XYZ_GaussSigmaZ", XYZ_GaussSigmaZ[energies[1].first]);
    SetObservableValue("SecondMaxTrack_XYZ_Length", XYZ_Length[energies[1].first]);
    SetObservableValue("SecondMaxTrack_XYZ_Volume", XYZ_Volume[energies[1].first]);
    SetObservableValue("SecondMaxTrack_XYZ_MeanX", XYZ_MeanX[energies[1].first]);
    SetObservableValue("SecondMaxTrack_XYZ_MeanY", XYZ_MeanY[energies[1].first]);
    SetObservableValue("SecondMaxTrack_XYZ_MeanZ", XYZ_MeanZ[energies[1].first]);
    SetObservableValue("SecondMaxTrack_XYZ_SkewZ", XYZ_SkewXY[energies[1].first]);
    SetObservableValue("SecondMaxTrack_XYZ_SkewZ", XYZ_SkewZ[energies[1].first]);

    SetObservableValue("SecondMaxTrack_XZ_YZ_MaxTrackEnergyPercentage",
                       (energies[1].second) / fTrackEvent->GetEnergy());

    // --- Distance obsevables between first two tracks --- //
    SetObservableValue("XYZ_FirstSecondTracksDistance", XYZ_FirstSecondTracksDistance);

    return fTrackEvent;
}

void TRestTrack3DAnalysisProcess::EndProcess() {
    // Function to be executed once at the end of the process
    // (after all events have been processed)

    // Start by calling the EndProcess function of the abstract class.
    // Comment this if you don't want it.
    // TRestEventProcess::EndProcess();
}

void TRestTrack3DAnalysisProcess::InitFromConfigFile() {}
