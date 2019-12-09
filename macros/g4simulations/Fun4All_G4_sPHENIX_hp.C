#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllDummyInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <g4main/PHG4SimpleEventGenerator.h>
#include <phool/recoConsts.h>

// own modules
#include <g4eval/EventCounter_hp.h>
#include <g4eval/TrackingEvaluator_hp.h>

R__ADD_INCLUDE_PATH( /phenix/u/hpereira/sphenix/src/macros/macros/g4simulations )
#include "G4Setup_sPHENIX.C"
#include "G4_Bbc.C"

R__LOAD_LIBRARY(libfun4all.so)

//____________________________________________________________________
int Fun4All_G4_sPHENIX_hp( const int nEvents = 500, const char *outputFile = "DST/dst_eval_500_full_beta2.root" )
{

  // options
  bool do_pipe = true;

  bool do_pstof = false;
  bool do_cemc = false;
  bool do_hcalin = false;
  bool do_magnet = false;
  bool do_hcalout = false;
  bool do_plugdoor = false;

  bool do_tracking = true;

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
  se->Verbosity(0);

  auto rc = recoConsts::instance();
  // rc->set_IntFlag("RANDOMSEED", 1);

  // event counter
  se->registerSubsystem( new EventCounter_hp() );

  // event generation
  // toss low multiplicity dummy events
  auto gen = new PHG4SimpleEventGenerator();
  gen->add_particles("pi+",1);

  gen->set_vertex_distribution_function(
    PHG4SimpleEventGenerator::Uniform,
    PHG4SimpleEventGenerator::Uniform,
    PHG4SimpleEventGenerator::Uniform);

  gen->set_vertex_distribution_mean(0.0, 0.0, 0.0);
  gen->set_vertex_distribution_width(0.0, 0.0, 5.0);

  // TODO: what are vertex_size
  gen->set_vertex_size_function(PHG4SimpleEventGenerator::Uniform);
  gen->set_vertex_size_parameters(0.0, 0.0);
  gen->set_eta_range(-1.0, 1.0);
  gen->set_phi_range(-1.0 * TMath::Pi(), 1.0 * TMath::Pi());

  gen->set_pt_range(0.1, 20.0);
  // gen->set_pt_range(0.5, 5.0);

  gen->Embed(2);
  gen->Verbosity(0);
  se->registerSubsystem(gen);

  // G4 setup
  G4Setup(
    absorberactive, magfield, EDecayType::kAll,
    do_tracking, do_pstof, do_cemc, do_hcalin, do_magnet, do_hcalout, do_pipe, do_plugdoor,
    magfield_rescale);

  // bbc reconstruction
  BbcInit();
  Bbc_Reco();

  // tracking
  Tracking_Cells();
  Tracking_Reco();

  // local evaluation
  se->registerSubsystem(new TrackingEvaluator_hp( "TRACKINGEVALUATOR_HP" ));

  // for single particle generators we just need something which drives
  // the event loop, the Dummy Input Mgr does just that
  auto in = new Fun4AllDummyInputManager("JADE");
  se->registerInputManager(in);

  // output manager
  /* all the nodes from DST and RUN are saved to the output */
  Fun4AllDstOutputManager *out = new Fun4AllDstOutputManager("DSTOUT", outputFile);
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
