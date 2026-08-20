//////////////////////////////////////////////////////////////////////////
///
///
///             RESTSoft : Software for Rare Event Searches with TPCs
///
///             TRestTrackDetachIsolatedNodesProcess.h
///
///              Jan 2016 : Javier Galan
///
//////////////////////////////////////////////////////////////////////////

#ifndef RestCore_TRestTrackDetachIsolatedNodesProcess
#define RestCore_TRestTrackDetachIsolatedNodesProcess

#include <TRestEventProcess.h>

#include "TRestTrackEvent.h"

class TRestTrackDetachIsolatedNodesProcess : public TRestEventProcess {
   private:
#ifndef __CINT__
    TRestTrackEvent* fInputTrackEvent;   //!
    TRestTrackEvent* fOutputTrackEvent;  //!
#endif

    void InitFromConfigFile() override;

    void Initialize() override;

    Double_t fThresholdDistance;
    Double_t fConnectivityThreshold;

    Double_t fTubeLengthReduction;
    Double_t fTubeRadius;

   protected:
   public:
    RESTValue GetInputEvent() const override { return fInputTrackEvent; }
    RESTValue GetOutputEvent() const override { return fOutputTrackEvent; }

    void InitProcess() override;
    TRestEvent* ProcessEvent(TRestEvent* inputEvent) override;
    void EndProcess() override;
    void LoadDefaultConfig();

    void LoadConfig(const std::string& configFilename, const std::string& name = "");

    void PrintMetadata() override {
        BeginPrintProcess();

        EndPrintProcess();
    }

    const char* GetProcessName() const override { return "trackDetachIsolatedNode"; }

    // Constructor
    TRestTrackDetachIsolatedNodesProcess();
    TRestTrackDetachIsolatedNodesProcess(const char* configFilename);
    // Destructor
    ~TRestTrackDetachIsolatedNodesProcess();

    ClassDefOverride(TRestTrackDetachIsolatedNodesProcess, 1);
};
#endif
