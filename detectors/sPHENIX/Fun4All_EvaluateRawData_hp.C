#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllDstOutputManager.h>

#include <fun4allraw/Fun4AllPrdfInputManager.h>

#include <g4main/PHG4SimpleEventGenerator.h>
#include <phool/PHRandomSeed.h>
#include <phool/recoConsts.h>

#include <fun4allraw/MicromegasBcoMatchingInformation.h>
#include <micromegas/MicromegasRawDataTimingEvaluation.h>

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
R__LOAD_LIBRARY(libmbd.so)

R__LOAD_LIBRARY(libmicromegas.so)

//____________________________________________________________________
int Fun4All_EvaluateRawData_hp(
  const int nEvents = 500,

//   const char* inputFile = "LUSTRE_PHYSICS/junk/TPOT_ebdc39_junk-00043402-0000.evt",
//   const char* evaluationFile =  "DST/CONDOR_RawDataEvaluation/MicromegasRawDataTimingEvaluation-00043402-0000-test.root"

//   const char* inputFile = "LUSTRE_PHYSICS/physics/TPOT_ebdc39_physics-00044284-0000.evt",
//   const char* evaluationFile =  "DST/CONDOR_RawDataEvaluation/MicromegasRawDataTimingEvaluation-00044284-0000.root"

//   const char* inputFile = "LUSTRE_PHYSICS/beam/TPOT_ebdc39_beam-00043817-0024.evt",
//   const char* evaluationFile =  "DST/CONDOR_RawDataEvaluation/MicromegasRawDataTimingEvaluation-00043817-0000.root"

//   const char* inputFile = "LUSTRE_PHYSICS/physics/TPOT_ebdc39_physics-00044380-0000.evt",
//   const char* evaluationFile =  "DST/CONDOR_RawDataEvaluation/MicromegasRawDataTimingEvaluation-00044380-0000.root"


//   const char* inputFile ="LUSTRE_PHYSICS/physics/TPOT_ebdc39_physics-00045288-0000.evt",
//   const char* evaluationFile =  "DST/CONDOR_RawDataEvaluation/MicromegasRawDataTimingEvaluation-00045288-0000-test.root"

//   const char* inputFile ="LUSTRE_PHYSICS/physics/TPOT_ebdc39_physics-00045490-0000.evt",
//   const char* evaluationFile =  "DST/CONDOR_RawDataEvaluation/MicromegasRawDataTimingEvaluation-00045490-0000-test.root"

  const char* inputFile ="LUSTRE_PHYSICS/physics/TPOT_ebdc39_physics-00045550-0000.evt",
  const char* evaluationFile =  "DST/CONDOR_RawDataEvaluation/MicromegasRawDataTimingEvaluation-00045550-0000-test.root"

)
{
  // print inputs
  std::cout << "Fun4All_EvaluateRawData_hp - nEvents: " << nEvents << std::endl;
  std::cout << "Fun4All_EvaluateRawData_hp - inputFile: " << inputFile << std::endl;
  std::cout << "Fun4All_EvaluateRawData_hp - evaluationFile: " << evaluationFile << std::endl;

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
  se->registerSubsystem( new EventCounter_hp( "EventCounter_hp", 1 ) );

  // raw data evaluation
  // MicromegasBcoMatchingInformation::set_gtm_clock_multiplier( 4.262916255 ); // for run 43817
  // MicromegasBcoMatchingInformation::set_gtm_clock_multiplier( 4.26291675 );  // for run 43402
  // MicromegasBcoMatchingInformation::set_gtm_clock_multiplier( 4.26291667 ); // for run 45288
  auto micromegasRawDataTimingEvaluation = new MicromegasRawDataTimingEvaluation;
  // micromegasRawDataTimingEvaluation->Verbosity(1);
  micromegasRawDataTimingEvaluation->set_evaluation_outputfile(evaluationFile);
  se->registerSubsystem( micromegasRawDataTimingEvaluation );

  // for single particle generators we just need something which drives
  // the event loop, the Dummy Input Mgr does just that
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

