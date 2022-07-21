#include <TCanvas.h>
#include <TF1.h>
#include <TH1D.h>
#include <TMath.h>
#include <TRestRun.h>
#include <TRestTask.h>
#include <TSystem.h>

#include "TRestTrackEvent.h"

#ifndef RestTask_MaxEnergyTrackCentering
#define RestTask_MaxEnergyTrackCentering

//*******************************************************************************************************
//*** Description: This macro takes the maximum energy track and reads the hits that make it inside the
//*** file.root given by argument and centers them around 0 by subtracting the mean event position to
//*** each individual hit position. It creates three
//*** histograms including events n1 to n2, where n2 is the minimum among the number of events in the run
//*** and the value specified byt he userm and stores them in file.root. One is for the x values in XZ
//*** events (with an 'X' suffix), one for the y values in YZ events (with a 'Y' suffix), and one for the
//*** z values in both XZ and YZ events.
//*** The given number of bins and bin range will be applied to all the histograms.
//*** --------------
//*** The output histograms show the standard shape of the hits event that defines the maximum energy track,
//*** so its symmetry, gaussianity and others can be estimated.
//*** --------------
//*** Usage: restManager MaxEnergyTrackCentering /full/path/file.root [histoName] [firstBin] [lastBin]
//*** [#bins] [firstEvent] [lastEvent]
//*******************************************************************************************************

Int_t REST_Track_MaxEnergyTrackCentering(TString rootFileName, TString histoName, int startVal = -30,
                                         int endVal = 30, int bins = 120, int n1 = 0, int n2 = 60000) {
    TRestStringOutput RESTLog;

    std::vector<string> inputFilesNew = TRestTools::GetFilesMatchingPattern((string)rootFileName);

    TH1D* hX = new TH1D(histoName + "X", histoName + "X", bins, startVal, endVal);
    TH1D* hY = new TH1D(histoName + "Y", histoName + "Y", bins, startVal, endVal);
    TH1D* hZ = new TH1D(histoName + "Z", histoName + "Z", bins, startVal, endVal);

    if (inputFilesNew.size() == 0) {
        RESTLog << "Files not found!" << RESTendl;
        return -1;
    }

    for (unsigned int n = 0; n < inputFilesNew.size(); n++) {
        TRestRun* run = new TRestRun();

        run->OpenInputFile(inputFilesNew[n]);

        // Reading events
        TRestTrackEvent* ev = new TRestTrackEvent();

        run->SetInputEvent(ev);
        int nEntries = run->GetEntries();
        if (nEntries < n2) {
            n2 = nEntries;
        }

        for (int i = n1; i < n2; i++) {
            run->GetEntry(i);

            // Fill in the histogram with position and energy if the coordinate is valid
            // i.e. XZ for X and YZ for Z.

            if (ev->GetMaxEnergyTrack("X")) {
                for (int n = 0; n < ev->GetMaxEnergyTrack("X")->GetHits()->GetNumberOfHits(); n++) {
                    Double_t en = ev->GetMaxEnergyTrack("X")->GetHits()->GetEnergy(n);
                    Double_t valX = ev->GetMaxEnergyTrack("X")->GetHits()->GetX(n);
                    Double_t meanX = ev->GetMaxEnergyTrack("X")->GetHits()->GetMeanPosition().X();
                    Double_t valZ = ev->GetMaxEnergyTrack("X")->GetHits()->GetZ(n);
                    Double_t meanZ = ev->GetMaxEnergyTrack("X")->GetHits()->GetMeanPosition().Z();
                    hX->Fill(valX - meanX, en);
                    hZ->Fill(valZ - meanZ, en);
                    // std::cout << "The number of hits of entry: " << i << " is: " <<
                    // ev->GetMaxEnergyTrack("X")->GetHits()->GetNumberOfHits() << std::endl;
                }
            }

            if (ev->GetMaxEnergyTrack("Y")) {
                for (int n = 0; n < ev->GetMaxEnergyTrack("Y")->GetHits()->GetNumberOfHits(); n++) {
                    Double_t en = ev->GetMaxEnergyTrack("Y")->GetHits()->GetEnergy(n);
                    Double_t valY = ev->GetMaxEnergyTrack("Y")->GetHits()->GetY(n);
                    Double_t meanY = ev->GetMaxEnergyTrack("Y")->GetHits()->GetMeanPosition().Y();
                    Double_t valZ = ev->GetMaxEnergyTrack("Y")->GetHits()->GetZ(n);
                    Double_t meanZ = ev->GetMaxEnergyTrack("Y")->GetHits()->GetMeanPosition().Z();
                    hY->Fill(valY - meanY, en);
                    hZ->Fill(valZ - meanZ, en);
                    // std::cout << "The number of hits of entry: " << i << " is: " <<
                    // ev->GetMaxEnergyTrack("Y")->GetHits()->GetNumberOfHits() << std::endl;
                }
            }
        }

        // cleanup
        delete ev;
        delete run;
    }

    TFile* f = new TFile(rootFileName, "update");
    hX->Write(histoName + "X");
    hY->Write(histoName + "Y");
    hZ->Write(histoName + "Z");
    f->Close();
    RESTLog << "Written histograms " << histoName << "X/Y/Z into " << rootFileName << RESTendl;

    return 0;
};

#endif
