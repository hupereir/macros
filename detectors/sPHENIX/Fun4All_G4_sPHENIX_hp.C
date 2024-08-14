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
#include <g4eval_hp/TrackingEvaluator_hp.h>

// local macros
#include <G4Setup_sPHENIX.C>
#include <G4_Global.C>

#include <Trkr_RecoInit.C>
#include <Trkr_Clustering.C>

#define USE_TRUTH_TRACK_FINDING
#define USE_ACTS

#ifdef USE_TRUTH_TRACK_FINDING
#include <Trkr_TruthReco.C>
#else
#include <Trkr_Reco.C>
#endif

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libg4eval_hp.so)

//____________________________________________________________________
int Fun4All_G4_sPHENIX_hp(
  const int nEvents = 200,
  #ifdef USE_ACTS
  const char* outputFile = "DST/dst_eval_acts_truth_nodistortion.root",
  const char* trackingEvaluationFile = "DST/tracking_evaluation_acts_nodistortion.root",
  const char* spaceChargeMatricesFile = "DST/TpcSpaceChargeMatrices_acts_truth_nodistortion.root"
  #else
  const char* outputFile = "DST/dst_eval_genfit_truth_nodistortion.root",
  const char* trackingEvaluationFile = "DST/tracking_evaluation_genfit_nodistortion.root",
  const char* spaceChargeMatricesFile = "DST/TpcSpaceChargeMatrices_genfit_truth_nodistortion.root"
  #endif
  )
{

  // print inputs
  std::cout << "Fun4All_G4_sPHENIX_hp - nEvents: " << nEvents << std::endl;
  std::cout << "Fun4All_G4_sPHENIX_hp - outputFile: " << outputFile << std::endl;
  std::cout << "Fun4All_G4_sPHENIX_hp - spaceChargeMatricesFile: " << spaceChargeMatricesFile << std::endl;

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
  G4TPC::DISTORTIONS_USE_PHI_AS_RADIANS = false;
  G4TPC::ENABLE_REACHES_READOUT = false;
  G4TPC::ENABLE_STATIC_DISTORTIONS = false;
  G4TPC::static_distortion_filename = "/phenix/u/hpereira/sphenix/work/g4simulations/distortion_maps/average_minus_static_distortion_converted.root";
  //  G4TPC::static_distortion_filename = "/sphenix/user/rcorliss/distortion_maps/2023.02/Summary_hist_mdc2_UseFieldMaps_AA_event_0_bX180961051_0.distortion_map.hist.root";

  // space charge corrections
  G4TPC::ENABLE_STATIC_CORRECTIONS = false;
  G4TPC::static_correction_filename = "/phenix/u/hpereira/sphenix/work/g4simulations/distortion_maps/average_minus_static_distortion_converted.root";
  // G4TPC::static_correction_filename = "/phenix/u/hpereira/sphenix/work/g4simulations/distortion_maps/average_minus_static_distortion_inverted_10.root";
  // G4TPC::static_correction_filename = "/sphenix/user/rcorliss/distortion_maps/2023.02/Summary_hist_mdc2_UseFieldMaps_AA_smoothed_average.correction_map.hist.root";
  // G4TPC::static_correction_filename = "/phenix/u/hpereira/sphenix/work/g4simulations/distortion_maps/Summary_hist_mdc2_UseFieldMaps_AA_event_0_bX180961051_0.distortion_map.inverted_10.root";

  // tracking configuration
  #ifdef USE_TRUTH_TRACK_FINDING
  G4TRACKING::use_full_truth_track_seeding = true;
  #endif

  #ifndef USE_ACTS
  G4TRACKING::use_genfit_track_fitter = true;
  #endif

  // distortion reconstruction
  G4TRACKING::SC_CALIBMODE = true;
  G4TRACKING::SC_USE_MICROMEGAS = true;
  G4TRACKING::SC_ROOTOUTPUT_FILENAME = spaceChargeMatricesFile;

  // server
  auto se = Fun4AllServer::instance();
  // se->Verbosity(2);

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
  se->registerSubsystem( new EventCounter_hp( "EventCounter_hp", 10 ) );

  {
    // event generation
    auto gen = new PHG4SimpleEventGenerator;
//     gen->add_particles("pi+",10);
//     gen->add_particles("pi-",10);

    gen->add_particles("pi+",1);
    gen->add_particles("pi-",1);

    gen->set_eta_range(-1.0, 1.0);
    gen->set_phi_range(-M_PI, M_PI);

    if( false )
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
  // MbdInit();
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

  Tracking_Reco();
  Global_Reco();

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

  if( false )
  {
    // Micromegas evaluation
    auto micromegasEvaluator = new MicromegasEvaluator_hp;
    // micromegasEvaluator->set_flags( MicromegasEvaluator_hp::EvalG4Hits|MicromegasEvaluator_hp::EvalHits|MicromegasEvaluator_hp::PrintGeometry );
    micromegasEvaluator->set_flags( MicromegasEvaluator_hp::PrintGeometry );
    se->registerSubsystem(micromegasEvaluator);
  }

  if( true )
  {
    auto trackingEvaluator = new TrackingEvaluator_hp;
    trackingEvaluator->set_flags(
      TrackingEvaluator_hp::EvalEvent
      |TrackingEvaluator_hp::EvalClusters
      |TrackingEvaluator_hp::EvalTracks
      );

    // special track map is used for space charge calibrations
    if( G4TRACKING::SC_CALIBMODE && !G4TRACKING::use_genfit_track_fitter)
    { trackingEvaluator->set_trackmapname( "SvtxSiliconMMTrackMap" ); }

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
