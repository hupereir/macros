#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllDstInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <phool/recoConsts.h>

// own modules
#include <g4eval_hp/EventCounter_hp.h>
#include <g4eval_hp/MicromegasEvaluator_hp.h>
#include <g4eval_hp/SimEvaluator_hp.h>

// local macros
#include "G4Setup_sPHENIX.C"
#include "G4_Global.C"
#include "Trkr_RecoInit.C"

R__LOAD_LIBRARY(libtpc.so)
R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libg4eval_hp.so)

//________________________________________________________________________________________________
int Fun4All_G4_SimEvaluation_hp(
    const int nEvents = 0,
    const int nSkipEvents = 0,
    const char* inputFile = "G4Hits_sHijing_0_488fm-0000000007-00000.root",
    const char* outputFile = "DST/dst_simeval_0_488fm-0000000007-00000.root"
)
{

  // print inputs
  std::cout << "Fun4All_G4_SimEvaluation_hp - nEvents: " << nEvents << std::endl;
  std::cout << "Fun4All_G4_SimEvaluation_hp - nSkipEvents: " << nSkipEvents << std::endl;
  std::cout << "Fun4All_G4_SimEvaluation_hp - inputFile: " << inputFile << std::endl;
  std::cout << "Fun4All_G4_SimEvaluation_hp - outputFile: " << outputFile << std::endl;

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

  // server
  auto se = Fun4AllServer::instance();
  se->Verbosity(1);

  // event counter
  se->registerSubsystem(new EventCounter_hp("EVENTCOUNTER_HP",1));

  // condition database
  Enable::CDB = true;
  auto rc = recoConsts::instance();
  // rc->set_IntFlag("RANDOMSEED", 1);
  rc->set_StringFlag("CDB_GLOBALTAG",CDB::global_tag);
  rc->set_uint64Flag("TIMESTAMP",CDB::timestamp);
  
//   G4Init();
//   G4Setup();

  TrackingInit();
  
//   // geometry initialization
//   ACTSGEOM::ActsGeomInit();

//   // need to run micromegas CELLS, in order to have a valid geometry
//   Micromegas_Cells();

  // local evaluation
  auto simEvaluator = new SimEvaluator_hp;
  simEvaluator->set_flags( SimEvaluator_hp::EvalEvent );
  se->registerSubsystem(simEvaluator);

  // Micromegas evaluation
  auto micromegasEvaluator = new MicromegasEvaluator_hp;
  micromegasEvaluator->set_flags( MicromegasEvaluator_hp::EvalG4Hits );
  se->registerSubsystem(micromegasEvaluator);

  // input manager
  auto in = new Fun4AllDstInputManager("DSTin");
  in->fileopen(inputFile);
  se->registerInputManager(in);

  // output manager
  auto out = new Fun4AllDstOutputManager("DSTOUT", outputFile);
  // out->AddNode("SimEvaluator_hp::Container");
  out->AddNode("MicromegasEvaluator_hp::Container");
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
