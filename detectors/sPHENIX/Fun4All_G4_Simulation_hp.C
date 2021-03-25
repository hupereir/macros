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
int Fun4All_G4_Simulation_hp(
  const int nEvents = 1000,
  const char *outputFile = "DST/G4Hits_realistic_micromegas.root"
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

    if( true )
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
