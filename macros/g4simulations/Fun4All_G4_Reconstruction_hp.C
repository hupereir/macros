#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllDstInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <phool/recoConsts.h>

// own modules
#include <g4eval/EventCounter_hp.h>
#include <g4eval/TrackingEvaluator_hp.h>

// R__ADD_INCLUDE_PATH( /phenix/u/hpereira/sphenix/src/macros/macros/g4simulations )
R__ADD_INCLUDE_PATH( /home/hpereira/sphenix/src/macros/macros/g4simulations )
#include "G4Setup_sPHENIX.C"
#include "G4_Bbc.C"

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libg4testbench.so)

//________________________________________________________________________________________________
int Fun4All_G4_Reconstruction_hp( const int nEvents = 0, const char* inputFile = "DST/dst_sim_5k_nphi1k.root", const char *outputFile = "DST/dst_reco_5k_truth_notpc_nphi1k.root" )
{

  // server
  auto se = Fun4AllServer::instance();
  se->Verbosity(0);

  auto rc = recoConsts::instance();
  rc->set_IntFlag("RANDOMSEED", 1);

  // customize tracking options
  TrackingParameters::use_track_prop = false;

  // event counter
  se->registerSubsystem( new EventCounter_hp() );

  // bbc reconstruction
  BbcInit();
  Bbc_Reco();

  // tracking
  Tracking_Cells();
  Tracking_Reco();

  // local evaluation
  if( false )
  { se->registerSubsystem(new TrackingEvaluator_hp( "TRACKINGEVALUATOR_HP" )); }

  // input manager
  auto in = new Fun4AllDstInputManager("DSTin");
  in->fileopen(inputFile);
  se->registerInputManager(in);

  // output manager
  /* all the nodes from DST and RUN are saved to the output */
  Fun4AllDstOutputManager *out = new Fun4AllDstOutputManager("DSTOUT", outputFile);
  se->registerOutputManager(out);

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
