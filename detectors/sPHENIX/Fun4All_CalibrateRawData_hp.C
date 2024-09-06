#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllDstOutputManager.h>

#include <fun4allraw/Fun4AllPrdfInputManager.h>

#include <g4main/PHG4SimpleEventGenerator.h>
#include <phool/PHRandomSeed.h>
#include <phool/recoConsts.h>

#include <micromegas/MicromegasRawDataCalibration.h>
#include <micromegas/MicromegasRawDataEvaluation.h>

// own modules
#include <g4eval_hp/EventCounter_hp.h>

// local macros
#include "G4Setup_sPHENIX.C"
#include "G4_Global.C"

#include "Trkr_RecoInit.C"
#include "Trkr_Clustering.C"
#include "Trkr_Reco.C"

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libg4eval_hp.so)
R__LOAD_LIBRARY(libfun4allraw.so)

R__LOAD_LIBRARY(libmicromegas.so)

//____________________________________________________________________
int Fun4All_CalibrateRawData_hp(
  const int nEvents = 500,

//   const char* inputFile = "LUSTRE/cosmics/TPOT_ebdc39_cosmics-00039495-0000.evt",
//   const char* evaluationFile = "DST/MicromegasRawDataEvaluation-00039495-0000.root",
//   const char* calibrationFile = "Calibrations/TPOT_Pedestal-00039495-0000.root"

  const char* inputFile = "LUSTRE_PHYSICS/junk/TPOT_ebdc39_junk-00052123-0000.evt",
  const char* evaluationFile = "DST/MicromegasRawDataEvaluation-00052123-0000.root",
  const char* calibrationFile = "Calibrations/TPOT_Pedestal-00052123-0000.root"

//   const char* inputFile = "LUSTRE_PHYSICS/junk/TPOT_ebdc39_junk-00045955-0000.evt",
//   const char* evaluationFile = "DST/MicromegasRawDataEvaluation-00045955-0000.root",
//   const char* calibrationFile = "Calibrations/TPOT_Pedestal-00045955-0000.root"

//   const char* inputFile = "LUSTRE_PHYSICS/junk/TPOT_ebdc39_junk-00052122-0000.evt",
//   const char* evaluationFile = "DST/MicromegasRawDataEvaluation-00052122-0000.root",
//   const char* calibrationFile = "Calibrations/TPOT_Pedestal-00052122-0000.root"

  )
{
  // print inputs
  std::cout << "Fun4All_ReadRawData_hp - nEvents: " << nEvents << std::endl;
  std::cout << "Fun4All_ReadRawData_hp - inputFile: " << inputFile << std::endl;
  std::cout << "Fun4All_ReadRawData_hp - evaluationFile: " << evaluationFile << std::endl;
  std::cout << "Fun4All_ReadRawData_hp - calibrationFile: " << calibrationFile << std::endl;

  // options
  Enable::PIPE = true;
  Enable::MBD = true;
  Enable::MAGNET = true;
  Enable::PLUGDOOR = false;

  // enable all absorbers
  // this is equivalent to the old "absorberactive" flag
  Enable::ABSORBER = true;

  // central tracking
  Enable::MVTX = true;
  Enable::INTT = true;
  Enable::TPC = true;
  Enable::MICROMEGAS = true;
  Enable::BLACKHOLE = true;

  // server
  auto se = Fun4AllServer::instance();
  // se->Verbosity(1);

  // make sure to printout random seeds for reproducibility
  PHRandomSeed::Verbosity(1);

  // reco const
  auto rc = recoConsts::instance();
  // rc->set_IntFlag("RANDOMSEED",PHRandomSeed());
  // rc->set_IntFlag("RANDOMSEED",1);

  // event counter
  se->registerSubsystem( new EventCounter_hp( "EventCounter_hp", 10 ) );

  // Geant4 initialization
  G4Init();

  // raw data evaluation
  if( true )
  {
    // raw data evaluation
    auto micromegasRawDataEvaluation = new MicromegasRawDataEvaluation;
    // micromegasRawDataEvaluation->Verbosity(1);
    micromegasRawDataEvaluation->set_evaluation_outputfile(evaluationFile);
    micromegasRawDataEvaluation->set_sample_min(0);
    micromegasRawDataEvaluation->set_sample_max(20);
    se->registerSubsystem( micromegasRawDataEvaluation );
  }

  if( true )
  {
    // raw data calibration
    auto micromegasRawDataCalibration = new MicromegasRawDataCalibration;
    micromegasRawDataCalibration->set_calibration_file(calibrationFile);
    micromegasRawDataCalibration->set_sample_min(0);
    micromegasRawDataCalibration->set_sample_max(20);
    se->registerSubsystem( micromegasRawDataCalibration );
  }

  // input manager
  auto in = new Fun4AllPrdfInputManager;
  in->fileopen(inputFile);
  se->registerInputManager(in);

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
