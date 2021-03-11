#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllDummyInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <g4main/PHG4SimpleEventGenerator.h>
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
int Fun4All_G4_sPHENIX_acts_hp(
  const int nEvents = 1000,
  const char *outputFile = "DST/dst_eval_acts_flat.root"
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
  G4TPC::ENABLE_STATIC_DISTORTIONS = false;
  // G4TPC::static_distortion_filename = "distortion_maps/fluct_average.rev3.1side.3d.file0.h_negz.real_B1.4_E-400.0.ross_phi1_sphenix_phislice_lookup_r26xp40xz40.distortion_map.hist.root";
  // G4TPC::static_distortion_filename = "distortion_maps/fluct_average-coarse.root";

  // space charge corrections
  G4TPC::ENABLE_CORRECTIONS = false;
  // G4TPC::correction_filename = "distortion_maps_rec/Distortions_full_iterate_realistic_micromegas_mm-new_extrapolated.root";
  // G4TPC::correction_filename = "distortion_maps_rec/Distortions_full_realistic_micromegas_mm-coarse_extrapolated.root";
  // G4TPC::correction_filename = "distortion_maps_rec/Distortions_full_realistic_micromegas_mm-coarse-new_extrapolated.root";

  // micromegas configuration
  G4MICROMEGAS::CONFIG = G4MICROMEGAS::CONFIG_BASELINE;

  // for testing the momentum resolution, focus on having Micromegas in only one sector
  // G4MICROMEGAS::CONFIG = G4MICROMEGAS::CONFIG_Z_ONE_SECTOR;

  // tracking configuration
  G4TRACKING::use_Genfit = false;
  G4TRACKING::seeding_type = G4TRACKING::PHTPCTRACKER_SEEDING;
  G4TRACKING::use_truth_track_seeding = false;
  G4TRACKING::disable_mvtx_layers = false;
  G4TRACKING::disable_tpc_layers = false;
  G4TRACKING::disable_micromegas_layers = false;

  // magnet
  G4MAGNET::magfield_rescale = -1.4 / 1.5;

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

  {
    // event generation
    auto gen = new PHG4SimpleEventGenerator;
    gen->add_particles("pi+",1);
    gen->add_particles("pi-",1);

    gen->set_eta_range(-1.0, 1.0);
    gen->set_phi_range(-1.0 * TMath::Pi(), 1.0 * TMath::Pi());

    if( false )
    {
      // use specific distribution to generate pt
      // values from "http://arxiv.org/abs/nucl-ex/0308006"
      const std::vector<double> pt_bins = {0.5, 0.6, 0.7, 0.8, 0.9, 1, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 2, 2.2, 2.4, 2.6, 2.8, 3, 3.2, 3.5, 3.8, 4, 4.4, 4.8, 5.2, 5.6, 6, 6.5, 7, 8, 9, 10};
      const std::vector<double> yield_int = {2.23, 1.46, 0.976, 0.663, 0.457, 0.321, 0.229, 0.165, 0.119, 0.0866, 0.0628, 0.0458, 0.0337, 0.0248, 0.0183, 0.023, 0.0128, 0.00724, 0.00412, 0.00238, 0.00132, 0.00106, 0.000585, 0.00022, 0.000218, 9.64e-05, 4.48e-05, 2.43e-05, 1.22e-05, 7.9e-06, 4.43e-06, 4.05e-06, 1.45e-06, 9.38e-07};
      gen->set_pt_range(pt_bins,yield_int);
    } else {
      // flat pt distribution
      gen->set_pt_range(0.5, 20.0);
    }

    // vertex
    gen->set_vertex_distribution_function(
      PHG4SimpleEventGenerator::Uniform,
      PHG4SimpleEventGenerator::Uniform,
      PHG4SimpleEventGenerator::Uniform);
    gen->set_vertex_distribution_mean(0.0, 0.0, 0.0);
    gen->set_vertex_distribution_width(0.0, 0.0, 5.0);
    gen->set_vertex_size_function(PHG4SimpleEventGenerator::Uniform);
    gen->set_vertex_size_parameters(0.0, 0.0);

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
    TrackingEvaluator_hp::EvalTracks);
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
