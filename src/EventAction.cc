//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
/// \file EventAction.cc
/// \brief Implementation of the EventAction class

#include <cmath>

#include "G4SystemOfUnits.hh"

#include "EventAction.hh"
#include "RunAction.hh"

#include "G4Event.hh"
#include "G4RunManager.hh"

#include "G4AnalysisManager.hh"
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

EventAction::EventAction(RunAction* runAction)
: G4UserEventAction(),
  fRunAction(runAction),
  fRawEdep(0.),
  fSmearedEdep(0.)
{} 

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

EventAction::~EventAction()
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::BeginOfEventAction(const G4Event*)
{    
  fRawEdep = 0.;
  fSmearedEdep = 0.;

  fFirstInteractionRecorded = false;

  fFirstX = 0.;
  fFirstY = 0.;
  fFirstZ = 0.;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void EventAction::EndOfEventAction(const G4Event*)
{   
  // accumulate statistics in run action
  //fRunAction->AddEdep(fRawEdep);

  //fEdep = G4RandGauss::shoot(fEdep,fEdep*0.05);  // ENERGY RESOLUTION OF 5%
  // 2020-11-16, discussion with Christian: final result is FWHM resolution
  // of 200.4 
  // --> sigma = 200.4/2.35 = 85.28  

  G4double E = fRawEdep / keV;

  const G4double a = -1252.39;
  const G4double b = 8.390725;
  const G4double c = -0.00205;
  
  G4double fwhm2 = a + b*E + c*E*E;  // FWHM in keV

  if (fwhm2 < 0.1)
    fwhm2 = 0.1;

  G4double fwhm = std::sqrt(fwhm2);
  G4double sigma = fwhm/2.35;

  fSmearedEdep = G4RandGauss::shoot(E,sigma) * keV;  // % energy resolution

  if (fSmearedEdep < 0.)
    fSmearedEdep = 0.;

//  G4cout << "Raw = " << fRawEdep/keV
//         << " keV,  Smeared = " << fSmearedEdep/keV
//         << " keV" << G4endl;

  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();

  if (fSmearedEdep < 1.0*keV)
    return;
  
  // filling ntuple only when there IS an energy deposit
  if (fSmearedEdep > 0) {
    analysisManager->FillH1(1, fSmearedEdep);

    analysisManager->FillNtupleDColumn(0, fRawEdep);
    analysisManager->FillNtupleDColumn(1, fSmearedEdep);
    analysisManager ->FillNtupleDColumn(2, fFirstX);
    analysisManager ->FillNtupleDColumn(3, fFirstY);
    analysisManager ->FillNtupleDColumn(4, fFirstZ);

    analysisManager->AddNtupleRow();
  }

}
