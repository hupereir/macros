#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllDummyInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <g4main/PHG4SimpleEventGenerator.h>
#include <phool/PHRandomSeed.h>
#include <phool/recoConsts.h>

// own modules
#include <g4eval_hp/EventCounter_hp.h>

// local macros
#include <G4Setup_sPHENIX.C>
#include <G4_Global.C>
#include <G4_Input.C>

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libg4eval_hp.so)

//____________________________________________________________________
int Fun4All_G4_sPHENIX_single_sim_hp(
  const int nEvents = 10,
  const int pid = 11,
  const char* outputFile = "DST/G4Hits_single.root"
  )
{

  // print inputs
  std::cout << "Fun4All_G4_sPHENIX_single_sim_hp - nEvents: " << nEvents << std::endl;
  std::cout << "Fun4All_G4_sPHENIX_single_sim_hp - pid: " << pid << std::endl;
  std::cout << "Fun4All_G4_sPHENIX_single_sim_hp - outputFile: " << outputFile << std::endl;

  // options
  Enable::PIPE = true;
  Enable::MBD = false;
  Enable::MBDFAKE = true;
  Enable::PLUGDOOR = false;

  // enable all absorbers
  // this is equivalent to the old "absorberactive" flag
  Enable::ABSORBER = false;

  // central tracking
  Enable::MVTX = true;
  Enable::INTT = true;
  Enable::TPC = true;
  Enable::MICROMEGAS = true;
  Enable::BLACKHOLE = true;

  Enable::CEMC = true;
  Enable::HCALIN = true;
  Enable::MAGNET = true;
  Enable::HCALOUT = true;

  Enable::CEMC_ABSORBER = false;
  Enable::HCALIN_ABSORBER = false;
  Enable::MAGNET_ABSORBER = false;
  Enable::HCALOUT_ABSORBER = false;

  // server
  auto se = Fun4AllServer::instance();
  // se->Verbosity(1);

  // make sure to printout random seeds for reproducibility
  PHRandomSeed::Verbosity(1);

  // reco const
  auto rc = recoConsts::instance();
  Enable::CDB = true;
  rc->set_StringFlag("CDB_GLOBALTAG",CDB::global_tag);
  rc->set_uint64Flag("TIMESTAMP",CDB::timestamp);

  // event counter
  se->registerSubsystem( new EventCounter_hp( "EventCounter_hp", 1 ) );

  {
    // event generation
    auto gen = new PHG4SimpleEventGenerator;

    // single electrons
    gen->add_particles(pid,1);
    gen->set_eta_range(-1.0, 1.0);
    gen->set_phi_range(-M_PI, M_PI);
    gen->set_pt_range(0.2, 20.0);

    // vertex
    gen->set_vertex_distribution_function(
      PHG4SimpleEventGenerator::Gaus,
      PHG4SimpleEventGenerator::Gaus,
      PHG4SimpleEventGenerator::Gaus);
    gen->set_vertex_distribution_mean(0.0, 0.0, 0.0);
    gen->set_vertex_distribution_width(0.01, 0.01, 5.0);

    gen->Embed(2);
    se->registerSubsystem(gen);
  }

  // Geant4 initialization
  G4Init();
  G4Setup();

  // for single particle generators we just need something which drives
  // the event loop, the Dummy Input Mgr does just that
  auto in = new Fun4AllDummyInputManager("JADE");
  se->registerInputManager(in);

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
