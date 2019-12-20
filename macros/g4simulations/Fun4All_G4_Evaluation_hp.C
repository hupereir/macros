#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllDstInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <phool/recoConsts.h>
#include <trackreco/PHGenFitTrkFitter.h>

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
R__LOAD_LIBRARY(libtrack_reco.so)

// need for own evaluator
R__LOAD_LIBRARY(libg4eval.so)

//_________________________________________________________________________
int Fun4All_G4_Evaluation_hp( const int nEvents = 0, const char* inputFile = "DST/dst_eval_1k_truth_notpc_nominal.root", const char *outputFile = "DST/dst_eval_1k_truth_notpc_noouter.root" )
{

  // server
  auto se = Fun4AllServer::instance();
  se->Verbosity(0);

  auto rc = recoConsts::instance();
  rc->set_IntFlag("RANDOMSEED", 1);

  // event counter
  se->registerSubsystem(new EventCounter_hp());

  // refit tracks
  if( true )
  {
    auto kalman = new PHGenFitTrkFitter;

    // disable tpc
    for( int layer = 7; layer < 23; ++layer ) { kalman->disable_layer( layer ); }
    for( int layer = 23; layer < 39; ++layer ) { kalman->disable_layer( layer ); }
    for( int layer = 39; layer < 55; ++layer ) { kalman->disable_layer( layer ); }

    // disable outer layer
    for( int layer = 55; layer < 57; ++layer ) { kalman->disable_layer( layer ); }

    kalman->set_vertexing_method("avf-smoothing:1");
    kalman->set_use_truth_vertex(false);

    se->registerSubsystem(kalman);
  }

  // evaluation
  se->registerSubsystem(new TrackingEvaluator_hp( "TRACKINGEVALUATOR_HP" ));

  // input manager
  auto *in = new Fun4AllDstInputManager("DSTin");
  in->fileopen(inputFile);
  se->registerInputManager(in);

  // output manager
  Fun4AllDstOutputManager *out = new Fun4AllDstOutputManager("DSTOUT", outputFile );
  out->AddNode("Container");
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
