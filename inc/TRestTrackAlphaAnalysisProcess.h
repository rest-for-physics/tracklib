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

#ifndef RestCore_TRestTrackAlphaAnalysisProcess
#define RestCore_TRestTrackAlphaAnalysisProcess

#include <TRestTrackEvent.h>

#include "TRestEventProcess.h"

//Help class for data handling
class alphaTrackHit{
  public:
     double xy;
     double z;
     double en;

  //Sort data structure by time
  static bool sortByTime (const alphaTrackHit &h1, const alphaTrackHit &h2 ){
    return h1.z > h2.z;
  }

  //Sort data structure by x or y coordinate
  static bool sortByPos (const alphaTrackHit &h1, const alphaTrackHit &h2 ){
    return h1.xy > h2.xy;
  }
};

//! A process to analyze alpha tracks
class TRestTrackAlphaAnalysisProcess : public TRestEventProcess {
   private:
   
#ifndef __CINT__
    // Input event Track Event
    TRestTrackEvent* fTrackEvent;
    // Output event Track event smoothed 
    TRestTrackEvent* fOutTrackEvent;
#endif

    void Initialize();

   protected:

   // A parameter to define the track balance, otherwise the eent is rejected 
   Double_t fTrackBalance = 0.65; 

   public:
    any GetInputEvent() { return fTrackEvent; }
    any GetOutputEvent() { return fOutTrackEvent; }

    void InitProcess();
    TRestEvent* ProcessEvent(TRestEvent* eventInput);
    void EndProcess();

    void PrintMetadata() {
        BeginPrintProcess();
        metadata<<"Track Balance: "<<fTrackBalance<<endl;
        EndPrintProcess();
    }

    // This function performs the track smoothing in the Z axis
    std::vector<alphaTrackHit> smoothTrack(std::vector <alphaTrackHit> &hits);

    TString GetProcessName() { return (TString) "alphaTrackAna"; }

    // Constructor
    TRestTrackAlphaAnalysisProcess();
    TRestTrackAlphaAnalysisProcess(char* cfgFileName);
    // Destructor
    ~TRestTrackAlphaAnalysisProcess();

    ClassDef(TRestTrackAlphaAnalysisProcess, 1);
                   // Template for a REST "event process" class inherited from TRestEventProcess
};
#endif
