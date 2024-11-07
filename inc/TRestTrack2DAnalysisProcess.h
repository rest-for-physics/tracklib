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

#ifndef RestCore_TRestTrack2DAnalysisProcess
#define RestCore_TRestTrack2DAnalysisProcess

#include <TRestEventProcess.h>

#include "TRestTrackEvent.h"

//! An analysis REST process to extract valuable information from Track type of data.
class TRestTrack2DAnalysisProcess : public TRestEventProcess {
   private:
    TRestTrackEvent* fTrackEvent;  //!

    void InitFromConfigFile() override;

    void Initialize() override;

    void LoadDefaultConfig();

   protected:
   public:
    RESTValue GetInputEvent() const override { return fTrackEvent; }
    RESTValue GetOutputEvent() const override { return fTrackEvent; }

    void InitProcess() override;
    TRestEvent* ProcessEvent(TRestEvent* inputEvent) override;
    void EndProcess() override;

    void LoadConfig(const std::string& configFilename, const std::string& name = "");

    void PrintMetadata() override {
        BeginPrintProcess();

        ///////// Metadata ////////

        EndPrintProcess();
    }

    const char* GetProcessName() const override { return "trackAnalysis"; }

    // Constructor
    TRestTrack2DAnalysisProcess();
    TRestTrack2DAnalysisProcess(const char* configFilename);
    // Destructor
    ~TRestTrack2DAnalysisProcess();

    ClassDefOverride(TRestTrack2DAnalysisProcess, 1);  // Template for a REST "event process" class inherited
                                                       // from TRestEventProcess
};
#endif
