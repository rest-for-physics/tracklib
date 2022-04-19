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

    // Initialize outputTrackEvent
    for (int t = 0; t < fTrackEvent->GetNumberOfTracks(); t++)
        fOutTrackEvent->AddTrack(fTrackEvent->GetTrack(t));

    TRestTrack* tckX = fTrackEvent->GetMaxEnergyTrackInX();
    TRestTrack* tckY = fTrackEvent->GetMaxEnergyTrackInY();

    if (!tckX || !tckY) return nullptr;

    TRestVolumeHits vHitsX = (TRestVolumeHits) * (tckX->GetVolumeHits());
    TRestVolumeHits vHitsY = (TRestVolumeHits) * (tckY->GetVolumeHits());

    TVector3 orig, end;
    // Retreive origin and end of the track for the XZ projection
    GetOriginEnd(vHitsX, vHitsY, orig, end);

    debug << "Origin: " << orig.X() << " y: " << orig.Y() << " z: " << orig.Z() << endl;
    debug << "End : " << end.X() << " y: " << end.Y() << " z: " << end.Z() << endl;

    // Compute some observables
    double dX = (orig.X() - end.X());
    double dY = (orig.Y() - end.Y());
    double dZ = (orig.Z() - end.Z());
    Double_t length = TMath::Sqrt(dX * dX + dY * dY + dZ * dZ);
    Double_t angle = TMath::ACos(dZ / length);
    bool downwards = dZ > 0;

    debug << "Track length " << length << " angle: " << angle << endl;

    Double_t trackEnergyX = tckX->GetEnergy();
    Double_t trackEnergyY = tckY->GetEnergy();
    Double_t trackEnergy = trackEnergyX + trackEnergyY;

    Double_t trackBalanceX = 0;
    if (trackEnergyX > 0) trackBalanceX = fTrackEvent->GetEnergy("X") / trackEnergyX;

    Double_t trackBalanceY = 0;
    if (trackEnergyY > 0) trackBalanceY = fTrackEvent->GetEnergy("Y") / trackEnergyY;

    // A new value for each observable is added
    SetObservableValue("trackBalanceX", trackBalanceX);
    SetObservableValue("trackBalanceY", trackBalanceY);
    SetObservableValue("originX", orig.X());
    SetObservableValue("originY", orig.Y());
    SetObservableValue("originZ", orig.Z());
    SetObservableValue("endX", end.X());
    SetObservableValue("endY", end.Y());
    SetObservableValue("endZ", end.Z());
    SetObservableValue("length", length);
    SetObservableValue("angle", angle);
    SetObservableValue("downwards", downwards);
    SetObservableValue("totalEnergy", trackEnergy);

    // Save most energetic XZ track
    TRestTrack newTrackX;
    newTrackX.SetVolumeHits(vHitsX);
    newTrackX.SetParentID(tckX->GetTrackID());
    newTrackX.SetTrackID(fOutTrackEvent->GetNumberOfTracks() + 1);
    fOutTrackEvent->AddTrack(&newTrackX);

    // Save most energetic YZ track
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
void TRestTrackLineAnalysisProcess::GetOriginEnd(TRestVolumeHits& hitsX, TRestVolumeHits& hitsY,
                                                 TVector3& orig, TVector3& end) {
    TVector3 origX, endX;
    // Retreive origin and end of the track for the XZ projection
    GetBoundaries(hitsX, origX, endX);
    TVector3 origY, endY;
    // Retreive origin and end of the track for the YZ projection
    GetBoundaries(hitsY, origY, endY);

    double originZ = (origX.Z() + origY.Z()) / 2.;
    double endZ = (endX.Z() + endY.Z()) / 2.;

    orig = TVector3(origX.X(), origY.Y(), originZ);
    end = TVector3(endX.X(), endY.Y(), endZ);
}

///////////////////////////////////////////////
/// \brief This function retreive the origin and the end of a single
/// track projection (XZ or YZ) based on the most energetic hit. The origin
/// is defined as the further hit deposition edge to the most energetic hit,
/// while the track end is defined as the closest edge to the most energetic hit.
///
void TRestTrackLineAnalysisProcess::GetBoundaries(TRestVolumeHits& hits, TVector3& orig, TVector3& end) {
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

    if (maxToFirst < maxToLast) {
        end = pos0;
        orig = posE;
    } else {
        orig = pos0;
        end = posE;
    }
}

///////////////////////////////////////////////
/// \brief Function to include required actions after all events have been
/// processed. Nothing to do...
///
void TRestTrackLineAnalysisProcess::EndProcess() {}
