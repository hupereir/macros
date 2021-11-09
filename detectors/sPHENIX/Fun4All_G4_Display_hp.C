#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllDummyInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <g4main/PHG4SimpleEventGenerator.h>
#include <phool/recoConsts.h>
// local macros
#include "G4Setup_sPHENIX.C"
#include "G4_Bbc.C"
#include "G4_Global.C"
#include "DisplayOn.C"

R__LOAD_LIBRARY(libfun4all.so)

//____________________________________________________________________
int Fun4All_G4_Display_hp( const int nEvents = 1 )
{

  // options
  Enable::PIPE = true;
  // Enable::BBC = true;
  Enable::BBCFAKE = true;
  Enable::MAGNET = true;
  Enable::PLUGDOOR = false;

  // enable all absorbers
  // this is equivalent to the old "absorberactive" flag
  Enable::ABSORBER = true;

  // central tracking
  Enable::MVTX = true;
  Enable::MVTX_SERVICE = true;
  Enable::INTT = true;
  Enable::TPC = true;
  Enable::MICROMEGAS = true;
  Enable::BLACKHOLE = true;

  // server
  auto se = Fun4AllServer::instance();

  // Geant4 initialization
  G4Init();
  G4Setup();

  DisplayOn();

  gROOT->ProcessLine("Fun4AllServer *se = Fun4AllServer::instance();");
  gROOT->ProcessLine("PHG4Reco *g4 = (PHG4Reco *) se->getSubsysReco(\"PHG4RECO\");");
  
  cout << "-------------------------------------------------" << endl;
  cout << "You are in event display mode. Run one event with" << endl;
  cout << "se->run(1)" << endl;
  cout << "Run Geant4 command with following examples" << endl;
  gROOT->ProcessLine("displaycmd()");
    
  return 0;
}

// This function is only used to test if we can load this as root6 macro
// without running into unresolved libraries and include files
void RunFFALoadTest() {}
