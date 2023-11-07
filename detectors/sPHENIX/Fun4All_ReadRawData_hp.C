#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllDstOutputManager.h>

#include <fun4allraw/Fun4AllPrdfInputManager.h>

#include <g4main/PHG4SimpleEventGenerator.h>
#include <phool/PHRandomSeed.h>
#include <phool/recoConsts.h>

#include <micromegas/MicromegasRawDataDecoder.h>

// own modules
#include <g4eval_hp/EventCounter_hp.h>
#include <g4eval_hp/MicromegasClusterEvaluator_hp.h>

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
int Fun4All_ReadRawData_hp(
  const int nEvents = 0,
  const char* inputFile = "LUSTRE/junk/TPOT_ebdc39_junk-00020121-0000.prdf",
  const char* outputFile =  "DST/CONDOR_RawDataEvaluation/dst_eval-00020121-0000-test.root",
  const char* calibrationFile = "DST/TPOT_Pedestal-00009416-0000.root"
  )
{  
  // print inputs
  std::cout << "Fun4All_ReadRawData_hp - nEvents: " << nEvents << std::endl;
  std::cout << "Fun4All_ReadRawData_hp - inputFile: " << inputFile << std::endl;
  std::cout << "Fun4All_ReadRawData_hp - outputFile: " << outputFile << std::endl;
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
  se->registerSubsystem( new EventCounter_hp( "EventCounter_hp", 1 ) );

  // condition database
  Enable::CDB = true;
  rc->set_StringFlag("CDB_GLOBALTAG",CDB::global_tag);
  rc->set_uint64Flag("TIMESTAMP",CDB::timestamp);
  
  G4Init();
  G4Setup();
  
  ACTSGEOM::ActsGeomInit();
  
  auto micromegasRawDataDecoder = new MicromegasRawDataDecoder;
  micromegasRawDataDecoder->set_calibration_file(calibrationFile);
//   micromegasRawDataDecoder->set_sample_min( 15 );
//   micromegasRawDataDecoder->set_sample_max( 35 );
  micromegasRawDataDecoder->set_sample_min( 20 );
  micromegasRawDataDecoder->set_sample_max( 45 );
  se->registerSubsystem( micromegasRawDataDecoder );
    
  // Micromegas clustering
  se->registerSubsystem(new MicromegasClusterizer);   
  
  // evaluation
  se->registerSubsystem( new MicromegasClusterEvaluator_hp );
  
  // for single particle generators we just need something which drives
  // the event loop, the Dummy Input Mgr does just that
  auto in = new Fun4AllPrdfInputManager;
  in->fileopen(inputFile);
  se->registerInputManager(in);

  // output manager
  if( true )
  {
    auto out = new Fun4AllDstOutputManager("DSTOUT", outputFile);
    se->registerOutputManager(out);
  }
  
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
