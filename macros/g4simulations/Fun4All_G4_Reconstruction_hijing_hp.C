#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllDstInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <phool/recoConsts.h>

// own modules
#include <g4eval/EventCounter_hp.h>
#include <g4eval/TrackingEvaluator_hp.h>

R__ADD_INCLUDE_PATH( /phenix/u/hpereira/sphenix/src/macros/macros/g4simulations )
#include "G4Setup_sPHENIX.C"
#include "G4_Bbc.C"

R__LOAD_LIBRARY(libfun4all.so)

//________________________________________________________________________________________________
int Fun4All_G4_Reconstruction_hijing_hp(
  const int nEvents = 1,
  const char *inputFile = "/sphenix/user/bogui/MacrosModular/macros/macros/g4simulations/clus_only/SvtxCluHijMBPu100_Mar20_1_1001.root",
  const char *outputFile = "DST/dst_eval_hijing.root" )
{

  // customize tpc
  Tpc::misalign_tpc_clusters = false;

  // customize track finding
  TrackingParameters::use_track_prop = true;
  TrackingParameters::disable_tpc_layers = true;

  // server
  auto se = Fun4AllServer::instance();
  se->Verbosity(1);

  auto rc = recoConsts::instance();
  rc->set_IntFlag("RANDOMSEED", 1);

  // event counter
  se->registerSubsystem( new EventCounter_hp( "EventCounter_hp", 10 ) );

  // bbc reconstruction
  // BbcInit();
  // Bbc_Reco();

  // tracking
  // Tracking_Cells();
  // Tracking_Clus();
  Tracking_Reco();

  // local evaluation
  se->registerSubsystem(new TrackingEvaluator_hp( "TrackingEvaluator_hp" ) );

  // input manager
  auto in = new Fun4AllDstInputManager("DSTin");
  in->fileopen(inputFile);
  se->registerInputManager(in);

  // output manager
  /* all the nodes from DST and RUN are saved to the output */
  auto out = new Fun4AllDstOutputManager("DSTOUT", outputFile);
  out->AddNode("TrackingEvaluator_hp::Container");
  se->registerOutputManager(out);

  // process events
  se->run(nEvents);

  // terminate
  se->End();
  se->PrintTimer();
  std::cout << "All done" << std::endl;
  delete se;
  gSystem->Exit(0);
  return 0;
}

// This function is only used to test if we can load this as root6 macro
// without running into unresolved libraries and include files
void RunFFALoadTest() {}
