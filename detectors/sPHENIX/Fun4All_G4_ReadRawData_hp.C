#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllDstOutputManager.h>

#include <fun4allraw/Fun4AllPrdfInputManager.h>

#include <g4main/PHG4SimpleEventGenerator.h>
#include <phool/PHRandomSeed.h>
#include <phool/recoConsts.h>

#include <micromegas/MicromegasRawDataDecoder.h>
#include <micromegas/MicromegasRawDataCalibration.h>
#include <micromegas/MicromegasRawDataEvaluation.h>

// own modules
#include <g4eval_hp/EventCounter_hp.h>

// local macros
#include "G4Setup_sPHENIX.C"
#include "G4_Bbc.C"
#include "G4_Global.C"

#include "Trkr_RecoInit.C"
#include "Trkr_Clustering.C"
#include "Trkr_Reco.C"

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libg4eval_hp.so)
R__LOAD_LIBRARY(libfun4allraw.so)

R__LOAD_LIBRARY(libmicromegas.so)

//____________________________________________________________________
int Fun4All_G4_ReadRawData_hp(
  const int nEvents = 1000,
  const char* inputFile = "LUSTRE/physics/TPOT_ebdc39_physics-00007363-0000.prdf",
  const char* outputFile = "DST/dst_eval-00007363-0000.root",
  const char* evaluationFile = "DST/MicromegasRawDataEvaluation-00007363-0000.root"
  )
{

  // print inputs
  std::cout << "Fun4All_G4_ReadRawData_hp - nEvents: " << nEvents << std::endl;
  std::cout << "Fun4All_G4_ReadRawData_hp - inputFile: " << inputFile << std::endl;
  std::cout << "Fun4All_G4_ReadRawData_hp - evaluationFile: " << evaluationFile << std::endl;

  // options
  Enable::PIPE = true;
  Enable::BBC = true;
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

  if( false )
  {

    // Geant4 initialization
    G4Init();
    
    // raw data calibration
    auto micromegasRawDataCalibration = new MicromegasRawDataCalibration;
    se->registerSubsystem( micromegasRawDataCalibration );
  }
  
  if( true )
  {  
    // condition database
    Enable::CDB = true;
    rc->set_StringFlag("CDB_GLOBALTAG",CDB::global_tag);
    rc->set_uint64Flag("TIMESTAMP",CDB::timestamp);

    G4Init();
    G4Setup();

    ACTSGEOM::ActsGeomInit();

    // raw data decoding
    auto micromegasRawDataDecoder = new MicromegasRawDataDecoder;
    // micromegasRawDataDecoder->Verbosity(1);
    micromegasRawDataDecoder->set_sample_min( 30 );
    micromegasRawDataDecoder->set_sample_max( 50 );
    se->registerSubsystem( micromegasRawDataDecoder );
    
    // Micromegas clustering
    auto mm_clus = new MicromegasClusterizer;
    mm_clus->set_cluster_version(G4TRACKING::cluster_version);
    se->registerSubsystem(mm_clus);
    
  }
      
  if( false )
  {  
    // raw data evaluation
    auto micromegasRawDataEvaluation = new MicromegasRawDataEvaluation;
    micromegasRawDataEvaluation->Verbosity(1);
    micromegasRawDataEvaluation->set_evaluation_outputfile(evaluationFile);
    se->registerSubsystem( micromegasRawDataEvaluation );
  }

  if( true )
  {
    auto trackingEvaluator = new TrackingEvaluator_hp;
    trackingEvaluator->set_flags(
      |TrackingEvaluator_hp::EvalClusters
      );

    se->registerSubsystem(trackingEvaluator);
  }

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
