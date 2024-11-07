///______________________________________________________________________________
///______________________________________________________________________________
///______________________________________________________________________________
///
///
///             RESTSoft : Software for Rare Event Searches with TPCs
///
///             TRestTrackBlobAnalysisProcess.h
///
///_______________________________________________________________________________

#ifndef RestCore_TRestTrackBlobAnalysisProcess
#define RestCore_TRestTrackBlobAnalysisProcess

#include <TRestEventProcess.h>

#include "TRestTrackEvent.h"

class TRestTrackBlobAnalysisProcess : public TRestEventProcess {
   private:
#ifndef __CINT__
    TRestTrackEvent* fInputTrackEvent;   //!
    TRestTrackEvent* fOutputTrackEvent;  //!

    // Q1 and Q2
    std::vector<std::string> fQ1_Observables;  //!
    std::vector<double> fQ1_Radius;            //!

    std::vector<std::string> fQ2_Observables;  //!
    std::vector<double> fQ2_Radius;            //!

    std::vector<std::string> fQ1_X_Observables;  //!
    std::vector<double> fQ1_X_Radius;            //!

    std::vector<std::string> fQ2_X_Observables;  //!
    std::vector<double> fQ2_X_Radius;            //!

    std::vector<std::string> fQ1_Y_Observables;  //!
    std::vector<double> fQ1_Y_Radius;            //!

    std::vector<std::string> fQ2_Y_Observables;  //!
    std::vector<double> fQ2_Y_Radius;            //!
    /////////////////

    // Qhigh and Qlow
    std::vector<std::string> fQhigh_Observables;  //!
    std::vector<double> fQhigh_Radius;            //!

    std::vector<std::string> fQlow_Observables;  //!
    std::vector<double> fQlow_Radius;            //!

    std::vector<std::string> fQhigh_X_Observables;  //!
    std::vector<double> fQhigh_X_Radius;            //!

    std::vector<std::string> fQlow_X_Observables;  //!
    std::vector<double> fQlow_X_Radius;            //!

    std::vector<std::string> fQhigh_Y_Observables;  //!
    std::vector<double> fQhigh_Y_Radius;            //!

    std::vector<std::string> fQlow_Y_Observables;  //!
    std::vector<double> fQlow_Y_Radius;            //!
    /////////////////

    // Qbalance and Qratio
    std::vector<std::string> fQbalance_Observables;  //!
    std::vector<double> fQbalance_Radius;            //!

    std::vector<std::string> fQratio_Observables;  //!
    std::vector<double> fQratio_Radius;            //!

    std::vector<std::string> fQbalance_X_Observables;  //!
    std::vector<double> fQbalance_X_Radius;            //!

    std::vector<std::string> fQratio_X_Observables;  //!
    std::vector<double> fQratio_X_Radius;            //!

    std::vector<std::string> fQbalance_Y_Observables;  //!
    std::vector<double> fQbalance_Y_Radius;            //!

    std::vector<std::string> fQratio_Y_Observables;  //!
    std::vector<double> fQratio_Y_Radius;            //!
                                                     /////////////////

#endif

    Double_t fHitsToCheckFraction;

    void InitFromConfigFile() override;

    void Initialize() override;

    void LoadDefaultConfig();

   protected:
    // add here the members of your event process

   public:
    RESTValue GetInputEvent() const override { return fInputTrackEvent; }
    RESTValue GetOutputEvent() const override { return fOutputTrackEvent; }

    void InitProcess() override;
    TRestEvent* ProcessEvent(TRestEvent* inputEvent) override;
    void EndProcess() override;

    void LoadConfig(const std::string& configFilename, const std::string& name = "");

    void PrintMetadata() override {
        BeginPrintProcess();

        RESTMetadata << " Hits to check factor : " << fHitsToCheckFraction << RESTendl;

        EndPrintProcess();
    }

    const char* GetProcessName() const override { return "findTrackBlobs"; }

    // Constructor
    TRestTrackBlobAnalysisProcess();
    TRestTrackBlobAnalysisProcess(const char* configFilename);
    // Destructor
    ~TRestTrackBlobAnalysisProcess();

    ClassDefOverride(TRestTrackBlobAnalysisProcess, 1);  // Template for a REST "event process" class
                                                         // inherited from TRestEventProcess
};
#endif
