#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllDstInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <phool/recoConsts.h>

#include <trackbase/TrkrDefs.h>
#include <trackbase/TrkrCluster.h>
#include <trackbase/TrkrClusterContainer.h>
#include <trackbase/TrkrHit.h>

#include <trackbase_historic/SvtxTrack.h>
#include <trackbase_historic/SvtxTrackMap.h>

// #include <intt/InttHit.h>
// #include <mvtx/MvtxHit.h>
// #include <outertracker/OuterTrackerHit.h>

// custom evaluator
#include <g4eval/TrackingEvaluator_hp.h>

// needed to avoid warnings at readback
R__LOAD_LIBRARY(libintt.so)
R__LOAD_LIBRARY(libmvtx.so)
R__LOAD_LIBRARY(liboutertracker.so)

// need for home evaluator
R__LOAD_LIBRARY(libg4eval.so)

int Fun4All_G4_ReadDST_hp( const int nEvents = 0, const char* inputFile = "DST/dst_5k.root", const char *outputFile = "DST/dst_5k_eval.root" )
{

  // server
  auto se = Fun4AllServer::instance();
  se->Verbosity(0);
  auto rc = recoConsts::instance();

  // evaluation
  auto evaluator = new TrackingEvaluator_hp( "TRACKINGEVALUATOR_HP" );
  evaluator->Verbosity(0);
  se->registerSubsystem(evaluator);

  // input manager
  auto *in = new Fun4AllDstInputManager("DSTin");
  in->fileopen(inputFile);
  se->registerInputManager(in);

  {
    // output manager
    Fun4AllDstOutputManager *out = new Fun4AllDstOutputManager("DSTOUT", outputFile );
    out->AddNode("ClusterContainer");
    se->registerOutputManager(out);
  }

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
