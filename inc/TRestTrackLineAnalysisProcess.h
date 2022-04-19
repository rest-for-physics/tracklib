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

#ifndef RestCore_TRestTrackLineAnalysisProcess
#define RestCore_TRestTrackLineAnalysisProcess

#include <TRestTrackEvent.h>

#include "TRestEventProcess.h"

//! A process to identify and extrack origin and end of a track assuming a linear track
class TRestTrackLineAnalysisProcess : public TRestEventProcess {
   private:
    /// A pointer to the input event Track Event
    TRestTrackEvent* fTrackEvent;

    /// A pointer to the output event Track event
    TRestTrackEvent* fOutTrackEvent;

    void Initialize();

   protected:
   public:
    any GetInputEvent() { return fTrackEvent; }
    any GetOutputEvent() { return fOutTrackEvent; }

    void InitProcess();
    TRestEvent* ProcessEvent(TRestEvent* eventInput);
    void EndProcess();

    void PrintMetadata() {
        BeginPrintProcess();
        EndPrintProcess();
    }

    static void GetOriginEnd(TRestVolumeHits& hitsX, TRestVolumeHits& hitsY, TVector3& orig, TVector3& end);
    static void GetBoundaries(TRestVolumeHits& hits, TVector3& orig, TVector3& end);

    TString GetProcessName() { return (TString) "trackLineAna"; }

    // Constructor
    TRestTrackLineAnalysisProcess();
    // Destructor
    ~TRestTrackLineAnalysisProcess();

    ClassDef(TRestTrackLineAnalysisProcess, 1);
};
#endif
