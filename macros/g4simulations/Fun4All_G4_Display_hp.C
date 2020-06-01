#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllDummyInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <g4main/PHG4SimpleEventGenerator.h>
#include <phool/recoConsts.h>

// own modules
#include <tpccalib/TpcSpaceChargeReconstruction.h>
#include <trackreco/PHTruthClustering_hp.h>

R__ADD_INCLUDE_PATH( /phenix/u/hpereira/sphenix/src/macros/macros/g4simulations )
#include "G4Setup_sPHENIX.C"
#include "G4_Bbc.C"
#include "DisplayOn.C"

R__LOAD_LIBRARY(libfun4all.so)

//____________________________________________________________________
int Fun4All_G4_Display_hp( const int nEvents = 1 )
{

  // options
  const bool do_pipe = true;
  const bool do_pstof = false;
  const bool do_cemc = false;
  const bool do_hcalin = false;
  const bool do_magnet = false;
  const bool do_hcalout = false;
  const bool do_plugdoor = false;

  const bool do_tracking = true;
  const bool display_on = true;

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

  // G4 setup
  G4Setup(
    absorberactive, magfield, EDecayType::kAll,
    do_tracking, do_pstof, do_cemc, do_hcalin, do_magnet, do_hcalout, do_pipe, do_plugdoor,
    magfield_rescale);

  if(display_on)
  {
    DisplayOn();
    
    // prevent macro from finishing so can see display
    int i;
    cout << "***** Enter any integer to proceed" << endl;
    cin >> i;
  }
  
  std::cout << "All done" << std::endl;
  delete se;
  gSystem->Exit(0);
  return 0;
}

// This function is only used to test if we can load this as root6 macro
// without running into unresolved libraries and include files
void RunFFALoadTest() {}
