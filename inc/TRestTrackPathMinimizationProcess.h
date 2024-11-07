//////////////////////////////////////////////////////////////////////////
///
///
///             RESTSoft : Software for Rare Event Searches with TPCs
///
///             TRestTrackPathMinimizationProcess.h
///
///              Jan 2016 : Javier Galan
///
//////////////////////////////////////////////////////////////////////////

#ifndef RestCore_TRestTrackPathMinimizationProcess
#define RestCore_TRestTrackPathMinimizationProcess

#include <TRestEventProcess.h>

#include "TRestTrackEvent.h"
#include "trackMinimization.h"

class TRestTrackPathMinimizationProcess : public TRestEventProcess {
   private:
#ifndef __CINT__
    TRestTrackEvent* fInputTrackEvent;   //!
    TRestTrackEvent* fOutputTrackEvent;  //!
#endif

    void Initialize() override;

   protected:
    Bool_t fWeightHits = false;

    TString fMinMethod = "default";  // Minimization method, default is HeldKarp
    Bool_t fCyclic = false;  // In case you want to find the minimum path using a cyclic loop (e.g. first hit
                             // is connected to last hit)

   public:
    RESTValue GetInputEvent() const override { return fInputTrackEvent; }
    RESTValue GetOutputEvent() const override { return fOutputTrackEvent; }

    void InitProcess() override;
    TRestEvent* ProcessEvent(TRestEvent* inputEvent) override;
    void BruteForce(TRestVolumeHits* hits, std::vector<int>& bestPath);
    void NearestNeighbour(TRestVolumeHits* hits, std::vector<int>& bestPath);
    void HeldKarp(TRestVolumeHits* hits, std::vector<int>& bestPath);
    void EndProcess() override;

    void PrintMetadata() override {
        BeginPrintProcess();

        //           std::cout << "Maximum number of nodes (hits) allowed : " <<
        //           fMaxNodes << endl;

        if (fWeightHits)
            RESTMetadata << "Weight hits : enabled" << RESTendl;
        else
            RESTMetadata << "Weight hits : disabled" << RESTendl;

        RESTMetadata << "Minimization method " << fMinMethod << RESTendl;
        EndPrintProcess();
    }

    const char* GetProcessName() const override { return "trackPathMinimization"; }

    // Constructor
    TRestTrackPathMinimizationProcess();
    // Destructor
    ~TRestTrackPathMinimizationProcess();

    ClassDefOverride(TRestTrackPathMinimizationProcess, 2);
};
#endif
