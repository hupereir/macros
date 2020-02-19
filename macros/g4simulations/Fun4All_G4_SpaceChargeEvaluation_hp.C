#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllDstInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <phool/recoConsts.h>

// own modules
#include <g4eval/EventCounter_hp.h>
#include <g4eval/SpaceChargeEvaluator_hp.h>

// R__ADD_INCLUDE_PATH( /phenix/u/hpereira/sphenix/src/macros/macros/g4simulations )
R__ADD_INCLUDE_PATH( /home/hpereira/sphenix/src/macros/macros/g4simulations )
#include "G4Setup_sPHENIX.C"
#include "G4_Bbc.C"

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libg4testbench.so)

//________________________________________________________________________________________________
int Fun4All_G4_SpaceChargeEvaluation_hp( const int nEvents = 50,
const char* inputFile = "/sphenix/sim/sim01/sphnxpro/Geant4-10.05.p01/fm_0-12/FTFP_BERT_HP/G4Hits_sHijing_0-12fm_00000_00050.root",
const char* spaceChargeMapFile = "SpaceCharge/spacechargemap_%05i.root",
const int offset = 0
)
{

  // server
  auto se = Fun4AllServer::instance();
  se->Verbosity(0);

  auto rc = recoConsts::instance();
  rc->set_IntFlag("RANDOMSEED", 1);

  // customize tracking options
  TrackingParameters::use_track_prop = false;

  // event counter
  se->registerSubsystem( new EventCounter_hp( "EVENTCOUNTER_HP", 1 ) );

  auto spaceChargeEvaluator_hp = new SpaceChargeEvaluator_hp;
  spaceChargeEvaluator_hp->set_basefilename( spaceChargeMapFile );
  spaceChargeEvaluator_hp->set_offset(offset);
  se->registerSubsystem( spaceChargeEvaluator_hp );

  // input manager
  auto in = new Fun4AllDstInputManager("DSTin");
  in->fileopen(inputFile);
  se->registerInputManager(in);

  // process events
  se->run(nEvents);

  // terminate
  se->End();
  std::cout << "All done" << std::endl;
  delete se;
  gSystem->Exit(0);
  return 0;
}

// This function is only used to test if we can load this as root6 macro
// without running into unresolved libraries and include files
void RunFFALoadTest() {}
