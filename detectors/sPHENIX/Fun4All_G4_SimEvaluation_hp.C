#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllDstInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <phool/recoConsts.h>

// own modules
#include <g4eval/EventCounter_hp.h>
#include <g4eval/SimEvaluator_hp.h>

// needed to avoid warnings at readback
R__LOAD_LIBRARY(libg4bbc.so)
R__LOAD_LIBRARY(libg4tpc.so)
R__LOAD_LIBRARY(libg4intt.so)
R__LOAD_LIBRARY(libg4mvtx.so)

R__LOAD_LIBRARY(libg4eval.so)
R__LOAD_LIBRARY(libfun4all.so)

//________________________________________________________________________________________________
int Fun4All_G4_SimEvaluation_hp( 
    const int nEvents = 0,
    // const char* inputFile = "/sphenix/sim/sim01/sphnxpro/Micromegas/1/G4Hits_sHijing_0-12fm_00000_00100.root",
    const char* inputFile = "DST/CONDOR_Hijing_Micromegas_100kHz/G4Hits_merged/G4Hits_sHijing_0-12fm_merged_00000_00100.root",
    const char* outputFile = "DST/dst_eval.root"
)
{
  // server
  auto se = Fun4AllServer::instance();
  se->Verbosity(1);

  auto rc = recoConsts::instance();
  rc->set_IntFlag("RANDOMSEED", 1);

  // event counter
  se->registerSubsystem(new EventCounter_hp("EVENTCOUNTER_HP",1));

  // local evaluation
  auto simEvaluator = new SimEvaluator_hp;
  simEvaluator->set_flags(
    SimEvaluator_hp::EvalEvent|
    SimEvaluator_hp::EvalVertices|
    // SimEvaluator_hp::EvalHits|
    SimEvaluator_hp::EvalParticles );
  se->registerSubsystem(simEvaluator);

  // input manager
  auto in = new Fun4AllDstInputManager("DSTin");
  in->fileopen(inputFile);
  se->registerInputManager(in);

  // output manager
  auto out = new Fun4AllDstOutputManager("DSTOUT", outputFile);
  out->AddNode("SimEvaluator_hp::Container");
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
