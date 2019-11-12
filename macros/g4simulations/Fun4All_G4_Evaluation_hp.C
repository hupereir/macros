#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllDstInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <phool/recoConsts.h>

// own modules
#include <g4eval/EventCounter_hp.h>
#include <g4eval/TrackingEvaluator_hp.h>

// needed to avoid warnings at readback
R__LOAD_LIBRARY(libg4tpc.so)
R__LOAD_LIBRARY(libg4intt.so)
R__LOAD_LIBRARY(libg4mvtx.so)
R__LOAD_LIBRARY(libg4outertracker.so)

R__LOAD_LIBRARY(libintt.so)
R__LOAD_LIBRARY(libmvtx.so)
R__LOAD_LIBRARY(liboutertracker.so)

// need for own evaluator
R__LOAD_LIBRARY(libg4eval.so)
R__LOAD_LIBRARY(libtrack_reco.so)

//_________________________________________________________________________
int Fun4All_G4_Evaluation_hp( const int nEvents = 0, const char* inputFile = "DST/dst_reco.root", const char *outputFile = "DST/dst_eval.root" )
{

  // server
  auto se = Fun4AllServer::instance();
  se->Verbosity(0);

  auto rc = recoConsts::instance();
  // rc->set_IntFlag("RANDOMSEED", 1);

  // event counter
  se->registerSubsystem( new EventCounter_hp() );

  // evaluation
  auto evaluator = new TrackingEvaluator_hp( "TRACKINGEVALUATOR_HP" );
  evaluator->Verbosity(0);
  se->registerSubsystem(evaluator);

  // input manager
  auto *in = new Fun4AllDstInputManager("DSTin");
  in->fileopen(inputFile);
  se->registerInputManager(in);

  // output manager
  Fun4AllDstOutputManager *out = new Fun4AllDstOutputManager("DSTOUT", outputFile );
  out->AddNode("ClusterContainer");
  se->registerOutputManager(out);

  //-----------------
  // Event processing
  //-----------------
  se->run(nEvents);

  //-----
  // Exit
  //-----
  se->End();
  std::cout << "All done" << std::endl;
  delete se;
  gSystem->Exit(0);
  return 0;
}

// This function is only used to test if we can load this as root6 macro
// without running into unresolved libraries and include files
void RunFFALoadTest() {}
