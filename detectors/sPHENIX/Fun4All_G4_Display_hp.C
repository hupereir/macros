#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllDummyInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <g4main/PHG4SimpleEventGenerator.h>
#include <phool/recoConsts.h>

// local macros
#include "G4Setup_sPHENIX.C"
#include "DisplayOn.C"

R__LOAD_LIBRARY(libfun4all.so)

//____________________________________________________________________
int Fun4All_G4_Display_hp( const int nEvents = 1 )
{

  // options
  Enable::PIPE = false;
  // Enable::BBC = true;
  Enable::BBCFAKE = false;
  Enable::MAGNET = false;
  Enable::PLUGDOOR = false;

  // enable all absorbers
  // this is equivalent to the old "absorberactive" flag
  Enable::ABSORBER = false;

  // central tracking
  Enable::CEMC = false;
  Enable::HCALOUT = false;
  Enable::HCALIN = false;
  Enable::MVTX = false;

  Enable::INTT = true;
  Enable::INTT_ABSORBER = false;
  Enable::INTT_SUPPORT = false;
  
  Enable::TPC = false;
  Enable::TPC_ENDCAP = false;
  Enable::MICROMEGAS = true;
  Enable::BLACKHOLE = false;

  // server
  auto se = Fun4AllServer::instance();

  // Geant4 initialization
  G4Init();
  G4Setup();

  auto g4reco = DisplayOn();
  g4reco->Dump_G4_GDML("sPHENIX_TPOT_INTT.gdml");

  // QTGui();    
  return 0;
}

// This function is only used to test if we can load this as root6 macro
// without running into unresolved libraries and include files
void RunFFALoadTest() {}
