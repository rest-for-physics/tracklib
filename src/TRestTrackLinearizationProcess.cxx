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
//////////////////////////////////////////////////////////////////////////
/// The TRestTrackLinearization process is meant to analyze linear tracks,
/// such as alpha tracks. It should retreive the information after performing
/// TRestTrackLinearization process, not implemented for XYZ tracks.
/// It fits the raw hits to a line, two fits are performed: to the prior direction
/// e.g. XZ and the inverted direction e.g. ZX. This is because linear fits are not
/// capable to fit vertical lines. The fit which minimizes the chi2, is selected.
/// Afterwards some nodes with equal distance are selected following the fit result,
/// the maximum number of nodes is defines by the parameter fMaxNodes. Then the closest
/// hits to the nodes are computed using kMeansClustering analysis. If a node is found
/// with no hits is rejected. The nodes are stored as a top node track for further
/// processing.
///
/// ### Parameters
/// * fMaxNodes : Maximum number of nodes (hits) to reduce the hits to a line
///
/// ### Examples
/// \code
///            <addProcess type="TRestTrackLinearizationProcess" name="alphaTrackAna" value="ON"
///            verboseLevel="info" >
///                   <parameter name="maxNodes" value="6" />
/// 		</addProcess>
/// \endcode
///
///--------------------------------------------------------------------------
///
/// REST-for-Physics - Software for Rare Event Searches Toolkit
///
/// History of developments:
///
/// 2022-March First implementation
/// JuanAn Garcia
///
/// \class TRestTrackLineAnalysisProcess
/// \author: JuanAn Garcia   e-mail: juanangp@unizar.es
///
/// <hr>
///

#include "TRestTrackLinearizationProcess.h"

#include "TRestTrackReductionProcess.h"
using namespace std;

ClassImp(TRestTrackLinearizationProcess);

///////////////////////////////////////////////
/// \brief Default constructor
///
TRestTrackLinearizationProcess::TRestTrackLinearizationProcess() { Initialize(); }

///////////////////////////////////////////////
/// \brief Default destructor
///
TRestTrackLinearizationProcess::~TRestTrackLinearizationProcess() { delete fOutTrackEvent; }

///////////////////////////////////////////////
/// \brief Function to initialize input/output event members and define
/// the section name
///
void TRestTrackLinearizationProcess::Initialize() {
    SetSectionName(this->ClassName());
    SetLibraryVersion(LIBRARY_VERSION);

    fTrackEvent = nullptr;
    fOutTrackEvent = new TRestTrackEvent();
}

///////////////////////////////////////////////
/// \brief Process initialization. Nothing to do here.
///
void TRestTrackLinearizationProcess::InitProcess() {}

///////////////////////////////////////////////
/// \brief The main processing event function
///
TRestEvent* TRestTrackLinearizationProcess::ProcessEvent(TRestEvent* inputEvent) {
    fTrackEvent = (TRestTrackEvent*)inputEvent;

    for (int t = 0; t < fTrackEvent->GetNumberOfTracks(); t++)
        fOutTrackEvent->AddTrack(fTrackEvent->GetTrack(t));

    for (int t = 0; t < fTrackEvent->GetNumberOfTracks(); t++) {
        if (fTrackEvent->GetLevel(t) > 1) continue;
        TRestTrack* track = fTrackEvent->GetTrack(t);
        TRestVolumeHits* hits = track->GetVolumeHits();
        TRestVolumeHits vHits;
        // Perform track linearization
        GetHitsProjection(hits, fMaxNodes, vHits);
        if (vHits.GetNumberOfHits() == 0) continue;

        RESTDebug << "Adding track " << RESTendl;
        // Store tracks after tinearization
        TRestTrack newTrack;
        newTrack.SetTrackID(fOutTrackEvent->GetNumberOfTracks() + 1);
        newTrack.SetParentID(track->GetTrackID());
        newTrack.SetVolumeHits(vHits);

        RESTDebug << "Is XZ " << newTrack.isXZ() << " Is YZ " << newTrack.isYZ() << RESTendl;

        fOutTrackEvent->AddTrack(&newTrack);
    }

    RESTDebug << "NTracks  X " << fOutTrackEvent->GetNumberOfTracks("X") << " Y "
              << fOutTrackEvent->GetNumberOfTracks("Y") << " " << fOutTrackEvent->GetNumberOfTracks("X")
              << RESTendl;

    fOutTrackEvent->SetLevels();
    return fOutTrackEvent;
}

///////////////////////////////////////////////
/// \brief This function performs the track linearization towards a given number of nodes
/// the nodes are extracted from the linear fit on GetBestNodes and afterwards
/// the closest hits to the nodes are merged to correct the position of the nodes
/// using kmeansClustering.
///
void TRestTrackLinearizationProcess::GetHitsProjection(TRestVolumeHits* hits, const int& nodes,
                                                       TRestVolumeHits& vHits) {
    const int nHits = hits->GetNumberOfHits();
    const REST_HitType hType = hits->GetType(0);
    vHits.RemoveHits();

    RESTDebug << "NHits " << nHits << " hits Type " << hType << RESTendl;

    if (hType == XZ) {
        auto cl = GetBestNodes(hits->GetX(), hits->GetZ(), hits->GetEnergyVector(), nodes);
        for (const auto& [xy, z] : cl) {
            vHits.AddHit(xy, hits->GetY(0), z, 0, 0, hType, 0, 0, 0);
        }
    } else if (hType == YZ) {
        auto cl = GetBestNodes(hits->GetY(), hits->GetZ(), hits->GetEnergyVector(), nodes);
        for (const auto& [xy, z] : cl) {
            vHits.AddHit(hits->GetX(0), xy, z, 0, 0, hType, 0, 0, 0);
        }
    } else if (hType == XY) {
        auto cl = GetBestNodes(hits->GetX(), hits->GetY(), hits->GetEnergyVector(), nodes);
        for (const auto& [xy, z] : cl) {
            vHits.AddHit(xy, z, hits->GetZ(0), 0, 0, hType, 0, 0, 0);
        }
    } else {
        RESTWarning << "Track linearization not implemented for XYZ tracks" << RESTendl;
        return;
    }

    TRestVolumeHits::kMeansClustering(hits, vHits, 1);

    if (GetVerboseLevel() >= TRestStringOutput::REST_Verbose_Level::REST_Debug)
        for (unsigned int i = 0; i < vHits.GetNumberOfHits(); i++)
            RESTDebug << i << " " << vHits.GetX(i) << " " << vHits.GetY(i) << " " << vHits.GetZ(i) << " "
                      << vHits.GetType(i) << RESTendl;
}

///////////////////////////////////////////////
/// \brief This function performs a linear fit to the volumehits of the track weighthed by the
/// energy of the hits. Two fit are performed by rotating the axis and the best Chi2 is
/// selected. Afterwards, equidistant nodes following the fit and after the linear fit are
/// extracted.
///
std::vector<std::pair<double, double>> TRestTrackLinearizationProcess::GetBestNodes(
    const std::vector<Float_t>& fXY, const std::vector<Float_t>& fZ, const std::vector<Float_t>& fEn,
    const int& nodes) {
    const int nHits = fXY.size();

    const double max[2] = {TMath::MaxElement(fXY.size(), fXY.data()),
                           TMath::MaxElement(fZ.size(), fZ.data())};
    const double min[2] = {TMath::MinElement(fXY.size(), fXY.data()),
                           TMath::MinElement(fZ.size(), fZ.data())};

    double totEn = 0;
    for (const auto& en : fEn) totEn += en;

    RESTDebug << "Max " << max[0] << " " << max[1] << RESTendl;
    RESTDebug << "Min " << min[0] << " " << min[1] << RESTendl;

    TGraph gr[2];

    const int hitAvg = std::round(totEn / (double)nHits / 10.);
    int c = 0;
    for (int h = 0; h < nHits; h++) {
        double en = fEn.at(h);
        for (int e = 0; e < en; e += hitAvg) {
            gr[0].SetPoint(c, fXY.at(h), fZ.at(h));
            gr[1].SetPoint(c, fZ.at(h), fXY.at(h));
            c++;
        }
    }

    RESTDebug << "Points graph " << c << " Hits " << nHits << RESTendl;

    int bestIndex = 0;
    double bestFit;
    double slope;
    double intercept;

    std::string fitOpt = "";
    if (GetVerboseLevel() < TRestStringOutput::REST_Verbose_Level::REST_Debug) fitOpt = "Q";

    for (int l = 0; l < 2; l++) {
        TF1 f1("f1", "[0] * x + [1]", min[l], max[l]);
        gr[l].Fit(&f1, fitOpt.c_str());
        double fitGoodness = f1.GetChisquare();

        if (l == 0) {
            bestFit = fitGoodness;
            slope = f1.GetParameter(0);
            intercept = f1.GetParameter(1);
            continue;
        } else if (fitGoodness < bestFit) {
            bestFit = fitGoodness;
            bestIndex = l;
            slope = f1.GetParameter(0);
            intercept = f1.GetParameter(1);
        }
    }

    RESTDebug << "Best fit " << bestFit << " " << bestIndex << RESTendl;
    RESTDebug << "Slope " << slope << " Intercept " << intercept << RESTendl;

    std::vector<std::pair<double, double>> cluster(nodes);
    for (int i = 0; i < nodes; i++) {
        double first = min[bestIndex] + ((double)i) * (max[bestIndex] - min[bestIndex]) / (double)(nodes - 1);
        double second = first * slope + intercept;
        if (bestIndex == 0)
            cluster[i] = std::make_pair(first, second);
        else
            cluster[i] = std::make_pair(second, first);
    }

    return cluster;
}

///////////////////////////////////////////////
/// \brief Function to include required actions after all events have been
/// processed.
///
void TRestTrackLinearizationProcess::EndProcess() {}
