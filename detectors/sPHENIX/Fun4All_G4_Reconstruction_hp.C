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
  const int nSkipEvents = 131,
  const char* inputFile = "DST/CONDOR_Hijing_Micromegas_50kHz/G4Hits_merged/G4Hits_sHijing_0-12fm_merged_000000_001000.root",
  const char *outputFile = "DST/dst_eval.root" )
{

  // print inputs
  std::cout << "Fun4All_G4_Reconstruction_hp - nEvents: " << nEvents << std::endl;
  std::cout << "Fun4All_G4_Reconstruction_hp - nSkipEvents: " << nSkipEvents << std::endl;
  std::cout << "Fun4All_G4_Reconstruction_hp - inputFile: " << inputFile << std::endl;
  std::cout << "Fun4All_G4_Reconstruction_hp - outputFile: " << outputFile << std::endl;

  // central tracking
  Enable::MVTX = true;
  Enable::INTT = true;
  Enable::TPC = true;
  Enable::TPC_ABSORBER = true;
  Enable::MICROMEGAS = true;

  // TPC
  G4TPC::enable_distortions = true;
  G4TPC::distortion_filename = "distortion_maps/BeamXingNBeamsx10.flat_B1.4_E-400.0.ross_phislice_lookup_r16xp36xz40.distortion_map.hist.root";

  // micromegas configuration
  G4MICROMEGAS::CONFIG = G4MICROMEGAS::CONFIG_Z_ONE_SECTOR;

  // tracking configuration
  G4TRACKING::use_Genfit = true;
  G4TRACKING::use_truth_track_seeding = true;
  G4TRACKING::disable_mvtx_layers = false;
  G4TRACKING::disable_tpc_layers = true;

  // server
  auto se = Fun4AllServer::instance();
  se->Verbosity(1);

  // reco const
  // auto rc = recoConsts::instance();

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

  if( true )
  {
    // tracking
    TrackingInit();
    Tracking_Reco();
  }

  if( true )
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
  if( true )
  {
    out->AddNode("SimEvaluator_hp::Container");
    out->AddNode("TrackingEvaluator_hp::Container");
  }
  se->registerOutputManager(out);

  // skip events if any specified
  if( nSkipEvents > 0 )
  { se->skip( nSkipEvents ); }

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
