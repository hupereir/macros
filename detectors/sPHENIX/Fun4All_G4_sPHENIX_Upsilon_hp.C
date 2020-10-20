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

//____________________________________________________________________
int Fun4All_G4_sPHENIX_Upsilon_hp(
  const int nEvents = 10,
  const char *outputFile = "DST/dst_eval_upsilon_corrected.root"
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
  Enable::MVTX_SERVICE = true;
  Enable::INTT = true;
  Enable::TPC = true;
  Enable::MICROMEGAS = true;
  Enable::BLACKHOLE = true;

  // TPC
  // space charge distortions
  G4TPC::enable_distortions = true;
  G4TPC::distortion_filename = "distortion_maps/fluct_average.rev3.1side.3d.file0.h_negz.real_B1.4_E-400.0.ross_phi1_sphenix_phislice_lookup_r26xp40xz40.distortion_map.hist.root";
  G4TPC::distortion_coordinates =
    PHG4TpcElectronDrift::COORD_PHI|
    PHG4TpcElectronDrift::COORD_R|
    PHG4TpcElectronDrift::COORD_Z;

//   // space charge corrections
//   G4TPC::enable_corrections = false;
//   G4TPC::correction_filename = "distortion_maps_rec/Distortions_drphi_full_Hijing_Micromegas_50kHz_truth_notpc.root";
//   // G4TPC::correction_filename = "distortion_maps_rec/Distortions_drphi_full_Hijing_Micromegas_50kHz_truth_notpc_truth.root";
//   G4TPC::correction_coordinates = TpcSpaceChargeCorrection_hp::COORD_PHI;

  // tracking configuration
  G4TRACKING::use_track_prop = false;
  G4TRACKING::disable_mvtx_layers = false;
  G4TRACKING::disable_tpc_layers = false;

  // magnet
  G4MAGNET::magfield_rescale = -1.4 / 1.5;

  // server
  auto se = Fun4AllServer::instance();
  se->Verbosity(1);

  // reco const
  auto rc = recoConsts::instance();
  rc->set_IntFlag("RANDOMSEED", PHRandomSeed());

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
  Micromegas_Cells();

  // digitizer and clustering
  Mvtx_Clustering();
  Intt_Clustering();
  TPC_Clustering();
  Micromegas_Clustering();

  // tracking
  TrackingInit();
  Tracking_Reco();

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
    TrackingEvaluator_hp::EvalTracks|
    TrackingEvaluator_hp::EvalTrackPairs);
  se->registerSubsystem(trackingEvaluator);

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
