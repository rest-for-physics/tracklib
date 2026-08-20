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

#ifndef RestCore_TRestTrackViewerProcess
#define RestCore_TRestTrackViewerProcess

#include <TGraph.h>
#include <TLegend.h>
#include <TRestEventProcess.h>

#include "TRestTrackEvent.h"

//! A generic viewer REST process to visualize track events
class TRestTrackViewerProcess : public TRestEventProcess {
   private:
    TRestTrackEvent* fTrackEvent;  //!

    // In case of alpha tracks one might be interested on drawing the origin and end from the analysis
    bool fDrawOriginEnd = false;
    bool fDrawHits = true;

    // Graph to store the origin of the track
    std::vector<TGraph*> originGr{nullptr, nullptr};  //!
    // Graph to store the end of the track
    std::vector<TGraph*> endGr{nullptr, nullptr};  //!
    // Graph to store legends
    std::vector<TLegend*> legOrEnd{nullptr, nullptr};  //!

    TCanvas* hCanvas = nullptr;

    void Initialize() override;

   protected:
    // add here the members of your event process

   public:
    RESTValue GetInputEvent() const override { return fTrackEvent; }
    RESTValue GetOutputEvent() const override { return fTrackEvent; }

    void InitProcess() override;
    TRestEvent* ProcessEvent(TRestEvent* inputEvent) override;
    void EndProcess() override;

    void PrintMetadata() override {
        BeginPrintProcess();

        RESTMetadata << "Draw Origin End : " << fDrawOriginEnd << RESTendl;

        EndPrintProcess();
    }

    const char* GetProcessName() const override { return "trackViewer"; }

    // Constructor
    TRestTrackViewerProcess();
    // Destructor
    ~TRestTrackViewerProcess();

    ClassDefOverride(TRestTrackViewerProcess, 1);  // Template for a REST "event process" class inherited from
                                                   // TRestEventProcess
};
#endif
