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

#ifndef RestCore_TRestTrackLinearizationProcess
#define RestCore_TRestTrackLinearizationProcess

#include <TRestTrackEvent.h>

#include "TRestEventProcess.h"

//! A process to perform track linearization
class TRestTrackLinearizationProcess : public TRestEventProcess {
   private:
    TRestTrackEvent* fTrackEvent;     //!
    TRestTrackEvent* fOutTrackEvent;  //!

    void Initialize();

   protected:
    // A parameter which defines the maximum number of nodes for the track linearization
    Int_t fMaxNodes = 6;

   public:
    inline any GetInputEvent() const { return fTrackEvent; }
    inline any GetOutputEvent() const { return fOutTrackEvent; }

    void InitProcess();
    TRestEvent* ProcessEvent(TRestEvent* eventInput);
    void EndProcess();

    void PrintMetadata() {
        BeginPrintProcess();
        metadata << "Max nodes: " << fMaxNodes << endl;
        EndPrintProcess();
    }

    void GetHitsProjection(TRestVolumeHits* hits, const int& nodes, TRestVolumeHits& vHits);
    std::vector<std::pair<double, double>> GetBestNodes(const std::vector<Float_t>& fXY,
                                                        const std::vector<Float_t>& fZ,
                                                        const std::vector<Float_t>& fEn, const int& nodes);

    inline const char* GetProcessName() const { return "trackLinearization"; }

    TRestTrackLinearizationProcess();
    ~TRestTrackLinearizationProcess();

    // ROOT class definition helper. Increase the number in it every time
    // you add/rename/remove the process parameters
    ClassDef(TRestTrackLinearizationProcess, 1);
};
#endif
