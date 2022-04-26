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

#include <TRestTrackEvent.h>

#include "TRestEventProcess.h"

#ifndef __CINT__
#ifndef WIN32
#include <trackMinimization.h>
#endif
#endif

class TRestTrackPathMinimizationProcess : public TRestEventProcess {
   private:
#ifndef __CINT__
    TRestTrackEvent* fInputTrackEvent;   //!
    TRestTrackEvent* fOutputTrackEvent;  //!
#endif

    void Initialize();

   protected:
    Bool_t fWeightHits = false;

    TString fMinMethod = "default";  // Minimization method, default is HeldKarp
    Bool_t fCyclic = false;  // In case you want to find the minimum path using a cyclic loop (e.g. first hit
                             // is connected to last hit)

   public:
    inline any GetInputEvent() const { return fInputTrackEvent; }
    inline any GetOutputEvent() const { return fOutputTrackEvent; }

    void InitProcess();
    TRestEvent* ProcessEvent(TRestEvent* eventInput);
    void BruteForce(TRestVolumeHits* hits, std::vector<int>& bestPath);
    void NearestNeighbour(TRestVolumeHits* hits, std::vector<int>& bestPath);
    void HeldKarp(TRestVolumeHits* hits, std::vector<int>& bestPath);
    void EndProcess();

    void PrintMetadata() {
        BeginPrintProcess();

        //           std::cout << "Maximum number of nodes (hits) allowed : " <<
        //           fMaxNodes << endl;

        if (fWeightHits)
            metadata << "Weight hits : enabled" << endl;
        else
            metadata << "Weight hits : disabled" << endl;

        metadata << "Minimization method " << fMinMethod << endl;
        EndPrintProcess();
    }

    inline TString GetProcessName() const { return (TString) "trackPathMinimization"; }

    // Constructor
    TRestTrackPathMinimizationProcess();
    // Destructor
    ~TRestTrackPathMinimizationProcess();

    ClassDef(TRestTrackPathMinimizationProcess, 2);
};
#endif
