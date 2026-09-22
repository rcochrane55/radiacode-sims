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

EventAction::EventAction(RunAction* runAction)
: G4UserEventAction(),
  fRunAction(runAction),
  fRawEdep(0.)
{} 

EventAction::~EventAction()
{}

void EventAction::AddStep(G4double edep, const G4ThreeVector& pos)
{
  fRawEdep += edep;
}

void EventAction::BeginOfEventAction(const G4Event*)
{    
  fRawEdep = 0.;
  fDetectedPhotons = 0;
}

void EventAction::EndOfEventAction(const G4Event*)
{   
  G4double E = fRawEdep / keV;

  const G4double a = -1252.39;
  const G4double b = 8.390725;
  const G4double c = -0.00205;
  
  G4double fwhm2 = a + b*E + c*E*E;  // FWHM in keV

  if (fwhm2 < 0.1)
    fwhm2 = 0.1;

  G4double fwhm = std::sqrt(fwhm2);
  G4double sigma = fwhm/2.35;

  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();

  analysisManager->FillNtupleDColumn(0, fRawEdep);
  analysisManager->FillNtupleDColumn(1, fDetectedPhotons);

  analysisManager->AddNtupleRow();
  }
