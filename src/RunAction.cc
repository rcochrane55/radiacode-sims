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
/// \file RunAction.cc
/// \brief Implementation of the RunAction class

#include "RunAction.hh"
// #include "Run.hh"

#include "G4RunManager.hh"
#include "G4Run.hh"

RunAction::RunAction()
: G4UserRunAction(), fDetectedPhotons(0)
{}

RunAction::~RunAction()
{}

void RunAction::BeginOfRunAction(const G4Run*)
{ 
  // inform the runManager to save random number seed
  G4RunManager::GetRunManager()->SetRandomNumberStore(false);
  fDetectedPhotons = 0;
}

void RunAction::EndOfRunAction(const G4Run* run)
{
  G4int emittedPhotons = run->GetNumberOfEvent();

  if (emittedPhotons == 0)
    return;

  G4double efficiency = static_cast<G4double>(fDetectedPhotons)/static_cast<G4double>(emittedPhotons);

  G4cout 
    << G4endl
    << "Optical map result:" << G4endl
    << "  Run ID: " << run->GetRunID() << G4endl
    << "  Emitted photons: " << emittedPhotons << G4endl
    << "  Detected Photons: " << fDetectedPhotons << G4endl
    << "  Efficiency: " << efficiency << G4endl;
}
  
