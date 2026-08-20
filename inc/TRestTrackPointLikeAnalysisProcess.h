///______________________________________________________________________________
///______________________________________________________________________________
///______________________________________________________________________________
///
///
///             RESTSoft : Software for Rare Event Searches with TPCs
///
///             TRestTrackPointLikeAnalysisProcess.h
///
///_______________________________________________________________________________

#ifndef RestCore_TRestTrackPointLikeAnalysisProcess
#define RestCore_TRestTrackPointLikeAnalysisProcess

#include <TRestEventProcess.h>

#include "TRestTrackEvent.h"

class TRestTrackPointLikeAnalysisProcess : public TRestEventProcess {
   private:
#ifndef __CINT__
    TRestTrackEvent* fTrackEvent;  //!
                                   // TODO We must get here a pointer to TRestDaqMetadata
                                   // In order to convert the parameters to time using the sampling time
#endif

    void InitFromConfigFile() override;

    void Initialize() override;

    void LoadDefaultConfig();

   protected:
    // add here the members of your event process

   public:
    RESTValue GetInputEvent() const override { return fTrackEvent; }
    RESTValue GetOutputEvent() const override { return fTrackEvent; }

    void InitProcess() override;
    TRestEvent* ProcessEvent(TRestEvent* inputEvent) override;
    void EndProcess() override;

    void LoadConfig(std::string configFilename);

    void PrintMetadata() override {
        BeginPrintProcess();

        EndPrintProcess();
    }

    const char* GetProcessName() const override { return "pointLikeTrackAna"; }
    //       Double_t GetCalibFactor();           ///< Calibration factor is
    //       found.

    // Constructor
    TRestTrackPointLikeAnalysisProcess();
    TRestTrackPointLikeAnalysisProcess(const char* configFilename);
    // Destructor
    ~TRestTrackPointLikeAnalysisProcess();

    ClassDefOverride(TRestTrackPointLikeAnalysisProcess,
                     1);  // Template for a REST "event process" class inherited from
                          // TRestEventProcess
};
#endif
