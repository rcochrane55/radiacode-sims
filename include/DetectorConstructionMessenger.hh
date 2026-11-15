#ifndef DetectorConstructionMessenger_h
#define DetectorConstructionMessenger_h 1

#include "G4UImessenger.hh"
#include "globals.hh"
#include "G4ThreeVector.hh"
#include "G4UIcommand.hh"
#include "G4UIdirectory.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIcmdWith3VectorAndUnit.hh"

class DetectorConstruction;
class G4UIcommand;
class G4UIdirectory;

class G4UIcmdWithAString;
class G4UIcmdWithADoubleAndUnit;
class G4UIcmdWith3VectorAndUnit;

class DetectorConstructionMessenger : public G4UImessenger
{
public:
    DetectorConstructionMessenger(DetectorConstruction* detector);
    virtual ~DetectorConstructionMessenger();
    virtual void SetNewValue(G4UIcommand* command, G4String newValue);

private:
    DetectorConstruction* fDetector;

    //directories
    G4UIdirectory* fGeometryDir;
    G4UIdirectory* fDetectorDir;
    G4UIdirectory* fCrystalDir;
    G4UIdirectory* fSiPMDir;
    G4UIdirectory* fReflectorDir;
    G4UIdirectory* fCladdingDir;
    G4UIdirectory* fSourceDir;
    G4UIdirectory* fMarinelliDir;
    G4UIdirectory* fDiskDir;
    
    //detector commands
    G4UIcmdWith3VectorAndUnit* fCrystalDimensionsCmd;
    G4UIcmdWith3VectorAndUnit* fCrystalPositionCmd;

    G4UIcmdWithADoubleAndUnit* fSiPMSideCmd;
    G4UIcmdWithADoubleAndUnit* fSiPMThicknessCmd;

    G4UIcmdWithADoubleAndUnit* fReflectorThicknessCmd;
    G4UIcmdWithADoubleAndUnit* fCladdingThicknessCmd;

    //source commands
    G4UIcmdWithAString* fSourceTypeCmd;
    G4UIcmdWithAString* fMarinelliMeshFileCmd;
    G4UIcmdWith3VectorAndUnit* fMarinelliPositionCmd;
    G4UIcmdWithADoubleAndUnit* fDiskDiameterCmd;
    G4UIcmdWithADoubleAndUnit* fDiskThicknessCmd;
    G4UIcmdWith3VectorAndUnit* fDiskPositionCmd;
    G4UIcmdWith3VectorAndUnit* fDiskRotationCmd;
};

#endif