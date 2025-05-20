#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllDstInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <fun4all/Fun4AllRunNodeInputManager.h>
#include <fun4all/Fun4AllUtils.h>

#include <phool/PHRandomSeed.h>
#include <phool/recoConsts.h>

// Micromegas
#include <micromegas/MicromegasCombinedDataDecoder.h>
#include <micromegas/MicromegasCombinedDataEvaluation.h>
#include <micromegas/MicromegasClusterizer.h>

// INTT
#include <intt/InttCombinedRawDataDecoder.h>
#include <intt/InttClusterizer.h>

// TPC
#include <tpc/TpcCombinedRawDataUnpacker.h>

#include <tpccalib/PHTpcResiduals.h>

#include <trackingdiagnostics/TrackResiduals.h>

// own modules
#include <g4eval_hp/EventCounter_hp.h>
#include <g4eval_hp/MicromegasEvaluator_hp.h>
#include <g4eval_hp/MicromegasClusterEvaluator_hp.h>
#include <g4eval_hp/MicromegasTrackEvaluator_hp.h>
#include <g4eval_hp/TrackingEvaluator_hp.h>


// local macros
#include "G4Setup_sPHENIX.C"
#include "G4_Global.C"

#include "Trkr_RecoInit.C"
#include "Trkr_TpcReadoutInit.C"

#include "Trkr_Clustering.C"
#include "Trkr_Reco.C"

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libg4eval_hp.so)

R__LOAD_LIBRARY(libmicromegas.so)

#include "make_filelist.C"

//____________________________________________________________________
int Fun4All_CombinedDataReconstruction_zf_hp(
  const int nEvents = 10,
  const int nSkipEvents = 0,
  const char* tag = "ana464_nocdbtag_v001",
  const int runnumber = 53534,
  const int segment = 0,
  const char* outputFile =  "DST/CONDOR_CombinedDataReconstruction/dst_eval-00053756-0000_corrected.root",
  const char* tpcResidualsFile = "DST/CONDOR_CombinedDataReconstruction/PHTpcResiduals-00053285-0000.root"

  )
{
  // print inputs
  std::cout << "Fun4All_CombinedDataReconstruction - nEvents: " << nEvents << std::endl;
  std::cout << "Fun4All_CombinedDataReconstruction - inputFile: " << inputFile << std::endl;
  std::cout << "Fun4All_CombinedDataReconstruction - outputFile: " << outputFile << std::endl;
  std::cout << "Fun4All_CombinedDataReconstruction - residualFile: " << residualFile << std::endl;

  TRACKING::pp_mode = true;
  G4TRACKING::SC_CALIBMODE = false;
  G4TRACKING::convert_seeds_to_svtxtracks = true;

  // condition database
  Enable::CDB = true;

  // readout initialization
  const auto [runnumber,segment] = Fun4AllUtils::GetRunSegment(inputFile);
  std::cout<< "Fun4All_CombinedDataReconstruction - runnumber: " << runnumber << std::endl;
  std::cout<< "Fun4All_CombinedDataReconstruction - segment: " << segment << std::endl;

  // reco const
  auto rc = recoConsts::instance();
  rc->set_IntFlag("RUNNUMBER", runnumber);
  rc->set_IntFlag("RUNSEGMENT", segment);
  rc->set_StringFlag("CDB_GLOBALTAG", "ProdA_2024");
  rc->set_uint64Flag("TIMESTAMP", runnumber);

  TpcReadoutInit( runnumber );

  // Ar/CF4
  // G4TPC::tpc_drift_velocity_reco = 0.00815238095238;

  // Ar/iC4H10/CF4 (default)
  // G4TPC::tpc_drift_velocity_reco = 0.00714;
  // G4TPC::tpc_drift_velocity_reco = 0.00726182; // from run 50015

  // try get drift velocity from CDB
  auto cdb = CDBInterface::instance();
  const auto tpc_dv_calib_dir = cdb->getUrl("TPC_DRIFT_VELOCITY");
  if (tpc_dv_calib_dir.empty())
  {
    std::cout << "Fun4All_CombinedDataReconstruction - No calibrated TPC drift velocity for Run " << runnumber << ". Use default value " << G4TPC::tpc_drift_velocity_reco << " cm/ns" << std::endl;
  }
  else
  {
    CDBTTree cdbttree(tpc_dv_calib_dir);
    cdbttree.LoadCalibrations();
    G4TPC::tpc_drift_velocity_reco = cdbttree.GetSingleFloatValue("tpc_drift_velocity");
    std::cout << "Fun4All_CombinedDataReconstruction - Use calibrated TPC drift velocity for Run " << runnumber << ": " << G4TPC::tpc_drift_velocity_reco << " cm/ns" << std::endl;
  }

  std::cout<< "Fun4All_CombinedDataReconstruction - pp_mode: " << TRACKING::pp_mode << std::endl;
  std::cout<< "Fun4All_CombinedDataReconstruction - samples: " << TRACKING::reco_tpc_maxtime_sample << std::endl;
  std::cout<< "Fun4All_CombinedDataReconstruction - pre: " << TRACKING::reco_tpc_time_presample << std::endl;
  std::cout<< "Fun4All_CombinedDataReconstruction - vdrift: " << G4TPC::tpc_drift_velocity_reco << std::endl;

  ACTSGEOM::mvtxMisalignment = 100;
  ACTSGEOM::inttMisalignment = 100;
  ACTSGEOM::tpotMisalignment = 100;

  // server
  auto se = Fun4AllServer::instance();
  // se->Verbosity(2);

  PHRandomSeed::Verbosity(1);

  // event counter
  se->registerSubsystem( new EventCounter_hp( "EventCounter_hp", 1 ) );

  // tracking geometry
  std::string geofile = CDBInterface::instance()->getUrl("Tracking_Geometry");
  std::cout << "Fun4All_CombinedDataReconstruction_hp - geofile: " << geofile << std::endl;

  auto ingeo = new Fun4AllRunNodeInputManager("GeoIn");
  ingeo->AddFile(geofile);
  se->registerInputManager(ingeo);

  if(true)
  {
    // module edge corrections
    G4TPC::ENABLE_MODULE_EDGE_CORRECTIONS = false;

    // static distortions
    // static distortion corrections are disabled for zero field runs
    G4TPC::ENABLE_STATIC_CORRECTIONS = false;

    // average distortions
    G4TPC::ENABLE_AVERAGE_CORRECTIONS = false;
  }

  // tpc zero suppression
  TRACKING::tpc_zero_supp = true;

  // from Tony
  G4MAGNET::magfield = "0.01";
  G4MAGNET::magfield_tracking = G4MAGNET::magfield;
  G4MAGNET::magfield_rescale = 1;
  TrackingInit();

  // input manager
  auto in = new Fun4AllDstInputManager("DSTin");
  in->fileopen(inputFile);
  se->registerInputManager(in);

  // hit unpackers
  Mvtx_HitUnpacking();
  Intt_HitUnpacking();
  if( true )
  {

    // custom TPC unpacking
    auto tpcunpacker = new TpcCombinedRawDataUnpacker("TpcCombinedRawDataUnpacker");
    tpcunpacker->set_presampleShift(TRACKING::reco_tpc_time_presample);
    tpcunpacker->set_t0(-4);

    if(TRACKING::tpc_zero_supp)
    {
      tpcunpacker->ReadZeroSuppressedData();
    }
    tpcunpacker->doBaselineCorr(TRACKING::tpc_baseline_corr);

    se->registerSubsystem(tpcunpacker);

  } else {

    // official TPC unpacking
    Tpc_HitUnpacking();

  }

  Micromegas_HitUnpacking();

  Mvtx_Clustering();
  Intt_Clustering();

  auto tpcclusterizer = new TpcClusterizer;
  tpcclusterizer->Verbosity(0);
  tpcclusterizer->set_do_hit_association(G4TPC::DO_HIT_ASSOCIATION);
  tpcclusterizer->set_rawdata_reco();
  se->registerSubsystem(tpcclusterizer);

//   Tpc_LaserEventIdentifying();

  Micromegas_Clustering();

  std::cout << "Fun4All_CombinedDataReconstruction_zf_hp - G4TPC::TPC_GAS_MIXTURE: " << G4TPC::TPC_GAS_MIXTURE  << std::endl;
  Tracking_Reco_TrackSeed_ZeroField();

  {
    auto silicon_match = new PHSiliconTpcTrackMatching;
    silicon_match->Verbosity(0);
    silicon_match->set_x_search_window(0.36);
    silicon_match->set_y_search_window(0.36);
    silicon_match->set_z_search_window(2.5);
    silicon_match->set_phi_search_window(0.014);
    silicon_match->set_eta_search_window(0.0091);
    silicon_match->set_test_windows_printout(false);
    silicon_match->set_pp_mode(TRACKING::pp_mode);
    silicon_match->zeroField(true);
    se->registerSubsystem(silicon_match);
  }

  if( true )
  {
    // matching with micromegas
    /* using extended windows */
    auto mm_match = new PHMicromegasTpcTrackMatching;
    mm_match->Verbosity(0);
    mm_match->zeroField(true);
    mm_match->set_use_silicon(true);

    mm_match->set_rphi_search_window_lyr1(3.0);
    mm_match->set_rphi_search_window_lyr2(15.0);

    mm_match->set_z_search_window_lyr1(30.0);
    mm_match->set_z_search_window_lyr2(3.0);

    mm_match->set_min_tpc_layer(38);             // layer in TPC to start projection fit
    mm_match->set_test_windows_printout(false);  // used for tuning search windows only
    se->registerSubsystem(mm_match);
  }

  if (G4TRACKING::convert_seeds_to_svtxtracks)
  {

    auto converter = new TrackSeedTrackMapConverter;
    // Default set to full SvtxTrackSeeds. Can be set to
    // SiliconTrackSeedContainer or TpcTrackSeedContainer
    converter->setTrackSeedName("SvtxTrackSeedContainer");
    converter->setFieldMap(G4MAGNET::magfield_tracking);
    converter->Verbosity(0);
    converter->constField();
    se->registerSubsystem(converter);

  } else {

    {
      // track fit
      // perform final track fit with ACTS
      auto actsFit = new PHActsTrkFitter;
      actsFit->Verbosity(0);
      actsFit->commissioning(G4TRACKING::use_alignment);

      // fit with Micromegas and Silicon ONLY
      actsFit->fitSiliconMMs(G4TRACKING::SC_CALIBMODE);
      actsFit->setUseMicromegas(G4TRACKING::SC_USE_MICROMEGAS);

      actsFit->set_pp_mode(TRACKING::pp_mode);
      actsFit->set_use_clustermover(true);
      actsFit->useActsEvaluator(false);
      actsFit->useOutlierFinder(false);
      actsFit->setFieldMap(G4MAGNET::magfield_tracking);
      se->registerSubsystem(actsFit);
    }

    if(false)
    {
      auto cleaner = new PHTrackCleaner();
      cleaner->Verbosity(0);
      se->registerSubsystem(cleaner);
    }
  }

  {
    // vertex finding
    auto finder = new PHSimpleVertexFinder;
    finder->Verbosity(0);
    finder->setDcaCut(0.5);
    finder->setTrackPtCut(-99999.);
    finder->setBeamLineCut(1);
    finder->setTrackQualityCut(1e9);
    finder->setNmvtxRequired(3);
    finder->setOutlierPairCut(0.1);

    se->registerSubsystem(finder);
  }

  // residual tree from tony
  // disabled because it is very large
  // achieves the same as TrackEvaluator_hp
  if( false )
  {
    auto resid = new TrackResiduals("TrackResiduals");
    resid->outfileName(residualFile);
    resid->Verbosity(0);
    resid->alignment(false);

    resid->convertSeeds(G4TRACKING::convert_seeds_to_svtxtracks);
    se->registerSubsystem(resid);
  }

  // own evaluator
  if( false )
  {
    auto trackingEvaluator = new TrackingEvaluator_hp;

    trackingEvaluator->set_flags(
      TrackingEvaluator_hp::EvalEvent
      |TrackingEvaluator_hp::EvalTracks
//       |TrackingEvaluator_hp::MicromegasOnly
      );
    se->registerSubsystem(trackingEvaluator);
  }

  // own evaluator
  if( true )
  {
    auto micromegasTrackEvaluator = new MicromegasTrackEvaluator_hp;
    micromegasTrackEvaluator->set_zero_field(true);
    micromegasTrackEvaluator->set_use_silicon(true);
    micromegasTrackEvaluator->set_min_tpc_layer(39);
    micromegasTrackEvaluator->set_max_tpc_layer(55);
    se->registerSubsystem(micromegasTrackEvaluator);
  }

  // output manager
  if( true )
  {
    auto out = new Fun4AllDstOutputManager("DSTOUT", outputFile);
    out->AddNode("TrackingEvaluator_hp::Container");
    out->AddNode("MicromegasTrackEvaluator_hp::Container");
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
