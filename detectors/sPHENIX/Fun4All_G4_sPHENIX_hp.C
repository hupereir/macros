#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllDummyInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <g4main/PHG4SimpleEventGenerator.h>
#include <phool/PHRandomSeed.h>
#include <phool/recoConsts.h>

// own modules
#include <g4eval_hp/EventCounter_hp.h>
#include <g4eval_hp/SimEvaluator_hp.h>
#include <g4eval_hp/MicromegasEvaluator_hp.h>
#include <g4eval_hp/MicromegasTrackEvaluator_hp.h>
#include <g4eval_hp/TrackingEvaluator_hp.h>

#include <trackingdiagnostics/TrackResiduals.h>

// local macros
#include <G4Setup_sPHENIX.C>
#include <G4_Global.C>

#include <Trkr_RecoInit.C>
#include <Trkr_Clustering.C>

#include <Trkr_Reco.C>

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libg4eval_hp.so)

#define USE_ACTS

//____________________________________________________________________
int Fun4All_G4_sPHENIX_hp(
  const int nEvents = 10,
#ifdef USE_ACTS
  const char* outputFile = "DST/CONDOR_sim_default/TRACKEVAL/TRACKEVAL_sim_default_test_0000.root",
  const char* residualsFile = "DST/CONDOR_sim_default/TpcResiduals/TpcResiduals_sim_default_test_0000.root"
#else
  const char* outputFile = "DST/CONDOR_sim_default/TRACKEVAL/TRACKEVAL_sim_default_genfit_test_0000.root",
  const char* residualsFile = "DST/CONDOR_sim_default/TpcResiduals/TpcResiduals_genfit_sim_default_test_0000.root"
#endif
  )
{

  // print inputs
  std::cout << "Fun4All_G4_sPHENIX_hp - nEvents: " << nEvents << std::endl;
  std::cout << "Fun4All_G4_sPHENIX_hp - outputFile: " << outputFile << std::endl;
  std::cout << "Fun4All_G4_sPHENIX_hp - residualsFile: " << residualsFile << std::endl;

  // options
  Enable::PIPE = true;
  // Enable::MBD = true;
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
  G4TPC::ENABLE_STATIC_DISTORTIONS = true;
  G4TPC::DISTORTIONS_USE_PHI_AS_RADIANS = false;
  G4TPC::static_distortion_filename = "/phenix/u/hpereira/sphenix/work/g4simulations/distortion_maps/average_minus_static_distortion_converted.root";
  // G4TPC::static_distortion_filename = "/cvmfs/sphenix.sdcc.bnl.gov/calibrations/sphnxpro/cdb/TPC_STATIC_CORRECTION_MODEL/ec/7b/ec7bd756f9fc7274af6b479ee39580e3_static_only_inverted_10-new.root";

  G4TPC::ENABLE_REACHES_READOUT = false;
  G4TPC::ENABLE_STATIC_CORRECTIONS = false;

  // distortion reconstruction
  G4TRACKING::SC_CALIBMODE = true;
  G4TRACKING::SC_USE_MICROMEGAS = true;

  std::cout<< "Fun4All_CombinedDataReconstruction - tpc_drift_velocity_sim: " << G4TPC::tpc_drift_velocity_sim << std::endl;
  std::cout<< "Fun4All_CombinedDataReconstruction - tpc_drift_velocity_reco: " << G4TPC::tpc_drift_velocity_reco << std::endl;

  // server
  auto se = Fun4AllServer::instance();

  // make sure to printout random seeds for reproducibility
  PHRandomSeed::Verbosity(1);

  // reco const
  auto rc = recoConsts::instance();
  // rc->set_IntFlag("RANDOMSEED",PHRandomSeed());
  // rc->set_IntFlag("RANDOMSEED",1);

  // condition database
  Enable::CDB = true;
  rc->set_StringFlag("CDB_GLOBALTAG",CDB::global_tag);
  rc->set_uint64Flag("TIMESTAMP",CDB::timestamp);

  // event counter
  se->registerSubsystem( new EventCounter_hp( "EventCounter_hp", 1 ) );

  {
    // event generation
    auto gen = new PHG4SimpleEventGenerator;
    gen->add_particles("pi+",10);
    gen->add_particles("pi-",10);
    gen->set_eta_range(-1.0, 1.0);
    gen->set_phi_range(-M_PI, M_PI);

    if( true )
    {

      // use specific distribution to generate pt
      // values from "http://arxiv.org/abs/nucl-ex/0308006"
      const std::vector<double> pt_bins = {0.5, 0.6, 0.7, 0.8, 0.9, 1, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 2, 2.2, 2.4, 2.6, 2.8, 3, 3.2, 3.5, 3.8, 4, 4.4, 4.8, 5.2, 5.6, 6, 6.5, 7, 8, 9, 10};
      const std::vector<double> yield_int = {2.23, 1.46, 0.976, 0.663, 0.457, 0.321, 0.229, 0.165, 0.119, 0.0866, 0.0628, 0.0458, 0.0337, 0.0248, 0.0183, 0.023, 0.0128, 0.00724, 0.00412, 0.00238, 0.00132, 0.00106, 0.000585, 0.00022, 0.000218, 9.64e-05, 4.48e-05, 2.43e-05, 1.22e-05, 7.9e-06, 4.43e-06, 4.05e-06, 1.45e-06, 9.38e-07};
      gen->set_pt_range(pt_bins,yield_int);

    } else if( false ) {

      // use power law
      gen->set_pt_range(0.5, 20.0);
      gen->set_power_law_n(-4);

    } else {

      // flat pt distribution
      // gen->set_pt_range(0.1, 10.0);
      gen->set_pt_range(0.1, 50.0);

    }

    // vertex
    gen->set_vertex_distribution_function(
      PHG4SimpleEventGenerator::Gaus,
      PHG4SimpleEventGenerator::Gaus,
      PHG4SimpleEventGenerator::Gaus);
    gen->set_vertex_distribution_mean(0.0, 0.0, 0.0);
    gen->set_vertex_distribution_width(0.01, 0.01, 5.0);

    gen->Embed(2);
    se->registerSubsystem(gen);
  }

  // Geant4 initialization
  G4Init();
  G4Setup();

  // BBC
  // Mbd_Reco();

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

  // silicon seeding
  auto silicon_Seeding = new PHActsSiliconSeeding;
  silicon_Seeding->Verbosity(0);
  silicon_Seeding->setStrobeRange(-5,5);
  silicon_Seeding->seedAnalysis(false);
  silicon_Seeding->setinttRPhiSearchWindow(0.2);
  silicon_Seeding->setinttZSearchWindow(1.0);
  se->registerSubsystem(silicon_Seeding);

  auto merger = new PHSiliconSeedMerger;
  merger->Verbosity(0);
  se->registerSubsystem(merger);

  // TPC seeding
  auto seeder = new PHCASeeding("PHCASeeding");
  seeder->Verbosity(0);
  seeder->SetLayerRange(7, 55);
  seeder->SetSearchWindow(2.,0.05); // z-width and phi-width, default in macro at 1.5 and 0.05
  seeder->SetClusAdd_delta_window(3.0,0.06); //  (0.5, 0.005) are default; sdzdr_cutoff, d2/dr2(phi)_cutoff
  seeder->SetMinHitsPerCluster(0);
  seeder->SetMinClustersPerTrack(3);
  seeder->useFixedClusterError(true);
  se->registerSubsystem(seeder);

  // expand stubs in the TPC using simple kalman filter
  auto cprop = new PHSimpleKFProp("PHSimpleKFProp");
  cprop->useFixedClusterError(true);
  cprop->set_max_window(5.);
  cprop->Verbosity(0);
  se->registerSubsystem(cprop);

  // matching to silicons
  auto silicon_match = new PHSiliconTpcTrackMatching;
  silicon_match->Verbosity(0);

  // narrow matching windows
  silicon_match->set_x_search_window(0.36);
  silicon_match->set_y_search_window(0.36);
  silicon_match->set_z_search_window(2.5);
  silicon_match->set_phi_search_window(0.014);
  silicon_match->set_eta_search_window(0.0091);
  silicon_match->set_test_windows_printout(false);
  se->registerSubsystem(silicon_match);

  // matching with micromegas
  auto mm_match = new PHMicromegasTpcTrackMatching;
  mm_match->Verbosity(0);
  mm_match->set_rphi_search_window_lyr1(3.0);
  mm_match->set_rphi_search_window_lyr2(15.0);

  mm_match->set_z_search_window_lyr1(30.0);
  mm_match->set_z_search_window_lyr2(3.0);

  mm_match->set_min_tpc_layer(38);             // layer in TPC to start projection fit
  mm_match->set_test_windows_printout(false);  // used for tuning search windows only
  se->registerSubsystem(mm_match);

  // track fit
  se->registerSubsystem(new PHTpcDeltaZCorrection);

  #ifdef USE_ACTS
  // perform final track fit with ACTS
  auto actsFit = new PHActsTrkFitter;
  actsFit->Verbosity(0);
  actsFit->commissioning(G4TRACKING::use_alignment);

  // fit with Micromegas and Silicon ONLY
  actsFit->fitSiliconMMs(G4TRACKING::SC_CALIBMODE);
  actsFit->setUseMicromegas(G4TRACKING::SC_USE_MICROMEGAS);

  actsFit->set_use_clustermover(true);
  actsFit->useActsEvaluator(false);
  actsFit->useOutlierFinder(false);
  actsFit->setFieldMap(G4MAGNET::magfield_tracking);

  actsFit->setExtrapolationMode(PHActsTrkFitter::ExtrapolationMode::Bidirectional);

  se->registerSubsystem(actsFit);

  auto cleaner = new PHTrackCleaner();
  cleaner->Verbosity(0);
  se->registerSubsystem(cleaner);

  if (G4TRACKING::SC_CALIBMODE)
  {
    /*
    * in calibration mode, calculate residuals between TPC and fitted tracks,
    * store in dedicated structure for distortion correction
    */
    auto residuals = new PHTpcResiduals;
    residuals->setOutputfile(residualsFile);
    residuals->setUseMicromegas(G4TRACKING::SC_USE_MICROMEGAS);

    // matches Tony's analysis
    residuals->setMinPt( 0.2 );

    // reconstructed distortion grid size (phi, r, z)
    // residuals->setGridDimensions(36, 48, 80);
    residuals->setGridDimensions(36, 16, 80);
    se->registerSubsystem(residuals);
  }

  #else

  // perform final track fit with GENFIT
  auto genfitFit = new PHGenFitTrkFitter;
  genfitFit->set_fit_silicon_mms(G4TRACKING::SC_CALIBMODE);
  genfitFit->set_use_micromegas(G4TRACKING::SC_USE_MICROMEGAS);

  genfitFit->setExtrapolationMode(PHGenFitTrkFitter::ExtrapolationMode::Bidirectional);

  se->registerSubsystem(genfitFit);

  if (G4TRACKING::SC_CALIBMODE)
  {
    /*
    * in calibration mode, calculate residuals between TPC and fitted tracks,
    * store in dedicated structure for distortion correction
    */
    auto residuals = new PHTpcResiduals;
    residuals->setOutputfile(residualsFile);
    residuals->setUseMicromegas(G4TRACKING::SC_USE_MICROMEGAS);
    residuals->setTrackMapName("SvtxTrackMap");

    // matches Tony's analysis
    residuals->setMinPt( 0.2 );

    // reconstructed distortion grid size (phi, r, z)
    // residuals->setGridDimensions(36, 48, 80);
    residuals->setGridDimensions(36, 16, 80);

    se->registerSubsystem(residuals);
  }

  #endif

  // local evaluation
  if( false )
  {
    auto simEvaluator = new SimEvaluator_hp;
    simEvaluator->set_flags(
      SimEvaluator_hp::EvalEvent
      |SimEvaluator_hp::EvalVertices
      |SimEvaluator_hp::EvalHits
      |SimEvaluator_hp::EvalParticles);
    se->registerSubsystem(simEvaluator);
  }

  if( true )
  {
    auto trackingEvaluator = new TrackingEvaluator_hp;
    trackingEvaluator->set_flags(
      TrackingEvaluator_hp::EvalTracks|
      TrackingEvaluator_hp::EvalTrackClusters|
      TrackingEvaluator_hp::MicromegasOnly
      );

    #ifdef USE_ACTS
    if( G4TRACKING::SC_CALIBMODE )
    { trackingEvaluator->set_trackmapname( "SvtxSiliconMMTrackMap" ); }
    #endif

    se->registerSubsystem(trackingEvaluator);
  }

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
