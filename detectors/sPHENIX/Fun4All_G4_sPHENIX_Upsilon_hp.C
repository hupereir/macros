#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllDummyInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>

#include <g4main/PHG4ParticleGeneratorVectorMeson.h>
#include <phool/PHRandomSeed.h>
#include <phool/recoConsts.h>

// own modules
#include <g4eval_hp/EventCounter_hp.h>
#include <g4eval_hp/SimEvaluator_hp.h>
#include <g4eval_hp/MicromegasEvaluator_hp.h>
#include <g4eval_hp/TrackingEvaluator_hp.h>

// local macros
#include <G4Setup_sPHENIX.C>
#include <G4_Global.C>

#include <Trkr_RecoInit.C>
#include <Trkr_Clustering.C>
#include <Trkr_Reco.C>
// #include <Trkr_TruthReco.C>
#include <Trkr_Eval.C>

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libg4eval_hp.so)

#define USE_ACTS

//____________________________________________________________________
int Fun4All_G4_sPHENIX_Upsilon_hp(
  const int nEvents = 500,
  #ifdef USE_ACTS
  const char *outputFile = "DST/dst_eval_upsilon_acts_full_nodistortion.root"
  #else
  const char *outputFile = "DST/dst_eval_upsilon_genfit_full_nodistortion.root"
  #endif
  )
{

  // options
  Enable::PIPE = true;
  // Enable::MBD = false;
  Enable::MBDFAKE = true;
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
  G4TPC::DISTORTIONS_USE_PHI_AS_RADIANS = true;
  G4TPC::ENABLE_STATIC_DISTORTIONS = false;
  // G4TPC::static_distortion_filename = "/phenix/u/hpereira/sphenix/work/g4simulations/distortion_maps/average_minus_static_distortion_converted.root";
  G4TPC::static_distortion_filename = std::string("/sphenix/user/rcorliss/distortion_maps/2023.02/Summary_hist_mdc2_UseFieldMaps_AA_event_0_bX180961051_0.distortion_map.hist.root");

  // space charge corrections
  G4TPC::ENABLE_STATIC_CORRECTIONS = false;
  // G4TPC::correction_filename = "/phenix/u/hpereira/sphenix/work/g4simulations/distortion_maps/average_minus_static_distortion_converted.root";
  // G4TPC::correction_filename = "/phenix/u/hpereira/sphenix/work/g4simulations/distortion_maps/average_minus_static_distortion_inverted_10.root";
  G4TPC::static_correction_filename = std::string("/sphenix/user/rcorliss/distortion_maps/2023.02/Summary_hist_mdc2_UseFieldMaps_AA_smoothed_average.correction_map.hist.root");
  // G4TPC::correction_filename = "/phenix/u/hpereira/sphenix/work/g4simulations/distortion_maps/Summary_hist_mdc2_UseFieldMaps_AA_event_0_bX180961051_0.distortion_map.inverted_10.root";

  // tracking configuration
  G4TRACKING::use_full_truth_track_seeding = false;

  #ifndef USE_ACTS
  G4TRACKING::use_genfit_track_fitter = true;
  #endif

  // space charge calibration mode
  G4TRACKING::SC_CALIBMODE = false;

  // server
  auto se = Fun4AllServer::instance();
  // se->Verbosity(2);

  // make sure to printout random seeds for reproducibility
  PHRandomSeed::Verbosity(1);

  // reco const
  auto rc = recoConsts::instance();
  Enable::CDB = true;
  rc->set_StringFlag("CDB_GLOBALTAG",CDB::global_tag);
  rc->set_uint64Flag("TIMESTAMP",CDB::timestamp);

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
  Mbd_Reco();

  // cells
  Mvtx_Cells();
  Intt_Cells();
  TPC_Cells();
  Micromegas_Cells();

  // tracking initialization
  TrackingInit();

  // digitizer and clustering
  Mvtx_Clustering();
  Intt_Clustering();
  TPC_Clustering();
  Micromegas_Clustering();

  // tracking
  Tracking_Reco();
  Global_Reco();

  if( true )
  {
    auto trackingEvaluator = new TrackingEvaluator_hp;
    trackingEvaluator->set_flags(
      TrackingEvaluator_hp::EvalEvent|
      // TrackingEvaluator_hp::EvalClusters|
      TrackingEvaluator_hp::EvalTracks|
      TrackingEvaluator_hp::EvalTrackPairs);
    se->registerSubsystem(trackingEvaluator);
  }

  // tracking evaluation
  Tracking_Eval("DST/tracking_evaluation.root");

//   // QA
//   Enable::QA = true;
//   Enable::TRACKING_QA = true;
//   Tracking_QA();
//   se->registerSubsystem( new QAG4SimulationUpsilon );

  // for single particle generators we just need something which drives
  // the event loop, the Dummy Input Mgr does just that
  auto in = new Fun4AllDummyInputManager("JADE");
  se->registerInputManager(in);

  // output manager
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
