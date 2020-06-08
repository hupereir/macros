#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllDstInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <phool/recoConsts.h>
#include <qa_modules/QAG4SimulationIntt.h>
#include <qa_modules/QAG4SimulationMvtx.h>
#include <qa_modules/QAG4SimulationTracking.h>
#include <qa_modules/QAHistManagerDef.h>

// own modules
#include <g4eval/EventCounter_hp.h>
#include <g4eval/SimEvaluator_hp.h>
#include <g4eval/TrackingEvaluator_hp.h>

R__ADD_INCLUDE_PATH( /phenix/u/hpereira/sphenix/src/macros/macros/g4simulations )
#include "G4Setup_sPHENIX.C"
#include "G4_Bbc.C"

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libqa_modules.so)

//________________________________________________________________________________________________
int Fun4All_G4_Reconstruction_hp(
  const int nEvents = 10,
  const char* inputFile = "DST/dst_sim.root",
  const char *outputFile = "DST/dst_eval.root" )
{

  // customize tpc
  Tpc::enable_tpc_distortions = false;
  Tpc::misalign_tpc_clusters = false;

  // customize outer tracker
  OuterTracker::n_outertrack_layers = 0;

  // enable micromegas
  Micromegas::add_micromegas = true;

  // customize track finding
  TrackingParameters::use_track_prop = true;
  TrackingParameters::disable_tpc_layers = true;
  TrackingParameters::disable_outertracker_layers = true;
  TrackingParameters::use_single_outertracker_layer = false;

  // qa
  const bool do_qa = false;

  // server
  auto se = Fun4AllServer::instance();

  auto rc = recoConsts::instance();
  rc->set_IntFlag("RANDOMSEED", 1);

  // event counter
  se->registerSubsystem( new EventCounter_hp( "EventCounter_hp", 10 ) );

  // tracking
  Tracking_Cells();
  Tracking_Clus();
  // Tracking_Reco();

  // local evaluation
  se->registerSubsystem(new SimEvaluator_hp);

  auto trackingEvaluator = new TrackingEvaluator_hp;
  trackingEvaluator->set_flags(
    TrackingEvaluator_hp::EvalEvent|
    TrackingEvaluator_hp::EvalClusters|
    TrackingEvaluator_hp::EvalTracks);
  se->registerSubsystem(trackingEvaluator);

  // QA modules
  if( do_qa )
  {
    se->registerSubsystem( new QAG4SimulationIntt );
    se->registerSubsystem( new QAG4SimulationMvtx );
    se->registerSubsystem( new QAG4SimulationTracking );
  }

  // input manager
  auto in = new Fun4AllDstInputManager("DSTin");
  in->fileopen(inputFile);
  se->registerInputManager(in);

  // output manager
  /* all the nodes from DST and RUN are saved to the output */
  auto out = new Fun4AllDstOutputManager("DSTOUT", outputFile);
  out->AddNode("SimEvaluator_hp::Container");
  out->AddNode("TrackingEvaluator_hp::Container");
  se->registerOutputManager(out);

  // process events
  se->run(nEvents);

  // QA
  if( do_qa )
  {
    const char *qaFile= "QA/qa_output.root";
    QAHistManagerDef::saveQARootFile(qaFile);
  }

  // terminate
  se->PrintTimer();
  se->End();
  std::cout << "All done" << std::endl;
  delete se;
  gSystem->Exit(0);
  return 0;
}

// This function is only used to test if we can load this as root6 macro
// without running into unresolved libraries and include files
void RunFFALoadTest() {}
