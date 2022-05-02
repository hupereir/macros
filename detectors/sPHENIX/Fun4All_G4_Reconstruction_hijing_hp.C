#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllDstInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <phool/PHRandomSeed.h>
#include <phool/recoConsts.h>
#include <qa_modules/QAG4SimulationIntt.h>
#include <qa_modules/QAG4SimulationMvtx.h>
#include <qa_modules/QAG4SimulationTracking.h>
#include <qa_modules/QAHistManagerDef.h>

// own modules
#include <g4eval/EventCounter_hp.h>
#include <g4eval/MicromegasEvaluator_hp.h>
#include <g4eval/SimEvaluator_hp.h>
#include <g4eval/TrackingEvaluator_hp.h>

#include "G4_Magnet.C"
#include "G4_Tracking.C"

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libqa_modules.so)

//________________________________________________________________________________________________
int Fun4All_G4_Reconstruction_hijing_hp(
  const int nEvents = 0,
  const int nSkipEvents = 0,
  const char* inputFile = "/sphenix/sim/sim01/sphnxpro/mdc2/shijing_hepmc/fm_0_20/trkrhit/DST_TRKR_HIT_sHijing_0_20fm_50kHz_bkg_0_20fm-0000000004-00000.root",
  /// const char* inputFile = "DST/CONDOR_hijing_micromegas/trkrcluster/DST_TRKR_CLUSTER_sHijing_0_20fm_50kHz_bkg_0_20fm-0000000004-00000_test.root",
  const char* outputFile = "DST/CONDOR_hijing_micromegas/tracks/DST_TRACKS_sHijing_0_20fm_50kHz_bkg_0_20fm-0000000004-00000_test.root"
  )
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
 
  // TPC configuration
  G4TPC::ENABLE_STATIC_DISTORTIONS = false;
  G4TPC::ENABLE_TIME_ORDERED_DISTORTIONS = false;
  G4TPC::DO_HIT_ASSOCIATION = false;

//   // enable empty correction
//   /* right now this produces a crash in PHTpcTrackSeedCircleFit */
//   G4TPC::ENABLE_CORRECTIONS = true;
//   G4TPC::correction_filename = "distortion_maps_rec/distortion_corrections_empty.root";  
  
  // tracking configuration
  G4TRACKING::use_full_truth_track_seeding = false;

  // server
  auto se = Fun4AllServer::instance();
  se->Verbosity(1);

  // make sure to printout random seeds for reproducibility
  PHRandomSeed::Verbosity(1);

  // reco const
  auto rc = recoConsts::instance();
  rc->set_IntFlag("RANDOMSEED",PHRandomSeed());

  // event counter
  se->registerSubsystem( new EventCounter_hp( "EventCounter_hp", 10 ) );

  if( false )
  {
    // cells
    Mvtx_Cells();
    Intt_Cells();
    TPC_Cells();
    Micromegas_Cells();
  }

  if( true )
  {
    // tracking init is needed for clustering
    MagnetFieldInit();
    TrackingInit();
  }
  
  if( true )
  {
    // clustering
    Mvtx_Clustering();
    Intt_Clustering();
    TPC_Clustering();
    Micromegas_Clustering();
  }
  
  if( true )
  {
    // tracking
    Tracking_Reco();
  }
  
  if( false )
  {
    // sim event evaluation
    auto simEvaluator = new SimEvaluator_hp;
    simEvaluator->set_flags(
      SimEvaluator_hp::EvalEvent
      |SimEvaluator_hp::EvalVertices
      |SimEvaluator_hp::EvalParticles
      );
    se->registerSubsystem(simEvaluator);
  }
  
  if( false )
  {
    // tracking evaluation
    auto trackingEvaluator = new TrackingEvaluator_hp;
    trackingEvaluator->set_flags(
      TrackingEvaluator_hp::EvalEvent
//       |TrackingEvaluator_hp::EvalClusters
      |TrackingEvaluator_hp::EvalTracks
      );
    se->registerSubsystem(trackingEvaluator);
  }
  
  // input manager
  auto in = new Fun4AllDstInputManager("DSTin");
  in->fileopen(inputFile);
  se->registerInputManager(in);

  // output manager
  /* all the nodes from DST and RUN are saved to the output */
  auto out = new Fun4AllDstOutputManager("DSTOUT", outputFile);
  out->AddNode("TRKR_CLUSTER");
  out->AddNode("SvtxTrackMap");
  out->AddNode("SvtxVertexMap");
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
