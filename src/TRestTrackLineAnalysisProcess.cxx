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

    TVector3 orig = TVector3(0, 0, 0), end = TVector3(0, 0, 0);
    Double_t length = -1;
    Double_t angle = -10;
    bool downwards = true;
    Double_t trackEnergyX = 0, trackEnergyY = 0;
    Double_t trackBalanceX = 0, trackBalanceY = 0;

    if (tckX && tckY) {
        // Retreive origin and end of the track for the XZ projection
        fTrackEvent->GetMaxTrackBoundaries(orig, end);

        debug << "Origin: " << orig.X() << " y: " << orig.Y() << " z: " << orig.Z() << endl;
        debug << "End : " << end.X() << " y: " << end.Y() << " z: " << end.Z() << endl;

        // Compute some observables
        double dX = (orig.X() - end.X());
        double dY = (orig.Y() - end.Y());
        double dZ = (orig.Z() - end.Z());
        length = TMath::Sqrt(dX * dX + dY * dY + dZ * dZ);
        angle = TMath::ACos(dZ / length);
        downwards = dZ > 0;

        debug << "Track length " << length << " angle: " << angle << endl;

        trackEnergyX = tckX->GetEnergy();
        trackEnergyY = tckY->GetEnergy();

        if (trackEnergyX > 0) trackBalanceX = fTrackEvent->GetEnergy("X") / trackEnergyX;
        if (trackEnergyY > 0) trackBalanceY = fTrackEvent->GetEnergy("Y") / trackEnergyY;
    }

    Double_t trackEnergy = trackEnergyX + trackEnergyY;

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

    if (!tckX || !tckY) return nullptr;

    // Save most energetic XZ track
    tckX->SetParentID(tckX->GetTrackID());
    tckX->SetTrackID(fOutTrackEvent->GetNumberOfTracks() + 1);
    fOutTrackEvent->AddTrack(tckX);

    // Save most energetic YZ track
    tckY->SetParentID(tckY->GetTrackID());
    tckY->SetTrackID(fOutTrackEvent->GetNumberOfTracks() + 1);
    fOutTrackEvent->AddTrack(tckY);

    fOutTrackEvent->SetLevels();
    return fOutTrackEvent;
}

///////////////////////////////////////////////
/// \brief Function to include required actions after all events have been
/// processed. Nothing to do...
///
void TRestTrackLineAnalysisProcess::EndProcess() {}
