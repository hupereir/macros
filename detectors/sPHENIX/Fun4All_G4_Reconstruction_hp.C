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
  const int nEvents = 500,
  const int nSkipEvents = 0,
  const char* inputFile = "DST/CONDOR_realistic_micromegas/G4Hits/G4Hits_realistic_micromegas_0.root",
  const char* outputFile = "DST/dst_eval_acts_full_distorted-new.root",
  const char* spaceChargeMatricesFile = "DST/TpcSpaceChargeMatrices_acts_full_distorted.root",
  const char* residualsFile = "DST/TpcResiduals_acts_full_distorted.root",
  const char* qaOutputFile = "DST/qa.root"
 )
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
  G4TPC::static_distortion_filename = "distortion_maps/average_minus_static_distortion_converted.root";
  // G4TPC::static_distortion_filename = std::string(getenv("CALIBRATIONROOT")) + "/distortion_maps/static_only.distortion_map.hist.root";

  G4TPC::ENABLE_TIME_ORDERED_DISTORTIONS=false;
  // G4TPC::time_ordered_distortion_filename = "/sphenix/user/rcorliss/distortion_maps/2022.07/TimeOrderedDistortions_deltas.root";  
  // G4TPC::time_ordered_distortion_filename = "/sphenix/user/rcorliss/distortion_maps/2022.07/TimeOrderedDistortions.root";  
  // G4TPC::time_ordered_distortion_filename = std::string(getenv("CALIBRATIONROOT")) + "/distortion_maps/TimeOrderedDistortions.root";
  // G4TPC::time_ordered_distortion_filename = "distortion_maps-new/TimeOrderedDistortions_converted.root";

  // space charge corrections
  G4TPC::ENABLE_CORRECTIONS = false;
  // G4TPC::correction_filename = "distortion_maps-new/Distortions_full_realistic_micromegas_mm_acts_truth_notpc_nodistortion.root";
  // G4TPC::correction_filename = "distortion_maps-new/Distortions_full_realistic_micromegas_mm_acts_truth_notpc_distorted.root";
  // G4TPC::correction_filename = "distortion_maps-new/Distortions_full_realistic_micromegas_mm_genfit_truth_notpc_nodistortion.root";
  // G4TPC::correction_filename = "distortion_maps-new/Distortions_full_realistic_micromegas_mm_genfit_truth_notpc_distorted.root";
  // G4TPC::correction_filename = std::string(getenv("CALIBRATIONROOT")) + "/distortion_maps/static_only_inverted_10-new.root";
 
  // tracking configuration
  G4TRACKING::use_full_truth_track_seeding = true;
  G4TRACKING::use_genfit_track_fitter = true;
  
  // distortion reconstruction
  G4TRACKING::SC_CALIBMODE = true;
  G4TRACKING::SC_SAVEHISTOGRAMS = true;
  G4TRACKING::SC_USE_MICROMEGAS = true;
  G4TRACKING::SC_ROOTOUTPUT_FILENAME = spaceChargeMatricesFile;
  G4TRACKING::SC_HISTOGRAMOUTPUT_FILENAME = residualsFile;
  
  // server
  auto se = Fun4AllServer::instance();
  // se->Verbosity(2);
  
  // make sure to printout random seeds for reproducibility
  PHRandomSeed::Verbosity(1);

  // reco const
  auto rc = recoConsts::instance();
  rc->set_IntFlag("RANDOMSEED", (int) PHRandomSeed());
  // rc->set_IntFlag("RANDOMSEED",1);

  // event counter
  se->registerSubsystem( new EventCounter_hp( "EventCounter_hp", 10 ) );

  // hit generation and digitizer
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

  // clustering
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
    // local sim evaluation
    auto simEvaluator = new SimEvaluator_hp;
    simEvaluator->set_flags(
      SimEvaluator_hp::EvalEvent
      |SimEvaluator_hp::EvalVertices
      |SimEvaluator_hp::EvalHits
      |SimEvaluator_hp::EvalParticles
      );
    se->registerSubsystem(simEvaluator);
  }

  if( false )
  {
    // local Micromegas evaluation
    auto micromegasEvaluator = new MicromegasEvaluator_hp;
    micromegasEvaluator->set_flags( MicromegasEvaluator_hp::EvalG4Hits | MicromegasEvaluator_hp::EvalHits );
    se->registerSubsystem(micromegasEvaluator);
  }

  if( true )
  {
    // local tracking evaluation
    auto trackingEvaluator = new TrackingEvaluator_hp;
    trackingEvaluator->set_flags(
      TrackingEvaluator_hp::EvalEvent
      |TrackingEvaluator_hp::EvalClusters
      |TrackingEvaluator_hp::EvalTracks
      |TrackingEvaluator_hp::EvalTrackPairs
      );

    // special track map is used for space charge calibrations
    if( G4TRACKING::SC_CALIBMODE && !G4TRACKING::use_genfit_track_fitter)
    { trackingEvaluator->set_trackmapname( "SvtxSiliconMMTrackMap" ); }

    se->registerSubsystem(trackingEvaluator);
  }

  // QA
  Enable::QA = false;
  if( Enable::QA )
  {  
    Micromegas_QA();
  }

  // input manager
  auto in = new Fun4AllDstInputManager("DSTin");
  in->fileopen(inputFile);
  
  // in->fileopen(inputFile);
  se->registerInputManager(in);

  // output manager
  auto out = new Fun4AllDstOutputManager("DSTOUT", outputFile);
  out->AddNode("TrackingEvaluator_hp::Container");

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
