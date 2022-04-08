#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllDummyInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <g4main/PHG4ParticleGeneratorVectorMeson.h>
#include <phool/PHRandomSeed.h>
#include <phool/recoConsts.h>
#include <qa_modules/QAG4SimulationIntt.h>
#include <qa_modules/QAG4SimulationMvtx.h>
#include <qa_modules/QAHistManagerDef.h>

// own modules
#include <g4eval/EventCounter_hp.h>
#include <g4eval/SimEvaluator_hp.h>
#include <g4eval/MicromegasEvaluator_hp.h>
#include <g4eval/TrackingEvaluator_hp.h>
#include <tpccalib/TpcSpaceChargeReconstruction.h>

// local macros
#include "G4Setup_sPHENIX.C"
#include "G4_Bbc.C"
#include "G4_Global.C"
#include "G4_Tracking.C"

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libqa_modules.so)

// #define USE_ACTS

//____________________________________________________________________
int Fun4All_G4_sPHENIX_Upsilon_hp(
  const int nEvents = 5000,

  #ifdef USE_ACTS
  const char *outputFile = "DST/dst_eval_upsilon_acts_full_no_distortion-test.root",
  const char* qaOutputFile = "DST/qa_upsilon_acts_full_no_distortion-test.root"
  #else
  const char* outputFile = "DST/dst_eval_upsilon_genfit_full_no_distortion-test.root",
  const char* qaOutputFile = "DST/qa_upsilon_acts_genfit_no_distortion-test.root"
  #endif
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
  // G4TPC::static_distortion_filename = "/phenix/u/hpereira/sphenix/work/g4simulations/distortion_maps-new/average_minus_static_distortion_converted.root";
  // G4TPC::static_distortion_filename = "/star/u/rcorliss/sphenix/trackingStudySampleNov2021/static_only.distortion_map.hist.root";
    
  // space charge corrections
  G4TPC::ENABLE_CORRECTIONS = false;
  // G4TPC::correction_filename = "distortion_maps-new/average_minus_static_distortion_inverted_10-new.root";
  // G4TPC::correction_filename = "distortion_maps-new/static_only_inverted_10-new.root";
    
  // tracking configuration
  G4TRACKING::use_full_truth_track_seeding = false;
  G4TRACKING::use_truth_tpc_seeding = false;
  
  // genfit track fitter
  #ifdef USE_ACTS
  G4TRACKING::use_genfit_track_fitter = false;
  #else
  G4TRACKING::use_genfit_track_fitter = true;
  #endif
  
  // space charge calibration mode
  G4TRACKING::SC_CALIBMODE = false;
    
//   // use 2D magnetic field
//   G4MAGNET::magfield = string(getenv("CALIBRATIONROOT")) + string("/Field/Map/sPHENIX.2d.root");
//   G4MAGNET::magfield_rescale = -1.4 / 1.5;  // make consistent with expected Babar field strength of 1.4T
  
  // server
  auto se = Fun4AllServer::instance();
  se->Verbosity(1);

  // make sure to printout random seeds for reproducibility
  PHRandomSeed::Verbosity(1);

  // reco const
  auto rc = recoConsts::instance();
//   rc->set_IntFlag("RANDOMSEED", PHRandomSeed());
//   rc->set_IntFlag("RANDOMSEED", 5268597 );

  // event counter
  se->registerSubsystem( new EventCounter_hp( "EventCounter_hp", 10 ) );

  {
    // event generation
    auto gen = new PHG4ParticleGeneratorVectorMeson();
    gen->set_mass( 9.4603 );
    gen->set_width( 54.02e-6 );
    gen->add_decay_particles( "e+", "e-", 0 );

    gen->set_vertex_distribution_function(
      PHG4ParticleGeneratorVectorMeson::Uniform,
      PHG4ParticleGeneratorVectorMeson::Uniform,
      PHG4ParticleGeneratorVectorMeson::Uniform);

    gen->set_vertex_distribution_mean(0.0, 0.0, 0.0);
    gen->set_vertex_distribution_width(0.0, 0.0, 5.0);

    // TODO: what are vertex_size
    gen->set_vertex_size_function(PHG4ParticleGeneratorVectorMeson::Uniform);
    gen->set_vertex_size_parameters(0.0, 0.0);
    gen->set_rapidity_range(-1.0, 1.0);
    gen->set_pt_range(0.1, 20.0);

    gen->Embed(2);
    se->registerSubsystem(gen);
  }

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

  if( Enable::MICROMEGAS )
  { Micromegas_Cells(); }

  // digitizer and clustering
  Mvtx_Clustering();
  Intt_Clustering();
  TPC_Clustering();

  if( Enable::MICROMEGAS )
  { Micromegas_Clustering(); }

  // tracking
  TrackingInit();
  Tracking_Reco();

  // local evaluation
  if( false )
  {
    auto simEvaluator = new SimEvaluator_hp;
    simEvaluator->set_flags(
      SimEvaluator_hp::EvalEvent|
      SimEvaluator_hp::EvalVertices|
      SimEvaluator_hp::EvalParticles );
    se->registerSubsystem(simEvaluator);
  }
  
  auto trackingEvaluator = new TrackingEvaluator_hp;
  trackingEvaluator->set_flags(
    TrackingEvaluator_hp::EvalEvent|
    TrackingEvaluator_hp::EvalClusters|
    TrackingEvaluator_hp::EvalTracks|
    TrackingEvaluator_hp::EvalTrackPairs);
  se->registerSubsystem(trackingEvaluator);

  // QA
  Enable::QA = false;
  Enable::TRACKING_QA = Enable::QA && true;
  if( Enable::TRACKING_QA ) 
  {  
    Tracking_QA();
    se->registerSubsystem(new QAG4SimulationUpsilon);
  }
  
  // for single particle generators we just need something which drives
  // the event loop, the Dummy Input Mgr does just that
  auto in = new Fun4AllDummyInputManager("JADE");
  se->registerInputManager(in);

  // output manager
  auto out = new Fun4AllDstOutputManager("DSTOUT", outputFile);
  out->AddNode("SimEvaluator_hp::Container");
  out->AddNode("TrackingEvaluator_hp::Container");
  se->registerOutputManager(out);

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
