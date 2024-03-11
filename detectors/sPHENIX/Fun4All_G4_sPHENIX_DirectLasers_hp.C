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
// #include "G4_Tracking.C"

#include <G4_TrkrSimulation.C>
#include <Trkr_RecoInit.C>
#include <Trkr_Clustering.C>
#include <Trkr_Reco.C>

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libqa_modules.so)
R__LOAD_LIBRARY(libg4eval_hp.so)

//____________________________________________________________________
int Fun4All_G4_sPHENIX_DirectLasers_hp(
  const int nEvents = 100,
  const char* outputFile = "DST/dst_reco_all_directlasers.root",
  const char* spaceChargeMatricesFile = "DST/TpcSpaceChargeMatrices_all_directlasers.root",
  const char* evaluationFile = "DST/TpcDirectLaserReconstruction_all_directlasers.root"
)
{

  std::cout << "Fun4All_G4_sPHENIX_DirectLasers_hp - outputFile: " << outputFile << std::endl;
  std::cout << "Fun4All_G4_sPHENIX_DirectLasers_hp - evaluationFile: " << evaluationFile << std::endl;

  // options
  Enable::PIPE = true;
  Enable::MBD = true;
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

  // space charge corrections
  G4TPC::ENABLE_CORRECTIONS = false;

  G4TPC::ENABLE_CENTRAL_MEMBRANE_HITS = false;

  G4TPC::ENABLE_DIRECT_LASER_HITS = true;
  G4TPC::DIRECT_LASER_SAVEHISTOGRAMS = true;
  G4TPC::DIRECT_LASER_ROOTOUTPUT_FILENAME = spaceChargeMatricesFile;
  G4TPC::DIRECT_LASER_HISTOGRAMOUTPUT_FILENAME = evaluationFile;

  // for testing the momentum resolution, focus on having Micromegas in only one sector
  // G4MICROMEGAS::CONFIG = G4MICROMEGAS::CONFIG_Z_ONE_SECTOR;

  // tracking configuration
  G4TRACKING::use_full_truth_track_seeding = false;
  G4TRACKING::SC_CALIBMODE = false;

  // server
  auto se = Fun4AllServer::instance();
  se->Verbosity(2);

  // make sure to printout random seeds for reproducibility
  PHRandomSeed::Verbosity(1);

  // reco const
  auto rc = recoConsts::instance();
  // rc->set_IntFlag("RANDOMSEED",PHRandomSeed());
  // rc->set_IntFlag("RANDOMSEED",1);
  Enable::CDB = true;
  rc->set_StringFlag("CDB_GLOBALTAG",CDB::global_tag);
  rc->set_uint64Flag("TIMESTAMP",CDB::timestamp);

  // event counter
  se->registerSubsystem( new EventCounter_hp( "EventCounter_hp", 10 ) );

  // Geant4 initialization
  G4Init();
  G4Setup();

  // MBD
  // Mbd_Init();
  // Mbd_Reco();

  // cells
  Mvtx_Cells();
  Intt_Cells();
  TPC_Cells();
  Micromegas_Cells();

  // tracking
  MagnetFieldInit();
  TrackingInit();

  TPC_Clustering();

  // local evaluation
  if( true )
  {
    auto simEvaluator = new SimEvaluator_hp;
    simEvaluator->set_flags(
      SimEvaluator_hp::EvalEvent
//       |SimEvaluator_hp::EvalVertices
//       |SimEvaluator_hp::EvalParticles
      |SimEvaluator_hp::EvalHits
      );
    se->registerSubsystem(simEvaluator);
  }

  // se->registerSubsystem(new MicromegasEvaluator_hp);

  if( true )
  {
    auto trackingEvaluator = new TrackingEvaluator_hp;
    trackingEvaluator->set_flags(
      TrackingEvaluator_hp::EvalEvent
      |TrackingEvaluator_hp::EvalClusters
      |TrackingEvaluator_hp::EvalTracks
//       |TrackingEvaluator_hp::PrintTracks
      );
    se->registerSubsystem(trackingEvaluator);
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
