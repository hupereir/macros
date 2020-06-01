#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllDummyInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <g4main/PHG4SimpleEventGenerator.h>
#include <phool/recoConsts.h>
#include <qa_modules/QAG4SimulationIntt.h>
#include <qa_modules/QAG4SimulationMvtx.h>
#include <qa_modules/QAHistManagerDef.h>

// own modules
#include <g4eval/EventCounter_hp.h>
#include <g4eval/SimEvaluator_hp.h>
#include <g4eval/TrackingEvaluator_hp.h>
#include <tpccalib/TpcSpaceChargeReconstruction.h>
#include <trackreco/PHTruthClustering_hp.h>

R__ADD_INCLUDE_PATH( /phenix/u/hpereira/sphenix/src/macros/macros/g4simulations )
#include "G4Setup_sPHENIX.C"
#include "G4_Bbc.C"

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libqa_modules.so)

//____________________________________________________________________
int Fun4All_G4_sPHENIX_hp(
  const int nEvents = 2000,
  const char *outputFile = "DST/dst_eval_2k_realistic_full_zhengyun_fix3.root",
//   const int nEvents = 10,
//   const char *outputFile = "DST/dst_eval.root",
  const int nSeg_phi = 10000,
  const int nSeg_z = 5400
  )
{

  // options
  const bool do_pipe = true;
  const bool do_pstof = false;
  const bool do_cemc = false;
  const bool do_hcalin = false;
  const bool do_magnet = false;
  const bool do_hcalout = false;
  const bool do_plugdoor = false;

  const bool do_tracking = true;
  const bool do_QA = false;

  // customize tpc
  Tpc::enable_tpc_distortions = false;
  Tpc::misalign_tpc_clusters = false;

  // customize outer tracker segmentation
  OuterTracker::n_outertrack_layers = 2;
  OuterTracker::NSeg_Phi = nSeg_phi;
  OuterTracker::NSeg_Z = nSeg_z;

  // customize track finding
  TrackingParameters::use_track_prop = true;
  TrackingParameters::disable_mvtx_layers = false;
  TrackingParameters::disable_tpc_layers = false;
  TrackingParameters::disable_outertracker_layers = false;
  TrackingParameters::use_single_outertracker_layer = false;

  // establish the geometry and reconstruction setup
  G4Init(do_tracking, do_pstof, do_cemc, do_hcalin, do_magnet, do_hcalout, do_pipe, do_plugdoor);

  // set to 1 to make all absorbers active volumes
  int absorberactive = 1;

  // default map from the calibration database
  // scale the map to a 1.4 T field
  const auto magfield = std::string(getenv("CALIBRATIONROOT")) + std::string("/Field/Map/sPHENIX.2d.root");
  const float magfield_rescale = -1.4 / 1.5;

  // server
  auto se = Fun4AllServer::instance();

  auto rc = recoConsts::instance();
  rc->set_IntFlag("RANDOMSEED", 1);

  // event counter
  se->registerSubsystem( new EventCounter_hp( "EventCounter_hp", 10 ) );

  {
    // event generation
    // toss low multiplicity dummy events
    auto gen = new PHG4SimpleEventGenerator;
    gen->add_particles("pi+",1);
    gen->add_particles("pi-",1);

    gen->set_eta_range(-1.0, 1.0);
    gen->set_phi_range(-1.0 * TMath::Pi(), 1.0 * TMath::Pi());

    // use specific distribution to generate pt
    // values from "http://arxiv.org/abs/nucl-ex/0308006"
    const std::vector<double> pt_bins = {0.5, 0.6, 0.7, 0.8, 0.9, 1, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 2, 2.2, 2.4, 2.6, 2.8, 3, 3.2, 3.5, 3.8, 4, 4.4, 4.8, 5.2, 5.6, 6, 6.5, 7, 8, 9, 10};
    const std::vector<double> yield_int = {2.23, 1.46, 0.976, 0.663, 0.457, 0.321, 0.229, 0.165, 0.119, 0.0866, 0.0628, 0.0458, 0.0337, 0.0248, 0.0183, 0.023, 0.0128, 0.00724, 0.00412, 0.00238, 0.00132, 0.00106, 0.000585, 0.00022, 0.000218, 9.64e-05, 4.48e-05, 2.43e-05, 1.22e-05, 7.9e-06, 4.43e-06, 4.05e-06, 1.45e-06, 9.38e-07};
    gen->set_pt_range(pt_bins,yield_int);

    // // flat pt distribution
    // gen->set_pt_range(0.5, 20.0);

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

  // G4 setup
  G4Setup(
    absorberactive, magfield, EDecayType::kAll,
    do_tracking, do_pstof, do_cemc, do_hcalin, do_magnet, do_hcalout, do_pipe, do_plugdoor,
    magfield_rescale);

  // bbc reconstruction
  BbcInit();
  Bbc_Reco();

  // tracking
  Tracking_Cells();
  Tracking_Clus();

  // replace clusters by truth information
  // se->registerSubsystem( new PHTruthClustering_hp );

  Tracking_Reco();

  // local evaluation
  se->registerSubsystem(new SimEvaluator_hp);

  auto trackingEvaluator = new TrackingEvaluator_hp;
  trackingEvaluator->set_flags(
    TrackingEvaluator_hp::EvalEvent|
    TrackingEvaluator_hp::EvalClusters|
    TrackingEvaluator_hp::EvalTracks);
  se->registerSubsystem(trackingEvaluator);

  // QA modules
  if( do_QA )
  {
    se->registerSubsystem( new QAG4SimulationMvtx );
    se->registerSubsystem( new QAG4SimulationIntt );
  }

//   // space charge reconstruction
//   auto spaceChargeReconstruction = new TpcSpaceChargeReconstruction();
//   spaceChargeReconstruction->set_tpc_layers( n_maps_layer + n_intt_layer, n_gas_layer );
//   spaceChargeReconstruction->set_grid_dimensions(1, 12, 1);
//   se->registerSubsystem( spaceChargeReconstruction );

  // for single particle generators we just need something which drives
  // the event loop, the Dummy Input Mgr does just that
  auto in = new Fun4AllDummyInputManager("JADE");
  se->registerInputManager(in);

  // output manager
  /* save all nodes */
  auto out = new Fun4AllDstOutputManager("DSTOUT", outputFile);
  // out->AddNode("SimEvaluator_hp::Container");
  // out->AddNode("TrackingEvaluator_hp::Container");
  se->registerOutputManager(out);

  // process events
  se->run(nEvents);

  // QA
  if( do_QA )
  {
    const std::string qaFile= "data/G4sPHENIX.root_qa.root";
    QAHistManagerDef::saveQARootFile(qaFile.c_str());
  }

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
