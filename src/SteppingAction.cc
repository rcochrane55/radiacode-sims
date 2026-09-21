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
/// \file SteppingAction.cc
/// \brief Implementation of the SteppingAction class

#include "RunAction.hh"
#include "SteppingAction.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4StepPoint.hh"
#include "G4RunManager.hh"
#include "G4OpticalPhoton.hh"
#include "G4OpBoundaryProcess.hh"
#include "G4ProcessManager.hh"

SteppingAction::SteppingAction(RunAction* runAction)
: G4UserSteppingAction(),
  fRunAction(runAction),
  fBoundaryProcess(nullptr)
{}

SteppingAction::~SteppingAction()
{}

void SteppingAction::UserSteppingAction(const G4Step* step)
{
  G4Track* track = step->GetTrack();

  if (track->GetDefinition() != G4OpticalPhoton::OpticalPhotonDefinition())
  {
    return;
  }

  if (!fBoundaryProcess)
  {
    G4ProcessManager* processManager = G4OpticalPhoton::OpticalPhotonDefinition()->GetProcessManager();

    G4ProcessVector* processList = processManager->GetProcessList();

    for (G4int i=0; i < processManager->GetProcessListLength(); ++i)
    {
      auto process = dynamic_cast<G4OpBoundaryProcess*>((*processList)[i]);

      if (process)
      {
        fBoundaryProcess = process;
        break;
      }
    }
  }

  G4StepPoint* postStep = step->GetPostStepPoint();
  if (postStep->GetStepStatus() != fGeomBoundary)
  {
    return;
  }

  if (fBoundaryProcess && fBoundaryProcess->GetStatus() == Detection)
  {
    fRunAction->AddDetectedPhoton();
  }
}


