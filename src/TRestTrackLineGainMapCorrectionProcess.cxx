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
/// * **lineAnaMethod**: Method to evaluate the origin and end of the track, currently
/// 3D method and default are implemented
///
/// ### Observables
/// * **trackBalanceXZ**: Track balance between the most energetic track and all tracks in the XZ projection
/// * **trackBalanceYZ**: Track balance between the most energetic track and all tracks in the YZ projection
/// * **trackBalance**: Total track balance between the most energetic tracks (XZ + YZ) and all tracks
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
/// * **relativeZ**: Relative Z position in which the half of the integral is reached,
/// when this value is below 0.5 it means that the track is downwards and upwards otherwise
///
/// ### Examples
/// \code
///            <addProcess type="TRestTrackLineGainMapCorrectionProcess" name="alphaTrackAna" value="ON"
///            verboseLevel="silent" >
///                   <observable name="trackBalanceXZ" value="ON" />
///                   <observable name="trackBalanceYZ" value="ON" />
///                   <observable name="trackBalance" value="ON" />
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
/// \class TRestTrackLineGainMapCorrectionProcess
/// \author: JuanAn Garcia   e-mail: juanangp@unizar.es
///
/// <hr>
///

#include "TRestTrackLineGainMapCorrectionProcess.h"

#include "TRestTrackReductionProcess.h"
using namespace std;

ClassImp(TRestTrackLineGainMapCorrectionProcess);

///////////////////////////////////////////////
/// \brief Default constructor
///
TRestTrackLineGainMapCorrectionProcess::TRestTrackLineGainMapCorrectionProcess() { Initialize(); }

///////////////////////////////////////////////
/// \brief Default destructor
///
TRestTrackLineGainMapCorrectionProcess::~TRestTrackLineGainMapCorrectionProcess() { delete fOutTrackEvent; }

///////////////////////////////////////////////
/// \brief Function to initialize input/output event members and define
/// the section name
///
void TRestTrackLineGainMapCorrectionProcess::Initialize() {
    SetSectionName(this->ClassName());
    SetLibraryVersion(LIBRARY_VERSION);

    fTrackEvent = nullptr;
    fOutTrackEvent = new TRestTrackEvent();
}

///////////////////////////////////////////////
/// \brief Process initialization.
/// Nothing to do...
///
void TRestTrackLineGainMapCorrectionProcess::InitProcess() {
    std::cout << "Importando GM" << std::endl;
    fGainMap.Import(fGainMapFile);
    std::cout << "Importado" << std::endl;
}

///////////////////////////////////////////////
/// \brief The main processing event function
///
TRestEvent* TRestTrackLineGainMapCorrectionProcess::ProcessEvent(TRestEvent* inputEvent) {
    fTrackEvent = (TRestTrackEvent*)inputEvent;

    auto m = fGainMap.GetModule();

    // Initialize outputTrackEvent
    for (int t = 0; t < fTrackEvent->GetNumberOfTracks(); t++)
        fOutTrackEvent->AddTrack(fTrackEvent->GetTrack(t));

    TRestTrack* tckX = fTrackEvent->GetMaxEnergyTrackInX();
    TRestTrack* tckY = fTrackEvent->GetMaxEnergyTrackInY();

    if (!tckX || !tckY) return nullptr;

    // Match the XZ with the YZ hits of the linearized track (should have small number of nodes)
    TRestTrack* tckXYZ = new TRestTrack();
    TRestVolumeHits* hitsXYZ = new TRestVolumeHits();
    auto hitsX = tckX->GetHits();
    auto hitsY = tckY->GetHits();
    for (size_t i = 0; i < hitsX->GetNumberOfHits(); i++) {
        double X = hitsX->GetPosition(i).X();
        double Z = hitsX->GetPosition(i).Z();
        double energy = hitsX->GetEnergy(i);
        int closestYhit = 0;
        double zClosestYhit = 9e9;
        double eClosestYhit = 0;
        double yClosestYhit = 0;
        for (int j = 0; j < hitsY->GetNumberOfHits(); j++) {
            double Y = hitsY->GetPosition(i).Y();
            double ZfromY = hitsY->GetPosition(i).Z();
            double energyFromY = hitsY->GetEnergy(i);
            if ((Z - ZfromY) * (Z - ZfromY) < (Z - zClosestYhit) * (Z - zClosestYhit)) {
                closestYhit = j;
                zClosestYhit = ZfromY;
                eClosestYhit = energyFromY;
                yClosestYhit = Y;
            }
        }
        hitsXYZ->AddHit(X, yClosestYhit, (Z + zClosestYhit) / 2., energy + eClosestYhit, 0, REST_HitType::XYZ,
                        0, 0, 0);
    }
    tckXYZ->SetVolumeHits(*hitsXYZ);

    // apply calibration from gainmap
    TRestTrack* tckXYZcalibrated = new TRestTrack();
    TRestVolumeHits* hitsXYZcalibrated = new TRestVolumeHits();
    for (size_t i = 0; i < hitsXYZ->GetNumberOfHits(); i++) {
        double x = hitsXYZ->GetPosition(i).X();
        double y = hitsXYZ->GetPosition(i).Y();
        double z = hitsXYZ->GetPosition(i).Z();
        double e = hitsXYZ->GetEnergy(i);
        double slope = m->GetSlope(x, y);
        double intercept = m->GetIntercept(x, y);
        if (slope == 0) slope = 1;
        double eCorrected = e * slope + intercept * e / hitsXYZ->GetTotalEnergy();
        RESTDebug << "Hit " << i << ": (" << x << "," << y << "," << z << "); " << e << " " << slope << " -> "
                  << eCorrected << RESTendl;

        hitsXYZcalibrated->AddHit(x, y, z, eCorrected, 0, REST_HitType::XYZ, 0, 0, 0);
    }
    tckXYZcalibrated->SetVolumeHits(*hitsXYZcalibrated);

    Double_t trackEnergy = tckXYZcalibrated->GetEnergy();
    Double_t trackEnergyNoSegmentation =
        tckXYZ->GetEnergy() * m->GetSlopeFullSpc() + m->GetInterceptFullSpc();

    // A new value for each observable is added
    SetObservableValue("energy", trackEnergy);
    SetObservableValue("energyNoSegmentation", trackEnergyNoSegmentation);

    // Save the merged XYZ track
    tckXYZ->SetParentID(tckX->GetTrackID());
    tckXYZ->SetTrackID(fOutTrackEvent->GetNumberOfTracks() + 1);
    fOutTrackEvent->AddTrack(tckXYZ);

    // Save the merged XYZ track calibrated
    tckXYZcalibrated->SetParentID(tckXYZ->GetTrackID());
    tckXYZcalibrated->SetTrackID(fOutTrackEvent->GetNumberOfTracks() + 1);
    fOutTrackEvent->AddTrack(tckXYZcalibrated);

    fOutTrackEvent->SetLevels();
    return fOutTrackEvent;
}

///////////////////////////////////////////////
/// \brief Function to include required actions after all events have been
/// processed. Nothing to do...
///
void TRestTrackLineGainMapCorrectionProcess::EndProcess() {}
