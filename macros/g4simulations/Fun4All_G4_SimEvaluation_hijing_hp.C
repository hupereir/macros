#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllDstInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <phool/recoConsts.h>

#include <g4main/Fun4AllDstPileupInputManager.h>
#include <g4main/PHG4VertexSelection.h>

// own modules
#include <g4eval/EventCounter_hp.h>
#include <g4eval/MicromegasEvaluator_hp.h>
#include <g4eval/SimEvaluator_hp.h>
#include <g4eval/TrackingEvaluator_hp.h>

R__ADD_INCLUDE_PATH( $SPHENIX/src/macros/macros/g4simulations )
#include "G4Setup_sPHENIX.C"
#include "G4_Bbc.C"

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libg4testbench.so)

//________________________________________________________________________________________________
int Fun4All_G4_SimEvaluation_hijing_hp( const int nEvents = 0,
// const char* inputFile = "DST/dst_g4hits_merged.root",
// const char* outputFile = "DST/dst_simeval_merged.root"
const char* inputFile = "DST/CONDOR_Hijing_Micromegas/G4Hits_merged/G4Hits_sHijing_0-12fm_merged_00000_00100.root",
// const char* outputFile = "DST/CONDOR_Hijing_Micromegas/dst_simeval_merged/dst_simeval_sHijing_0-12fm_merged_00000_00100.root"
const char* outputFile = "DST/dst_simeval_merged.root"
)
{

  // server
  auto se = Fun4AllServer::instance();
  se->Verbosity(1);

  auto rc = recoConsts::instance();
  rc->set_IntFlag("RANDOMSEED", 1);

  // event counter
  se->registerSubsystem(new EventCounter_hp( "EventCounter_hp", 1 ));
  se->registerSubsystem(new SimEvaluator_hp);

  // input manager
  auto in = new Fun4AllDstInputManager("DSTin");  
  
  // open file
  in->fileopen(inputFile);
  se->registerInputManager(in);

  // output manager
  /* all the nodes from DST and RUN are saved to the output */
  Fun4AllDstOutputManager *out = new Fun4AllDstOutputManager("DSTOUT", outputFile);
  out->AddNode("SimEvaluator_hp::Container");
  se->registerOutputManager(out);

  // process events
  se->run(nEvents);

  // terminate
  se->End();
  std::cout << "All done" << std::endl;
  delete se;
  gSystem->Exit(0);
  return 0;
}

// This function is only used to test if we can load this as root6 macro
// without running into unresolved libraries and include files
void RunFFALoadTest() {}
