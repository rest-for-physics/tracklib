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
/// The TRestTrackViewerProcess allows to display tracks
/// registered inside a TRestTrackEvent.
///
/// <hr>
///
/// \warning **⚠ REST is under continous development.** This
/// documentation
/// is offered to you by the REST community. Your HELP is needed to keep this
/// code
/// up to date. Your feedback will be worth to support this software, please
/// report
/// any problems/suggestions you may find while using it at [The REST Framework
/// forum](http://ezpc10.unizar.es). You are welcome to contribute fixing typos,
/// updating
/// information or adding/proposing new contributions. See also our
/// <a href="https://github.com/rest-for-physics/framework/blob/master/CONTRIBUTING.md">Contribution
/// Guide</a>.
///
///
///--------------------------------------------------------------------------
///
/// RESTsoft - Software for Rare Event Searches with TPCs
///
/// \class      TRestTrackViewerProcess
/// \author     Juanan Garcia
///
/// <hr>
///
#include "TRestTrackViewerProcess.h"

#include "TRestTrackLineAnalysisProcess.h"

ClassImp(TRestTrackViewerProcess);

///////////////////////////////////////////////
/// \brief Default constructor
///
TRestTrackViewerProcess::TRestTrackViewerProcess() { Initialize(); }

///////////////////////////////////////////////
/// \brief Default destructor
///
TRestTrackViewerProcess::~TRestTrackViewerProcess() {}

///////////////////////////////////////////////
/// \brief Function to initialize input/output event members and define the
/// section name
///
void TRestTrackViewerProcess::Initialize() {
    SetSectionName(this->ClassName());
    SetLibraryVersion(LIBRARY_VERSION);

    fTrackEvent = nullptr;
    fSingleThreadOnly = true;
}

///////////////////////////////////////////////
/// \brief Process initialization. It creates the canvas available in
/// TRestEventProcess
///
void TRestTrackViewerProcess::InitProcess() {
    CreateCanvas();
    if (!hCanvas && fDrawHits)
        hCanvas = new TCanvas("hitsCanvas", "hitsCanvas", fCanvasSize.X(), fCanvasSize.Y());
}

///////////////////////////////////////////////
/// \brief The main processing event function
///
TRestEvent* TRestTrackViewerProcess::ProcessEvent(TRestEvent* evInput) {
    // no need for verbose copy now
    fTrackEvent = (TRestTrackEvent*)evInput;

    fCanvas->cd();
    TPad* pad = fTrackEvent->DrawEvent();
    if (fDrawOriginEnd) {
        fTrackEvent->GetOriginEnd(originGr, endGr, legOrEnd);
        fTrackEvent->DrawOriginEnd(pad, originGr, endGr, legOrEnd);
    }

    fCanvas->cd();
    pad->Draw();
    fCanvas->Update();

    if (!fDrawHits) return fTrackEvent;

    hCanvas->cd();
    TPad* hPad = fTrackEvent->DrawHits();
    if (fDrawOriginEnd) {
        fTrackEvent->DrawOriginEnd(hPad, originGr, endGr, legOrEnd);
    }

    hCanvas->cd();
    hPad->Draw();
    hCanvas->Update();

    return fTrackEvent;
}

///////////////////////////////////////////////
/// \brief Function to include required actions after all events have been
/// processed.
///
void TRestTrackViewerProcess::EndProcess() {}
