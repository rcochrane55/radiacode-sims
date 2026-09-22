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

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

RunAction::RunAction()
: G4UserRunAction(), fDetectedPhotons(0)
{ 
  /* // Register accumulable to the accumulable manager
  G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
  accumulableManager->RegisterAccumulable(fEdep);
  accumulableManager->RegisterAccumulable(fEdep2); 

  // Create analysis manager and 1 histo
  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
  analysisManager->SetVerboseLevel(1);
  analysisManager->SetFirstHistoId(1);
  analysisManager->SetNtupleMerging(true);
  analysisManager->CreateH1("name","title", 1000, 0., 20.*keV);

  analysisManager->CreateNtuple("t", "Edep");
  analysisManager->CreateNtupleDColumn("RawEdep");
  analysisManager->CreateNtupleDColumn("fStepEnergy", fStepEnergy);
  analysisManager->CreateNtupleDColumn("DetectedPhotons");
  analysisManager ->CreateNtupleDColumn("fStepX", fStepX);
  analysisManager ->CreateNtupleDColumn("fStepY", fStepY);
  analysisManager ->CreateNtupleDColumn("fStepZ", fStepZ);
  analysisManager->FinishNtuple(0); */
}

RunAction::~RunAction()
{}

void RunAction::BeginOfRunAction(const G4Run*)
{ 
  // inform the runManager to save random number seed
  G4RunManager::GetRunManager()->SetRandomNumberStore(false);
  fDetectedPhotons = 0;
  // reset accumulables to their initial values
/*   G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
  accumulableManager->Reset(); */

  // Get analysis manager
  /* G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
  G4String fileName="rootOutput";
  
  if(!analysisManager->GetFileName().empty())
    fileName = analysisManager->GetFileName();
  
  analysisManager->OpenFile(fileName);
 */
}

/* void RunAction::AddStep(G4double edep, const G4ThreeVector& pos)
{
  fStepEnergy.push_back(edep);
  fStepX.push_back(pos.x());
  fStepY.push_back(pos.y());
  fStepZ.push_back(pos.z());
}

void RunAction::ClearSteps()
{
  fStepEnergy.clear();
  fStepX.clear();
  fStepY.clear();
  fStepZ.clear();
} */

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
  
  /* G4int nofEvents = run->GetNumberOfEvent();
  if (nofEvents == 0) return;

  // Merge accumulables 
  G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
  accumulableManager->Merge();

  // Compute dose = total energy deposit in a run and its variance
  //
  G4double edep  = fEdep.GetValue();
  G4double edep2 = fEdep2.GetValue();
  
  G4double rms = edep2 - edep*edep/nofEvents;
  if (rms > 0.) rms = std::sqrt(rms); else rms = 0.;  

  const DetectorConstruction* detectorConstruction
   = static_cast<const DetectorConstruction*>
     (G4RunManager::GetRunManager()->GetUserDetectorConstruction());
  G4double mass = detectorConstruction->GetScoringVolume()->GetMass();
  G4double dose = edep/mass;
  G4double rmsDose = rms/mass;

  // Run conditions
  //  note: There is no primary generator action object for "master"
  //        run manager for multi-threaded mode.
  const PrimaryGeneratorAction* generatorAction
   = static_cast<const PrimaryGeneratorAction*>
     (G4RunManager::GetRunManager()->GetUserPrimaryGeneratorAction());
  G4String runCondition;
  if (generatorAction)
  {
    //const G4ParticleGun* particleGun = generatorAction->GetParticleGun();
    //runCondition += particleGun->GetParticleDefinition()->GetParticleName();
    //runCondition += " of ";
    //G4double particleEnergy = particleGun->GetParticleEnergy();
    //runCondition += G4BestUnit(particleEnergy,"Energy");
  } */

        
  // Print
  //  
/*   if (IsMaster()) {
    G4cout
     << G4endl
     << "--------------------End of Global Run-----------------------";
  }
  else {
    G4cout
     << G4endl
     << "--------------------End of Local Run------------------------";
  } */

/* // print histogram statistics
  G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();

  // save histograms
  analysisManager->Write();
  analysisManager->CloseFile(); */




/* void RunAction::AddEdep(G4double edep)
{
  fEdep  += edep;
  fEdep2 += edep*edep;
}
 */
