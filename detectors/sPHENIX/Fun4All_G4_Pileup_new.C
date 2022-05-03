#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <fun4all/Fun4AllDstInputManager.h>
#include <g4main/Fun4AllDstPileupInputManager.h>
#include <g4main/PHG4VertexSelection.h>
#include <phool/PHRandomSeed.h>
#include <phool/recoConsts.h>

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libg4testbench.so)

//________________________________________________________________________________________________
int Fun4All_G4_Pileup_new(
const int nEvents = 0,
const char* inputFile = "DST/CONDOR_realistic_micromegas/G4Hits/G4Hits_realistic_micromegas_0.root",
const char* backgroundFile = "DST/CONDOR_realistic_micromegas/G4Hits/G4Hits_realistic_micromegas_1.root",
const char* outputFile = "DST/dst_g4hits_merged.root"
)
{

  // server
  auto se = Fun4AllServer::instance();
  se->Verbosity(1);

  auto rc = recoConsts::instance();
  rc->set_IntFlag("RANDOMSEED", PHRandomSeed());

  {
    // signal input manager
    auto in = new Fun4AllDstInputManager("DST_signal");
    in->registerSubsystem( new PHG4VertexSelection );

    // open file
    in->fileopen(inputFile);
    se->registerInputManager(in);
  }

  if( true )
  {
    // background input manager
    auto in = new Fun4AllDstPileupInputManager("DST_background");

    // open file
    in->fileopen(backgroundFile);
    se->registerInputManager(in);
  }

  // output manager
  /* all the nodes from DST and RUN are saved to the output */
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
