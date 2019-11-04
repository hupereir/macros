#if ROOT_VERSION_CODE >= ROOT_VERSION(6,00,0)
#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllInputManager.h>
#include <fun4all/Fun4AllDummyInputManager.h>
#include <fun4all/Fun4AllOutputManager.h>
#include <fun4all/Fun4AllDstInputManager.h>
#include <fun4all/Fun4AllNoSyncDstInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <g4main/PHG4ParticleGeneratorBase.h>
#include <g4main/PHG4ParticleGenerator.h>
#include <g4main/PHG4SimpleEventGenerator.h>
#include <g4main/PHG4ParticleGeneratorVectorMeson.h>
#include <g4main/PHG4ParticleGun.h>
#include <g4main/HepMCNodeReader.h>
#include <g4detectors/PHG4DetectorSubsystem.h>
#include <phool/recoConsts.h>
#include <phpythia6/PHPythia6.h>
#include <phpythia8/PHPythia8.h>
#include <phhepmc/Fun4AllHepMCPileupInputManager.h>
#include <phhepmc/Fun4AllHepMCInputManager.h>

R__ADD_INCLUDE_PATH( /phenix/u/hpereira/sphenix/src/tony/macros/macros/g4simulations )
#include "G4Setup_sPHENIX.C"
#include "G4_Bbc.C"
#include "G4_Global.C"
#include "G4_CaloTrigger.C"
#include "G4_Jets.C"
#include "G4_HIJetReco.C"
#include "G4_DSTReader.C"
#include "DisplayOn.C"

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libg4testbench.so)
R__LOAD_LIBRARY(libphhepmc.so)
R__LOAD_LIBRARY(libPHPythia6.so)
R__LOAD_LIBRARY(libPHPythia8.so)
#endif

using namespace std;


int Fun4All_G4_sPHENIX_hp( const int nEvents = 5000, const char *outputFile = "rootfiles/G4sPHENIX" )
{

  //===============
  // Input options
  //===============


  //======================
  // What to run
  //======================

  bool do_bbc = true;
  bool do_pipe = true;

  bool do_tracking = true;
  bool do_tracking_cell = do_tracking && true;
  bool do_tracking_track = do_tracking_cell && true;
  bool do_tracking_eval = do_tracking_track && true;

  bool do_pstof = false;
  bool do_cemc = false;
  bool do_hcalin = false;
  bool do_magnet = false;
  bool do_hcalout = false;
  bool do_plugdoor = false;

  //---------------
  // Load libraries
  //---------------

  gSystem->Load("libfun4all.so");
  gSystem->Load("libg4detectors.so");
  gSystem->Load("libphhepmc.so");
  gSystem->Load("libg4testbench.so");
  gSystem->Load("libg4eval.so");
  gSystem->Load("libg4intt.so");

  // establish the geometry and reconstruction setup
  gROOT->SetMacroPath( "/phenix/u/hpereira/sphenix/src/tony/macros/macros/g4simulations" );
  gROOT->LoadMacro("G4Setup_sPHENIX.C");
  G4Init(do_tracking, do_pstof, do_cemc, do_hcalin, do_magnet, do_hcalout, do_pipe, do_plugdoor);

  int absorberactive = 1;  // set to 1 to make all absorbers active volumes

  //  const string magfield = "1.5"; // alternatively to specify a constant magnetic field, give a float number, which will be translated to solenoidal field in T, if string use as fieldmap name (including path)
  const std::string magfield = string(getenv("CALIBRATIONROOT")) + string("/Field/Map/sPHENIX.2d.root"); // default map from the calibration database
  const float magfield_rescale = -1.4 / 1.5;                                     // scale the map to a 1.4 T field

  //---------------
  // Fun4All server
  //---------------

  Fun4AllServer *se = Fun4AllServer::instance();
  se->Verbosity(0);
  recoConsts *rc = recoConsts::instance();

  //-----------------
  // Event generation
  //-----------------

  // toss low multiplicity dummy events
  {
    PHG4SimpleEventGenerator *gen = new PHG4SimpleEventGenerator();
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
    gen->Embed(2);
    gen->Verbosity(0);

    se->registerSubsystem(gen);
  }

  // G4 setup
  {
    G4Setup(
      absorberactive, magfield, EDecayType::kAll,
      do_tracking, do_pstof, do_cemc, do_hcalin, do_magnet, do_hcalout, do_pipe, do_plugdoor,
      magfield_rescale);
  }

  {
    // bbc reconstruction
    gROOT->LoadMacro("G4_Bbc.C");
    BbcInit();
    Bbc_Reco();
  }

  {
    // tracking
    Tracking_Cells();
    Tracking_Reco();
    Tracking_Eval(string(outputFile) + "_g4svtx_eval.root");
  }

  {
    // for single particle generators we just need something which drives
    // the event loop, the Dummy Input Mgr does just that
    Fun4AllInputManager *in = new Fun4AllDummyInputManager("JADE");
    se->registerInputManager(in);
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
