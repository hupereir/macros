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

// local macros
#include "G4Setup_sPHENIX.C"
#include "G4_Bbc.C"
#include "G4_Global.C"
#include "G4_Tracking.C"

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libqa_modules.so)

//________________________________________________________________________________________________
int Fun4All_G4_Reconstruction_hp(
  const int nEvents = 0,
  const int nSkipEvents = 0,
  const char* inputFile = "DSTNEW_TRKR_CLUSTER_sHijing_0_20fm_50kHz_bkg_0_20fm-0000000001-00000.root",
  const char* outputFile = "DST/dst_eval_mdc1.root",
  const char* residualsFile = "DST/TpcSpaceChargeMatrices.root",
  const char* qaOutputFile = "DST/qa.root"
 )
  // const char* inputFile = "/sphenix/user/pinkenbu/newout/DSTNEW_TRKR_CLUSTER_sHijing_0_20fm_50kHz_bkg_0_20fm-0000000001-00000.root",
  // const char* inputFile = "DST/CONDOR_realistic_micromegas/G4Hits/G4Hits_realistic_micromegas_0.root",
  // const char *outputFile = "DST/dst_eval_genfit_truth_realistic.root" )
  // const char *outputFile = "DST/dst_eval_genfit_truth_realistic-newgeom.root" )
  // const char *outputFile = "DST/dst_eval_genfit_truth_realistic_notpc.root" )
{

  // print inputs
  std::cout << "Fun4All_G4_Reconstruction_hp - nEvents: " << nEvents << std::endl;
  std::cout << "Fun4All_G4_Reconstruction_hp - nSkipEvents: " << nSkipEvents << std::endl;
  std::cout << "Fun4All_G4_Reconstruction_hp - inputFile: " << inputFile << std::endl;
  std::cout << "Fun4All_G4_Reconstruction_hp - outputFile: " << outputFile << std::endl;

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
  Enable::MVTX_SERVICE = true;
  Enable::INTT = true;
  Enable::TPC = true;
  Enable::MICROMEGAS = true;
  Enable::BLACKHOLE = true;

  // magnet
  G4MAGNET::magfield_rescale = -1.4 / 1.5;

  // TPC
  G4TPC::ENABLE_STATIC_DISTORTIONS = true;
  // G4TPC::static_distortion_filename = "distortion_maps/average-coarse.root";
  G4TPC::static_distortion_filename = "distortion_maps/fluct_average-coarse.root";

  // space charge corrections
  G4TPC::ENABLE_CORRECTIONS = false;
  // G4TPC::correction_filename = "distortion_maps_rec/Distortions_full_realistic_micromegas_mm-empty-new_extrapolated.root";
  // G4TPC::correction_filename = "distortion_maps_rec/Distortions_full_realistic_micromegas_mm-coarse-newgeom_extrapolated.root";
  
  // micromegas configuration
  G4MICROMEGAS::CONFIG = G4MICROMEGAS::CONFIG_BASELINE;

  // tracking configuration
  G4TRACKING::use_genfit = true;
  G4TRACKING::use_truth_init_vertexing = true;
  G4TRACKING::use_full_truth_track_seeding = true;
  G4TRACKING::SC_CALIBMODE = true;
  G4TRACKING::SC_ROOTOUTPUT_FILENAME = residualsFile;
  
  // server
  auto se = Fun4AllServer::instance();
  // se->Verbosity(1);

  // make sure to printout random seeds for reproducibility
  PHRandomSeed::Verbosity(1);

  // reco const
  auto rc = recoConsts::instance();
  rc->set_IntFlag("RANDOMSEED",PHRandomSeed());

  // event counter
  se->registerSubsystem( new EventCounter_hp( "EventCounter_hp", 10 ) );

  // cells
  if( true )
  {
    Mvtx_Cells();
    Intt_Cells();
    TPC_Cells();
    Micromegas_Cells();
  }

  // tracking init is needed for clustering
  MagnetFieldInit();
  TrackingInit();

  // digitizer and clustering
  if( true )
  {
    Mvtx_Clustering();
    Intt_Clustering();
    TPC_Clustering();
    Micromegas_Clustering();
  }
  
  // tracking
  if( true )
  {
    Tracking_Reco();
  }
  
  if( false )
  {
    // local evaluation
    auto simEvaluator = new SimEvaluator_hp;
    simEvaluator->set_flags(
      SimEvaluator_hp::EvalEvent
      // |SimEvaluator_hp::EvalVertices
      // |SimEvaluator_hp::EvalParticles
      );
    se->registerSubsystem(simEvaluator);
  }

  if( false )
  {
    // Micromegas evaluation
    auto micromegasEvaluator = new MicromegasEvaluator_hp;
    micromegasEvaluator->set_flags( MicromegasEvaluator_hp::EvalG4Hits | MicromegasEvaluator_hp::EvalHits );
    se->registerSubsystem(micromegasEvaluator);
  }

  if( true )
  {
    // tracking evaluation
    auto trackingEvaluator = new TrackingEvaluator_hp;
    trackingEvaluator->set_flags(
      TrackingEvaluator_hp::EvalEvent
      // |TrackingEvaluator_hp::EvalClusters
      // |TrackingEvaluator_hp::PrintClusters
      |TrackingEvaluator_hp::EvalTracks
      );
    se->registerSubsystem(trackingEvaluator);
  }

  // QA
  Enable::QA = false;
  if( Enable::QA )
  {
    Intt_QA();
    Mvtx_QA();
    TPC_QA();
    Micromegas_QA();
    Tracking_QA();
  }

  // input manager
  auto in = new Fun4AllDstInputManager("DSTin");
  in->fileopen(inputFile);
  se->registerInputManager(in);

  // output manager
  auto out = new Fun4AllDstOutputManager("DSTOUT", outputFile);
  if( true )
  {
    // add evaluation nodes
    // out->AddNode("MicromegasEvaluator_hp::Container");
    // out->AddNode("SimEvaluator_hp::Container");
    out->AddNode("TrackingEvaluator_hp::Container");
  }

  if( false )
  {
    // add cluster and tracks nodes
    out->AddNode( "TRKR_CLUSTER" );
    out->AddNode( "SvtxTrackMap" );
  }

  se->registerOutputManager(out);

  // skip events if any specified
  if( nSkipEvents > 0 )
  { se->skip( nSkipEvents ); }

  // process events
  se->run(nEvents);

  // QA output
  if (Enable::QA) QA_Output(qaOutputFile);

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
