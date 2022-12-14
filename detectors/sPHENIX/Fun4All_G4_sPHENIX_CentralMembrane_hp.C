#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllDummyInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <g4main/PHG4SimpleEventGenerator.h>
#include <phool/PHRandomSeed.h>
#include <phool/recoConsts.h>

// own modules
#include <g4eval/EventCounter_hp.h>
#include <g4eval/SimEvaluator_hp.h>
#include <g4eval/MicromegasEvaluator_hp.h>
#include <g4eval/TrackingEvaluator_hp.h>

#include <g4eval/TrackEvaluation.h>

#include <tpccalib/TpcSpaceChargeReconstruction.h>

// local macros
#include "G4Setup_sPHENIX.C"
#include "G4_Bbc.C"
#include "G4_Global.C"
#include "G4_Tracking.C"

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libqa_modules.so)

//____________________________________________________________________
int Fun4All_G4_sPHENIX_CentralMembrane_hp(
  const int nEvents = 1,
  const char *outputFile = "DST/dst_eval_centralmembrane-nominal-1ev_300.root",
  const char *central_membrane_output_file = "distortion_maps-new/CMDistortionCorrections-1ev_300.root"

//   const char *outputFile = "DST/dst_eval_centralmembrane_distorted-10ev-new.root",
//   const char *central_membrane_output_file = "distortion_maps-new/CMDistortionCorrections_distorted-10ev-new.root"

//   const char *outputFile = "DST/dst_eval_centralmembrane_distorted_scaled_x2-10ev-new.root",
//   const char *central_membrane_output_file = "distortion_maps-new/CMDistortionCorrections_distorted_scaled_x2-10ev-new.root"

//   const char *outputFile = "DST/dst_eval_centralmembrane_distorted_full-10ev-new.root",
//   const char *central_membrane_output_file = "distortion_maps-new/CMDistortionCorrections_distorted_full-10ev-new.root"
  )
{

  // options
  Enable::PIPE = true;
  Enable::BBC = true;
  Enable::MAGNET = true;
  Enable::PLUGDOOR = false;

  // enable all absorbers
  // this is equivalent to the old "absorberactive" flag
  Enable::ABSORBER = true;

  // central tracking
  Enable::MVTX = true;
  Enable::INTT = true;
  Enable::TPC = true;
  Enable::MICROMEGAS = true;
  Enable::BLACKHOLE = true;

  // TPC
  // space charge distortions
  G4TPC::ENABLE_STATIC_DISTORTIONS = false;
  G4TPC::static_distortion_filename = "distortion_maps/average_minus_static_distortion_converted.root";
  // G4TPC::static_distortion_filename = "distortion_maps/average_minus_static_distortion_converted_scaled_x2.root";
  // G4TPC::static_distortion_filename = "distortion_maps/static_only.distortion_map.hist.root";

  // space charge corrections
  G4TPC::ENABLE_CORRECTIONS = false;

  G4TPC::ENABLE_CENTRAL_MEMBRANE_HITS = true;
  G4TPC::CENTRAL_MEMBRANE_SAVEHISTOGRAMS = true;
  G4TPC::CENTRAL_MEMBRANE_ROOTOUTPUT_FILENAME = central_membrane_output_file;
  
  
  // for testing the momentum resolution, focus on having Micromegas in only one sector
  // G4MICROMEGAS::CONFIG = G4MICROMEGAS::CONFIG_Z_ONE_SECTOR;

  // tracking configuration
  G4TRACKING::use_full_truth_track_seeding = false;
  G4TRACKING::SC_CALIBMODE = false;

  // server
  auto se = Fun4AllServer::instance();
  se->Verbosity(1);

  // make sure to printout random seeds for reproducibility
  PHRandomSeed::Verbosity(1);

  // reco const
  auto rc = recoConsts::instance();
  // rc->set_IntFlag("RANDOMSEED",PHRandomSeed());
  rc->set_IntFlag("RANDOMSEED",1);

  // event counter
  se->registerSubsystem( new EventCounter_hp( "EventCounter_hp", 10 ) );

  // Geant4 initialization
  G4Init();
  G4Setup();

  // BBC
  BbcInit();
  Bbc_Reco();

  // cells
  Mvtx_Cells();
  Intt_Cells();
  TPC_Cells();
  Micromegas_Cells();

  // tracking
  TrackingInit();

  TPC_Clustering();

  // local evaluation
  if( true )
  {
    auto simEvaluator = new SimEvaluator_hp;
    simEvaluator->set_flags(
      SimEvaluator_hp::EvalEvent
//       |SimEvaluator_hp::EvalVertices
//       |SimEvaluator_hp::EvalParticles
      |SimEvaluator_hp::EvalHits
      );
    se->registerSubsystem(simEvaluator);
  }
  
  if( true )
  {
    auto micromegasEvaluator = new MicromegasEvaluator_hp;
    micromegasEvaluator->set_flags( MicromegasEvaluator_hp::EvalHits );
    se->registerSubsystem(micromegasEvaluator);
  }
  
  if( true )
  {
    auto trackingEvaluator = new TrackingEvaluator_hp;
    trackingEvaluator->set_flags(
      TrackingEvaluator_hp::EvalEvent
      |TrackingEvaluator_hp::EvalClusters
      |TrackingEvaluator_hp::EvalCMClusters
//       |TrackingEvaluator_hp::PrintClusters
      |TrackingEvaluator_hp::EvalTracks
      );
    se->registerSubsystem(trackingEvaluator);
  }

  // for single particle generators we just need something which drives
  // the event loop, the Dummy Input Mgr does just that
  auto in = new Fun4AllDummyInputManager("JADE");
  se->registerInputManager(in);

  // output manager
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
