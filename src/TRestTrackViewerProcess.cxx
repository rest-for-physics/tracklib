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

//////////////////////////////////////////////////////////////////////////
/// The TRestTrackViewerProcess allows to display tracks
/// registered inside a TRestTrackEvent.
///
/// <hr>
///
/// \warning **⚠ REST is under continous development.** This
/// documentation
/// is offered to you by the REST community. Your HELP is needed to keep this
/// code
/// up to date. Your feedback will be worth to support this software, please
/// report
/// any problems/suggestions you may find while using it at [The REST Framework
/// forum](http://ezpc10.unizar.es). You are welcome to contribute fixing typos,
/// updating
/// information or adding/proposing new contributions. See also our
/// <a href="https://github.com/rest-for-physics/framework/blob/master/CONTRIBUTING.md">Contribution
/// Guide</a>.
///
///
///--------------------------------------------------------------------------
///
/// RESTsoft - Software for Rare Event Searches with TPCs
///
/// \class      TRestTrackViewerProcess
/// \author     Juanan Garcia
///
/// <hr>
///
#include "TRestTrackViewerProcess.h"

ClassImp(TRestTrackViewerProcess);

///////////////////////////////////////////////
/// \brief Default constructor
///
TRestTrackViewerProcess::TRestTrackViewerProcess() { Initialize(); }

///////////////////////////////////////////////
/// \brief Default destructor
///
TRestTrackViewerProcess::~TRestTrackViewerProcess() {}

///////////////////////////////////////////////
/// \brief Function to initialize input/output event members and define the
/// section name
///
void TRestTrackViewerProcess::Initialize() {
    SetSectionName(this->ClassName());
    SetLibraryVersion(LIBRARY_VERSION);

    fTrackEvent = nullptr;
    fSingleThreadOnly = true;

}

///////////////////////////////////////////////
/// \brief Process initialization. It creates the canvas available in
/// TRestEventProcess
///
void TRestTrackViewerProcess::InitProcess() {
  CreateCanvas();
  if (!hCanvas && fDrawHits)hCanvas = new TCanvas("hitsCanvas","hitsCanvas",fCanvasSize.X(), fCanvasSize.Y());
}

///////////////////////////////////////////////
/// \brief The main processing event function
///
TRestEvent* TRestTrackViewerProcess::ProcessEvent(TRestEvent* evInput) {

  // no need for verbose copy now
  fTrackEvent = (TRestTrackEvent*)evInput;

  fCanvas->cd();
  TPad *pad = fTrackEvent->DrawEvent();
  if(fDrawOriginEnd){
    GetOriginEnd(fAnalysisTree, originGr, endGr, legOrEnd);
    DrawOriginEnd(pad, originGr, endGr, legOrEnd);
  }

  fCanvas->cd();
  pad->Draw();
  fCanvas->Update();

  if(!fDrawHits)return fTrackEvent;

  hCanvas->cd();
  TPad *hPad = fTrackEvent->DrawHits();
    if(fDrawOriginEnd){
      DrawOriginEnd(hPad, originGr, endGr, legOrEnd);
    }

  hCanvas->cd();
  hPad->Draw();
  hCanvas->Update();

  return fTrackEvent;
}

///////////////////////////////////////////////
/// \brief Get origin and end of the track if trackLineAnalysis have
/// been performed
///
void TRestTrackViewerProcess::GetOriginEnd(TRestAnalysisTree *anaTree, std::vector<TGraph *>  &origin, std::vector<TGraph *>  &end, std::vector<TLegend *> &leg){

  for(auto gr : origin)
    if(gr)delete gr;

  for(auto gr : end)
    if(gr)delete gr;

  for(auto l : leg)
    if(l)delete l;

  std::vector <double> originV(3,0);
  std::vector<std::string> oName = {"originX","originY","originZ"}; 
  std::vector <double> endV(3,0);
  std::vector<std::string> eName = {"endX","endY","endZ"}; 

    for(int i=0;i<3;i++){
      std::string fName = "alphaTrackAna_"+oName[i];
        if(anaTree->GetObservableID(fName) ==-1){
          warning<<"Observable "<< fName <<"not found\n";
          return;
        }
      originV[i] = anaTree->GetObservableValue<Double_t>(fName);
      fName = "alphaTrackAna_"+eName[i];
        if(anaTree->GetObservableID(fName) ==-1){
          warning<<"Observable "<< fName <<"not found\n";
          return;
        }
      endV[i] = anaTree->GetObservableValue<Double_t>(fName);
    }

    for(int i=0;i<2;i++){
      origin[i] = new TGraph();
      origin[i]->SetPoint(0,originV[i],originV[2]);
      origin[i]->SetMarkerColor(kRed);
      origin[i]->SetMarkerStyle(20);
      end[i] = new TGraph();
      end[i]->SetPoint(0,endV[i],endV[2]);
      end[i]->SetMarkerColor(kBlack);
      end[i]->SetMarkerStyle(20);
      leg[i] = new TLegend(0.7,0.7,0.9,0.9);
      leg[i]->AddEntry(origin[i],"Origin","p");
      leg[i]->AddEntry(end[i],"End","p");
    }

}
///////////////////////////////////////////////
/// \brief Draw origin and end of the track if trackLineAnalysis have
/// been performed
///
void TRestTrackViewerProcess::DrawOriginEnd(TPad *pad, std::vector<TGraph *> &origin, std::vector<TGraph *> &end, std::vector<TLegend *> &leg){

    for(int i=0;i<2;i++){
      pad->cd(i+1);
        if(origin[i])
          origin[i]->Draw("LP");
        if(end[i])
          end[i]->Draw("LP");
        if(leg[i])
          leg[i]->Draw();
      pad->Update();
    }
  
}

///////////////////////////////////////////////
/// \brief Function to include required actions after all events have been
/// processed.
///
void TRestTrackViewerProcess::EndProcess() {

}
