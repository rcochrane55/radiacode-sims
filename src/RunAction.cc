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
#include "G4AccumulableManager.hh"
#include <fstream>

RunAction::RunAction()
: G4UserRunAction()
{
    G4AccumulableManager::Instance()->Register(fDetectedPhotons);
    if (IsMaster())
    {
        std::ofstream csvFile("optical_pdes.csv");
        csvFile << "run_id,n_emitted,n_detected,efficiency\n";
    }
}

RunAction::~RunAction()
{}

void RunAction::BeginOfRunAction(const G4Run*)
{ 
  G4RunManager::GetRunManager()->SetRandomNumberStore(false);
G4AccumulableManager::Instance()->Reset();
}

void RunAction::EndOfRunAction(const G4Run* run)
{
  G4int emittedPhotons = run->GetNumberOfEvent();
  if (emittedPhotons == 0)
      return;
  G4AccumulableManager::Instance()->Merge();
  if (IsMaster())
  {
      G4int detectedPhotons = fDetectedPhotons.GetValue();
      G4double efficiency = static_cast<G4double>(detectedPhotons)/static_cast<G4double>(emittedPhotons);
      std::ofstream csvFile("optical_pdes.csv", std::ios::app);
      csvFile
          << run->GetRunID() << ","
          << emittedPhotons << ","
          << detectedPhotons << ","
          << efficiency << "\n";
      csvFile.close();
   }
  // G4cout 
    // << G4endl
    // << "Optical map result:" << G4endl
    // << "  Run ID: " << run->GetRunID() << G4endl
    // << "  Emitted photons: " << emittedPhotons << G4endl
    // << "  Detected Photons: " << fDetectedPhotons << G4endl
    // << "  Efficiency: " << efficiency << G4endl;
}
  
