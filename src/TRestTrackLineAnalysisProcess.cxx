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

//////////////////////////////////////////////////////////////////////////
/// The TRestTrackLineAnalysis process is meant to analyze linear tracks,
/// such as alpha tracks. It should retreive the information after performing
/// the track reduction with the hits arranged following the minimum path. Which
/// can be archieved by performing TRestTrackPathMinimizationProcess after
/// TRestTrackReductionProcess; alternatively it can be achieved performing
/// TRestTrackLinearization process. For the time being, this process only works when
/// XZ and YZ are present in the readout. Moreover, the hits have to be stored
/// following the minimum path. It assumes single track events, otherwise, the most
/// energetic track is used in the analysis.
/// Several parameters of the alphaTrack are extracted such as origin, end,
/// length, energy and downwards (bool).
///
/// ### Parameters
/// None
///
/// ### Observables
/// * **trackBalanceXZ**: Track balance between the most energetic track and all tracks in the XZ projection
/// * **trackBalanceYZ**: Track balance between the most energetic track and all tracks in the YZ projection
/// * **originX**: Track origin in the X cordinate
/// * **originY**: Track origin in the Y cordinate
/// * **originZ**: Track origin in the Z cordinate
/// * **endX**: Track end in the X cordinate
/// * **endY**: Track end in the Y cordinate
/// * **endZ**: Track end in the Z cordinate
/// * **length**: Track length in mm
/// * **angle**: Track polar angle in radians
/// * **downwards**: (bool) true if the track direction is downwards, false otherwise
/// * **totalEnergy**: Energy of the track
///
/// ### Examples
/// \code
///            <addProcess type="TRestTrackLineAnalysisProcess" name="alphaTrackAna" value="ON"
///            verboseLevel="silent" >
///                   <observable name="trackBalanceXZ" value="ON" />
///                   <observable name="trackBalanceYZ" value="ON" />
///                   <observable name="originX" value="ON" />
///                   <observable name="originY" value="ON" />
///                   <observable name="originZ" value="ON" />
///                   <observable name="endX" value="ON" />
///                   <observable name="endY" value="ON" />
///                   <observable name="endZ" value="ON" />
///                   <observable name="length" value="ON" />
///                   <observable name="angle" value="ON" />
///                   <observable name="downwards" type="bool" value="ON" />
///                   <observable name="totalEnergy" value="ON" />
/// 		</addProcess>
/// \endcode
///
///--------------------------------------------------------------------------
///
/// REST-for-Physics - Software for Rare Event Searches Toolkit
///
/// History of developments:
///
/// 2022-January First implementation
///
///              JuanAn Garcia
///
/// \class TRestTrackLineAnalysisProcess
/// \author: JuanAn Garcia   e-mail: juanangp@unizar.es
///
/// <hr>
///

#include "TRestTrackLineAnalysisProcess.h"
#include "TRestTrackReductionProcess.h"
using namespace std;

ClassImp(TRestTrackLineAnalysisProcess);

///////////////////////////////////////////////
/// \brief Default constructor
///
TRestTrackLineAnalysisProcess::TRestTrackLineAnalysisProcess() { Initialize(); }

///////////////////////////////////////////////
/// \brief Default destructor
///
TRestTrackLineAnalysisProcess::~TRestTrackLineAnalysisProcess() { delete fOutTrackEvent; }

///////////////////////////////////////////////
/// \brief Function to initialize input/output event members and define
/// the section name
///
void TRestTrackLineAnalysisProcess::Initialize() {
    SetSectionName(this->ClassName());
    SetLibraryVersion(LIBRARY_VERSION);

    fTrackEvent = nullptr;
    fOutTrackEvent = new TRestTrackEvent();
}

///////////////////////////////////////////////
/// \brief Process initialization.
/// Nothing to do...
///
void TRestTrackLineAnalysisProcess::InitProcess() {}

///////////////////////////////////////////////
/// \brief The main processing event function
///
TRestEvent* TRestTrackLineAnalysisProcess::ProcessEvent(TRestEvent* evInput) {
    fTrackEvent = (TRestTrackEvent*)evInput;
    fOutTrackEvent->SetEventInfo(fTrackEvent);

    for (int t = 0; t < fTrackEvent->GetNumberOfTracks(); t++)
        fOutTrackEvent->AddTrack(fTrackEvent->GetTrack(t));

    Int_t nTracksX = fTrackEvent->GetNumberOfTracks("X");
    Int_t nTracksY = fTrackEvent->GetNumberOfTracks("Y");

    debug << "Ntracks X " << nTracksX << " Y " << nTracksY << " " << fTrackEvent->GetNumberOfTracks() << endl;

    // No tracks in X or Y nothing to do
    if (nTracksX == 0 || nTracksY == 0) return nullptr;

    Double_t totEnergyX = 0, totEnergyY = 0;
    Double_t maxTrackEnergyX = 0, maxTrackEnergyY = 0;
    Int_t maxIndexX = -1, maxIndexY = -1;

    for (int t = 0; t < fTrackEvent->GetNumberOfTracks(); t++) {
        if (!fTrackEvent->isTopLevel(t)) continue;
        TRestTrack* track = fTrackEvent->GetTrack(t);
        Double_t trackEn = track->GetEnergy();
        if (track->isXZ()) {
            if (trackEn > maxTrackEnergyX) {
                maxTrackEnergyX = trackEn;
                maxIndexX = t;
            }
            totEnergyX += trackEn;
        } else if (track->isYZ()) {
            if (trackEn > maxTrackEnergyY) {
                maxTrackEnergyY = trackEn;
                maxIndexY = t;
            }
            totEnergyY += trackEn;
        }
    }

    if (maxTrackEnergyX < totEnergyX * fTrackBalance || maxTrackEnergyY < totEnergyY * fTrackBalance)
        return nullptr;

    debug << "MaxIndex X " << maxIndexX << " Y " << maxIndexY << endl;

    if (maxIndexX == -1 || maxIndexY == -1) return nullptr;

    TRestTrack* tckX = fTrackEvent->GetTrack(maxIndexX);
    TRestTrack* tckY = fTrackEvent->GetTrack(maxIndexY);

    Double_t trackEnergy = maxTrackEnergyX + maxTrackEnergyY;

    TRestVolumeHits vHitsX = (TRestVolumeHits) * (tckX->GetVolumeHits());
    TRestVolumeHits vHitsY = (TRestVolumeHits) * (tckY->GetVolumeHits());

    TVector3 origX, endX;
    GetOriginEnd(vHitsX, origX, endX);
    TVector3 origY, endY;
    GetOriginEnd(vHitsY, origY, endY);

    double originZ = (origX.Z() + origY.Z()) / 2.;
    double endZ = (endX.Z() + endY.Z()) / 2.;

    debug << "Origin: " << origX.X() << " y: " << origY.Y() << " z: " << originZ << endl;
    debug << "End : " << endX.X() << " y: " << endY.Y() << " z: " << endZ << endl;

    double dX = (origX.X() - endX.X());
    double dY = (origY.Y() - endY.Y());
    double dZ = (originZ - endZ);
    Double_t length = TMath::Sqrt(dX * dX + dY * dY + dZ * dZ);
    Double_t angle = TMath::ACos(dZ / length);
    bool downwards = dZ > 0;

    debug << "Track length " << length << " angle: " << angle << endl;

    // A new value for each observable is added
    SetObservableValue("originX", origX.X());
    SetObservableValue("originY", origY.Y());
    SetObservableValue("originZ", originZ);
    SetObservableValue("endX", endX.X());
    SetObservableValue("endY", endY.Y());
    SetObservableValue("endZ", endZ);
    SetObservableValue("length", length);
    SetObservableValue("angle", angle);
    SetObservableValue("downwards", downwards);
    SetObservableValue("totalEnergy", trackEnergy);

    TRestTrack newTrackX;
    newTrackX.SetVolumeHits(vHitsX);
    newTrackX.SetParentID(tckX->GetTrackID());
    newTrackX.SetTrackID(fOutTrackEvent->GetNumberOfTracks() + 1);
    fOutTrackEvent->AddTrack(&newTrackX);

    TRestTrack newTrackY;
    newTrackY.SetVolumeHits(vHitsY);
    newTrackY.SetParentID(tckY->GetTrackID());
    newTrackY.SetTrackID(fOutTrackEvent->GetNumberOfTracks() + 1);
    fOutTrackEvent->AddTrack(&newTrackY);

    fOutTrackEvent->SetLevels();
    return fOutTrackEvent;
}

///////////////////////////////////////////////
/// \brief This function retreive the origin and the end of the track
/// based on the most energetic hit. The origin is defined as the further
/// hit deposition edge to the most energetic hit, while the track end is
/// defined as the closest edge to the most energetic hit.
///
void TRestTrackLineAnalysisProcess::GetOriginEnd(TRestVolumeHits& hits, TVector3& orig, TVector3& end) {
    const int nHits = hits.GetNumberOfHits();
    int maxBin;
    double maxEn = 0;

    for (int i = 0; i < nHits; i++) {
        double en = hits.GetEnergy(i);
        if (en > maxEn) {
            maxEn = en;
            maxBin = i;
        }
    }

    TVector3 maxPos = hits.GetPosition(maxBin);

    TVector3 pos0 = hits.GetPosition(0);
    TVector3 posE = hits.GetPosition(nHits - 1);

    const double maxToFirst = (pos0 - maxPos).Mag();
    const double maxToLast = (posE - maxPos).Mag();

    debug << "Max index " << maxBin << " Pos to first " << maxToFirst << " last " << maxToLast << endl;
    if (maxToFirst < maxToLast) {
        end = pos0;
        orig = posE;
    } else {
        orig = pos0;
        end = posE;
    }

    debug << "Origin " << orig.X() << " " << orig.Y() << " " << orig.Z() << endl;
    debug << "End    " << end.X() << " " << end.Y() << " " << end.Z() << endl;
}

///////////////////////////////////////////////
/// \brief Function to include required actions after all events have been
/// processed. Nothing to do...
///
void TRestTrackLineAnalysisProcess::EndProcess() {}
