#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllDstInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <phool/recoConsts.h>
#include <trackreco/PHGenFitTrkFitter.h>

// own modules
#include <g4eval/EventCounter_hp.h>
#include <g4eval/SimEvaluator_hp.h>
#include <g4eval/TrackingEvaluator_hp.h>

#include "G4_Magnet.C"
#include "G4_Tracking.C"

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libqa_modules.so)

//_________________________________________________________________________
int Fun4All_G4_Evaluation_hp(
    const int nEvents = 1,
    const char* inputFile = "DST/DST_TRACKS_sHijing_0_20fm_50kHz_bkg_0_20fm-0000000002-00000.root",
    const char *outputFile = "DST/dst_eval.root" )
{
  
  // central tracking
  Enable::MVTX = true;
  Enable::INTT = true;
  Enable::TPC = true;
  Enable::TPC_ABSORBER = true;
  Enable::MICROMEGAS = true;

  // tracking configuration
  G4TRACKING::use_full_truth_track_seeding = false;
  
  // server
  auto se = Fun4AllServer::instance();
  se->Verbosity(1);

  auto rc = recoConsts::instance();
  rc->set_IntFlag("RANDOMSEED", 1);

  // event counter
  se->registerSubsystem(new EventCounter_hp("EVENTCOUNTER_HP",1));

  // tracking init is needed for clustering
  MagnetFieldInit();
  TrackingInit();

  // local evaluation
  if( false )
  {
    auto simEvaluator = new SimEvaluator_hp;
    simEvaluator->set_flags(
      SimEvaluator_hp::EvalEvent
      |SimEvaluator_hp::EvalVertices
      // |SimEvaluator_hp::EvalParticles 
      );
    se->registerSubsystem(simEvaluator);
  }
  
  if( true )
  {
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
  auto out = new Fun4AllDstOutputManager("DSTOUT", outputFile );
  out->AddNode("SimEvaluator_hp::Container");
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
