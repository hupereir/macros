#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllDummyInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <g4main/PHG4SimpleEventGenerator.h>
#include <phhepmc/Fun4AllHepMCInputManager.h>
#include <phool/recoConsts.h>

// own modules
#include <g4eval/EventCounter_hp.h>
#include <g4eval/MicromegasEvaluator_hp.h>
#include <g4eval/TrackingEvaluator_hp.h>
#include <g4eval/SimEvaluator_hp.h>


#include <array>

R__ADD_INCLUDE_PATH( /afs/rhic.bnl.gov/phenix/users/hpereira/sphenix/src/macros/macros/g4simulations )
#include "G4Setup_sPHENIX.C"
#include "G4_Bbc.C"

R__LOAD_LIBRARY(libfun4all.so)

//______________________________________________________________________________________
int Fun4All_G4_HepMC_hp( 
  const int nEvents = 100, 
  const char *inputFile = "phpythia8_200GeVMB_hepmc.dat.gz",
  const char* outputFile = "DST/dst_sim_hepmc.root" )
{

  // options
  const bool do_pipe = true;
  const bool do_pstof = false;
  const bool do_cemc = false;
  const bool do_hcalin = false;
  const bool do_magnet = false;
  const bool do_hcalout = false;
  const bool do_plugdoor = false;

  bool do_tracking = true;

  // customize TPC
  Tpc::misalign_tpc_clusters = false;

  // enable micromegas
  Micromegas::enable_micromegas = true;

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
  se->Verbosity(1);

  auto rc = recoConsts::instance();
  // rc->set_IntFlag("RANDOMSEED", 1);

  // event counter
  se->registerSubsystem( new EventCounter_hp( "EVENTCOUNTER_HP", 1 ) );

  // G4 setup
  G4Setup(
    absorberactive, magfield, EDecayType::kAll,
    do_tracking, do_pstof, do_cemc, do_hcalin, do_magnet, do_hcalout, do_pipe, do_plugdoor, false,
    magfield_rescale);

  // tracking
  if( true )
  {
    Tracking_Cells();
    Tracking_Clus();
  }
  
  // evaluators
  if( false )
  {
    se->registerSubsystem( new SimEvaluator_hp );
    auto trackingEvaluator = new TrackingEvaluator_hp;
    trackingEvaluator->set_flags(
      TrackingEvaluator_hp::EvalEvent|
      // TrackingEvaluator_hp::PrintClusters|
      TrackingEvaluator_hp::EvalClusters|
      TrackingEvaluator_hp::EvalTracks);
    se->registerSubsystem(trackingEvaluator);
  }
  
  // for single particle generators we just need something which drives
  // the event loop, the Dummy Input Mgr does just that
  auto in = new Fun4AllHepMCInputManager("HepMCInput_1");
  se->registerInputManager(in);
  in->fileopen(inputFile);

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
