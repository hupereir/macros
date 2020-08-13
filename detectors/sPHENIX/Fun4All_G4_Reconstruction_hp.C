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

#include "G4_Tracking.C"

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libqa_modules.so)

//________________________________________________________________________________________________
int Fun4All_G4_Reconstruction_hp(
  const int nEvents = 1,
  const char* inputFile = "DST/CONDOR_Hijing_Micromegas_50kHz/G4Hits_merged/G4Hits_sHijing_0-12fm_merged_000000_001000.root",
  const char *outputFile = "DST/Clusters.root" )
{

  // central tracking
  Enable::MVTX = true;
  Enable::INTT = true;
  Enable::TPC = true;
  Enable::TPC_ABSORBER = true;
  Enable::MICROMEGAS = true;

  // tracking configuration
  G4TRACKING::use_track_prop = true;
  G4TRACKING::disable_mvtx_layers = false;
  G4TRACKING::disable_tpc_layers = false;

  // server
  auto se = Fun4AllServer::instance();
  se->Verbosity(1);

  // reco const
  auto rc = recoConsts::instance();

  // event counter
  se->registerSubsystem( new EventCounter_hp( "EventCounter_hp", 1 ) );

  // cells 
  Mvtx_Cells();
  Intt_Cells();
  TPC_Cells();
  Micromegas_Cells();

  // digitizer and clustering
  Mvtx_Clustering();
  Intt_Clustering();
  TPC_Clustering();
  Micromegas_Clustering();
  
  if( false )
  {
    // tracking
    TrackingInit();
    Tracking_Reco();
  }
  
  if( false )
  {
    // local evaluation
    auto simEvaluator = new SimEvaluator_hp;
    simEvaluator->set_flags(
      SimEvaluator_hp::EvalEvent|
      SimEvaluator_hp::EvalVertices|
      SimEvaluator_hp::EvalParticles );
    se->registerSubsystem(simEvaluator);
    
    auto trackingEvaluator = new TrackingEvaluator_hp;
    trackingEvaluator->set_flags(
      TrackingEvaluator_hp::EvalEvent|
      TrackingEvaluator_hp::EvalClusters|
      TrackingEvaluator_hp::EvalTracks);
    se->registerSubsystem(trackingEvaluator);
  }
  
  // input manager
  auto in = new Fun4AllDstInputManager("DSTin");
  in->fileopen(inputFile);
  se->registerInputManager(in);

  // output manager
  /* all the nodes from DST and RUN are saved to the output */
  auto out = new Fun4AllDstOutputManager("DSTOUT", outputFile);
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
