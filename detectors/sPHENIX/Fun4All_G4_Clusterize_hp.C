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
#include <g4eval_hp/EventCounter_hp.h>
#include <g4eval_hp/MicromegasEvaluator_hp.h>
#include <g4eval_hp/SimEvaluator_hp.h>
#include <g4eval_hp/TrackingEvaluator_hp.h>

// local macros
#include "G4Setup_sPHENIX.C"
#include "G4_Bbc.C"
#include "G4_Global.C"

#include "Trkr_RecoInit.C"
#include "Trkr_Clustering.C"

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libg4eval_hp.so)
R__LOAD_LIBRARY(libqa_modules.so)

//________________________________________________________________________________________________
int Fun4All_G4_Clusterize_hp(
  const int nEvents = 100,
  const int nSkipEvents = 0,
  const char* inputFile = "DST/CONDOR_realistic_micromegas/G4Hits/G4Hits_realistic_micromegas_0.root",
  const char* outputFile = "DST/dst_clusters.root"
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
  G4TPC::ENABLE_STATIC_DISTORTIONS = false;
  G4TPC::ENABLE_TIME_ORDERED_DISTORTIONS = false;
  G4TPC::ENABLE_CORRECTIONS = false;
  G4TPC::DO_HIT_ASSOCIATION = false;

  // tracking configuration
  G4TRACKING::use_full_truth_track_seeding = false;

  // do not initialize magnetic field in ACTS
  G4TRACKING::init_acts_magfield = false;
  
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

  // cells
  if( true )
  {
    Mvtx_Cells();
    Intt_Cells();
    TPC_Cells();
    Micromegas_Cells();
  }
  
  // digitizer and clustering
  Mvtx_Clustering();
  Intt_Clustering();
  TPC_Clustering();
  Micromegas_Clustering();

  // needed for makeActsGeometry
  TrackingInit();
  
  // input manager
  auto in = new Fun4AllDstInputManager("DSTin");
  in->fileopen(inputFile);
  se->registerInputManager(in);

  // output manager
  /* all the nodes from DST and RUN are saved to the output */
  auto out = new Fun4AllDstOutputManager("DSTOUT", outputFile);
  out->AddNode("TRKR_CLUSTER"); 
  out->AddNode("TRKR_CLUSTERCROSSINGASSOC");
  se->registerOutputManager(out);

  // skip events if any specified
  if( nSkipEvents > 0 )
  { se->skip( nSkipEvents ); }

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
