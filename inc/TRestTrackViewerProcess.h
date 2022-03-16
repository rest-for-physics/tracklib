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

#include <TLegend.h>
#include <TGraph.h>

#include "TRestTrackEvent.h"
#include "TRestEventProcess.h"

//! A generic viewer REST process to visualize track events
class TRestTrackViewerProcess : public TRestEventProcess {
   private:
    TRestTrackEvent* fTrackEvent;  //!

    //In case of alpha tracks one might be interested on drawing the origin and end from the analysis
    bool fDrawOriginEnd = false;
    bool fDrawHits = true;

    //Graph to store the origin of the track
    std::vector<TGraph *>  originGr {nullptr,nullptr}; //!
    //Graph to store the end of the track
    std::vector<TGraph *>  endGr {nullptr,nullptr};    //!
    //Graph to store legends
    std::vector<TLegend *> legOrEnd {nullptr,nullptr}; //!

    TCanvas *hCanvas = nullptr;

    void Initialize();

   protected:
    // add here the members of your event process

   public:
    any GetInputEvent() { return fTrackEvent; }
    any GetOutputEvent() { return fTrackEvent; }

    void InitProcess();
    TRestEvent* ProcessEvent(TRestEvent* eventInput);
    void EndProcess();

    void PrintMetadata() {
        BeginPrintProcess();

        metadata << "Draw Origin End : " << fDrawOriginEnd << endl;

        EndPrintProcess();
    }

    TString GetProcessName() { return (TString) "trackViewer"; }

    static void GetOriginEnd(TRestAnalysisTree *anaTree, std::vector<TGraph *> &origin, std::vector<TGraph *>  &end, std::vector<TLegend *> &leg);
    static void DrawOriginEnd(TPad *pad, std::vector<TGraph *> &origin, std::vector<TGraph *> &end, std::vector<TLegend *> &leg);

    // Constructor
    TRestTrackViewerProcess();
    // Destructor
    ~TRestTrackViewerProcess();

    ClassDef(TRestTrackViewerProcess, 1);  // Template for a REST "event process" class inherited from
                                               // TRestEventProcess
};
#endif
