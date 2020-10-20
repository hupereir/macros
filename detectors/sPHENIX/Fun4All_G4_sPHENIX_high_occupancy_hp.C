#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllDummyInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <g4main/PHG4SimpleEventGenerator.h>
#include <phool/PHRandomSeed.h>
#include <phool/recoConsts.h>

#include <qa_modules/QAG4SimulationUpsilon.h>
#include <qa_modules/QAG4SimulationTracking.h>
#include <qa_modules/QAG4SimulationVertex.h>
#include <qa_modules/QAHistManagerDef.h>
#include <qa_modules/QAG4SimulationIntt.h>
#include <qa_modules/QAG4SimulationMvtx.h>
#include <qa_modules/QAG4SimulationTpc.h>

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
int Fun4All_G4_sPHENIX_high_occupancy_hp(
  const int nEvents = 1,
  const char* outputFile = "DST/dst_eval.root",
  const char* qaOutputFile = "QA/qa_output.root"
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

  // tracking configuration
  G4TRACKING::use_Genfit = true;
  G4TRACKING::use_truth_track_seeding = false;
  G4TRACKING::disable_mvtx_layers = false;
  G4TRACKING::disable_tpc_layers = false;

  // magnet
  G4MAGNET::magfield_rescale = -1.4 / 1.5;

  // server
  auto se = Fun4AllServer::instance();
  se->Verbosity(1);

//   // reco const
//   auto rc = recoConsts::instance();
//   rc->set_IntFlag("RANDOMSEED", 1);

  // event counter
  se->registerSubsystem( new EventCounter_hp( "EventCounter_hp", 10 ) );

  {

    // 10.1103/PhysRevC.83.024913 : 0-10%AuAu 200 GeV dNch_deta = 609.
    static constexpr double target_dNch_deta = 609;
    
    // eta range
    static constexpr double deta_dphi = .5;
    static constexpr double eta_start = .2;
    
    //number particle  per 1/4 batch
    static constexpr int n_pion = int(target_dNch_deta * deta_dphi * deta_dphi / 4);
    
    // low momentum pions (background)
    auto gen = new PHG4SimpleEventGenerator();
    gen->add_particles("pi-",n_pion); 
    gen->add_particles("pi+",n_pion); 
 
    gen->set_vertex_distribution_function(
      PHG4SimpleEventGenerator::Uniform,
      PHG4SimpleEventGenerator::Uniform,
      PHG4SimpleEventGenerator::Uniform);
    gen->set_vertex_distribution_mean(0.0, 0.0, 0.0);
    gen->set_vertex_distribution_width(0.0, 0.0, 5.0);

    gen->set_vertex_size_function(PHG4SimpleEventGenerator::Uniform);
    gen->set_vertex_size_parameters(0.0, 0.0);
    
    // particle range
    gen->set_eta_range(eta_start, eta_start + deta_dphi);
    gen->set_phi_range(0, deta_dphi);
    gen->set_pt_range(0.1, 2);
    gen->Embed(2);
    gen->Verbosity(0);
    se->registerSubsystem(gen);

    // high momentum poions
    gen = new PHG4SimpleEventGenerator();
    gen->add_particles("pi-", n_pion);
    gen->add_particles("pi+", n_pion);
    
    {
      // reuse vertex of the last generator
      gen->set_reuse_existing_vertex(true);
      gen->set_existing_vertex_offset_vector(0.0, 0.0, 0.0);
    }
    
    gen->set_vertex_size_function(PHG4SimpleEventGenerator::Uniform);
    gen->set_vertex_size_parameters(0.0, 0.0);
    gen->set_eta_range(eta_start, eta_start + deta_dphi);
    gen->set_phi_range(0, deta_dphi);
    gen->set_pt_range(2, 50);
    gen->Embed(2);
    gen->Verbosity(0);

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

  {
    // tracking QA
    auto qa = new QAG4SimulationTracking();
    qa->addEmbeddingID(2);
    se->registerSubsystem(qa);
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

  // save QA histograms
  { QAHistManagerDef::saveQARootFile(qaOutputFile); }
  
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
