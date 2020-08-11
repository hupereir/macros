bg
#include <phool/PHRandomSeed.h>
#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllInputManager.h>
#include <fun4all/Fun4AllDummyInputManager.h>
#include <fun4all/Fun4AllOutputManager.h>
#include <fun4all/Fun4AllDstInputManager.h>
#include <phool/recoConsts.h>
// #include <phpythia6/PHPythia6.h>
#include <phpythia8/PHPythia8.h>
#include <phhepmc/Fun4AllHepMCOutputManager.h>

R__ADD_INCLUDE_PATH( $SPHENIX/src/macros/macros/g4simulations )

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libg4testbench.so)
R__LOAD_LIBRARY(libphhepmc.so)
R__LOAD_LIBRARY(libPHPythia8.so)

int Fun4All_PythiaHepMC(
    const int nEvents = 100,
    const char *inputFile = "phpythia8.cfg",
    const char *outputFile = "phpythia8_200GeVMB")
{
  gSystem->Load("libfun4all.so");
  gSystem->Load("libPHPythia8.so");

  //---------------
  // Fun4All server
  //---------------
  Fun4AllServer *se = Fun4AllServer::instance();
  se->Verbosity(0);
  // just if we set some flags somewhere in this macro
  recoConsts *rc = recoConsts::instance();

  //-----------------
  // Event generation
  //-----------------

  PHPythia8 *pythia8 = new PHPythia8();
  // see coresoftware/generators/PHPythia8 for example config
  pythia8->set_config_file(inputFile);

  pythia8->print_config();

  se->registerSubsystem(pythia8);

  //-----------------
  // output
  //-----------------
  Fun4AllHepMCOutputManager *out1 = new Fun4AllHepMCOutputManager("HepMCOUT", string(outputFile) + string("_hepmc.dat.gz"));
  out1->set_embedding_id(1);  //! output the pythia sub-event
  se->registerOutputManager(out1);

  //-----------------
  // Event processing
  //-----------------
  se->run(nEvents);
  se->End();
  se->PrintTimer();
  std::cout << "All done" << std::endl;
  delete se;
  gSystem->Exit(0);
}
