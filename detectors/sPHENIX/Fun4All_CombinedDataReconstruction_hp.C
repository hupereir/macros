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

//____________________________________________________________________
int Fun4All_CombinedDataReconstruction_hp(
  const int nEvents = 100,
  const char* inputFile = "/sphenix/lustre01/sphnxpro/commissioning/slurp/tpcbeam/run_00041900_00042000/DST_BEAM_run2pp_new_2023p013-00041989-0000.root",
  const char* outputFile =  "DST/CONDOR_CombinedDataReconstruction_corrected/dst_eval-00041989-0000.root",
  const char* residualFile = "DST/CONDOR_CombinedDataReconstruction_corrected/TrackResiduals-00041989-0000.root",
  const char* evaluationFile = "DST/CONDOR_CombinedDataReconstruction_corrected/MicromegasCombinedDataEvaluation-00041989-0000.root"

//   const char* inputFile = "/sphenix/lustre01/sphnxpro/physics/slurp/streaming/fast/run_00050400_00050500/DST_STREAMING_EVENT_run2ppfast_new_2024p002-00050450-00000.root",
//   const char* outputFile =  "DST/CONDOR_CombinedDataReconstruction_corrected/dst_eval-00050450-0000-test.root",
//   const char* residualFile = "DST/CONDOR_CombinedDataReconstruction_corrected/TrackResiduals-00050450-0000-test.root"

//   const char* inputFile = "/sphenix/lustre01/sphnxpro/physics/slurp/streaming/physics/new_2024p002/run_00051200_00051300/DST_STREAMING_EVENT_run2pp_new_2024p002-00051249-00000.root",
//   const char* outputFile =  "DST/CONDOR_CombinedDataReconstruction_corrected/dst_eval-00051249-0000-test.root",
//   const char* residualFile = "DST/CONDOR_CombinedDataReconstruction_corrected/TrackResiduals-00051249-0000-test.root"

  )
{
  // print inputs
  std::cout << "Fun4All_CombinedDataReconstruction - nEvents: " << nEvents << std::endl;
  std::cout << "Fun4All_CombinedDataReconstruction - inputFile: " << inputFile << std::endl;
  std::cout << "Fun4All_CombinedDataReconstruction - outputFile: " << outputFile << std::endl;
  std::cout << "Fun4All_CombinedDataReconstruction - residualFile: " << residualFile << std::endl;
  std::cout << "Fun4All_CombinedDataReconstruction - evaluationFile: " << evaluationFile << std::endl;

  TRACKING::pp_mode = true;
  TRACKING::pp_extended_readout_time = 3000;  // ns

  G4TRACKING::SC_CALIBMODE = false;

  // readout initialization
  const auto [runnumber,segment] = Fun4AllUtils::GetRunSegment(inputFile);
  std::cout<< "Fun4All_CombinedDataReconstruction - runnumber: " << runnumber << std::endl;
  std::cout<< "Fun4All_CombinedDataReconstruction - segment: " << segment << std::endl;
  TpcReadoutInit( runnumber );

  // Ar/CF4
  G4TPC::tpc_drift_velocity_reco = 0.00815238095238;

  // Ar/iC4H10/CF4
  // G4TPC::tpc_drift_velocity_reco = 0.00714;

  // reco const
  auto rc = recoConsts::instance();
  rc->set_IntFlag("RUNNUMBER", runnumber);
  rc->set_IntFlag("RUNSEGMENT", segment);

  // condition database
  Enable::CDB = true;
  rc->set_StringFlag("CDB_GLOBALTAG", "ProdA_2024");
  rc->set_uint64Flag("TIMESTAMP", runnumber);
  // rc->set_uint64Flag("TIMESTAMP", 6);

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

  // distortion correction
  if(true)
  {
    // module edge corrections
    G4TPC::ENABLE_MODULE_EDGE_CORRECTIONS = true;

    // static distortions
    G4TPC::ENABLE_STATIC_CORRECTIONS = true;
    G4TPC::USE_PHI_AS_RAD_STATIC_CORRECTIONS = false;

//     // average distortions
//     G4TPC::ENABLE_AVERAGE_CORRECTIONS = true;
//     G4TPC::USE_PHI_AS_RAD_AVERAGE_CORRECTIONS = false;
  }

  // tpc zero suppression
  // TRACKING::tpc_zero_supp = true;

  G4MAGNET::magfield_rescale = 1;
  TrackingInit();

  // input manager
  auto in = new Fun4AllDstInputManager("DSTin");
  in->fileopen(inputFile);
  se->registerInputManager(in);

  // hit unpackers
  Mvtx_HitUnpacking();
  Intt_HitUnpacking();
  Tpc_HitUnpacking();

  // Micromegas_HitUnpacking();
  {
    auto tpotunpacker = new MicromegasCombinedDataDecoder;
    const auto calibrationFile = CDBInterface::instance()->getUrl("TPOT_Pedestal");
    tpotunpacker->set_calibration_file(calibrationFile);
    // tpotunpacker->set_hot_channel_map_file("Calibrations/TPOT_HotChannels-00041989-0000.root" );
    se->registerSubsystem(tpotunpacker);
  }

  Mvtx_Clustering();
  Intt_Clustering();

  auto tpcclusterizer = new TpcClusterizer;
  tpcclusterizer->Verbosity(0);
  tpcclusterizer->set_do_hit_association(G4TPC::DO_HIT_ASSOCIATION);
  tpcclusterizer->set_rawdata_reco();
  se->registerSubsystem(tpcclusterizer);

  Tpc_LaserEventIdentifying();

  Micromegas_Clustering();

  {
    // silicon seeding
    auto silicon_Seeding = new PHActsSiliconSeeding;
    silicon_Seeding->Verbosity(0);
    silicon_Seeding->setinttRPhiSearchWindow(1.0);
    silicon_Seeding->setinttZSearchWindow(7.0);
    silicon_Seeding->seedAnalysis(false);
    se->registerSubsystem(silicon_Seeding);
  }

  {
    auto merger = new PHSiliconSeedMerger;
    merger->Verbosity(0);
    se->registerSubsystem(merger);
  }


  double fieldstrength = std::numeric_limits<double>::quiet_NaN();
  const bool ConstField = isConstantField(G4MAGNET::magfield_tracking, fieldstrength);

  {
    // TPC seeding
    auto seeder = new PHCASeeding("PHCASeeding");
    if (ConstField)
    {
      seeder->useConstBField(true);
      seeder->constBField(fieldstrength);
    }
    else
    {
      seeder->set_field_dir(-1 * G4MAGNET::magfield_rescale);
      seeder->useConstBField(false);
      seeder->magFieldFile(G4MAGNET::magfield_tracking);  // to get charge sign right
    }
    seeder->Verbosity(0);
    seeder->SetLayerRange(7, 55);
    seeder->SetSearchWindow(2.,0.05); // z-width and phi-width, default in macro at 1.5 and 0.05
    seeder->SetClusAdd_delta_window(3.0,0.06); //  (0.5, 0.005) are default; sdzdr_cutoff, d2/dr2(phi)_cutoff
    seeder->SetMinHitsPerCluster(0);
    seeder->SetMinClustersPerTrack(3);
    seeder->useFixedClusterError(true);
    seeder->set_pp_mode(TRACKING::pp_mode);
    se->registerSubsystem(seeder);
  }

  {
    // expand stubs in the TPC using simple kalman filter
    auto cprop = new PHSimpleKFProp("PHSimpleKFProp");
    cprop->set_field_dir(G4MAGNET::magfield_rescale);
    if (ConstField)
    {
      cprop->useConstBField(true);
      cprop->setConstBField(fieldstrength);
    }
    else
    {
      cprop->magFieldFile(G4MAGNET::magfield_tracking);
      cprop->set_field_dir(-1 * G4MAGNET::magfield_rescale);
    }
    cprop->useFixedClusterError(true);
    cprop->set_max_window(5.);
    cprop->Verbosity(0);
    cprop->set_pp_mode(TRACKING::pp_mode);
    se->registerSubsystem(cprop);

    if (TRACKING::pp_mode)
    {
      // for pp mode, apply preliminary distortion corrections to TPC clusters before crossing is known
      // and refit the trackseeds. Replace KFProp fits with the new fit parameters in the TPC seeds.
      auto prelim_distcorr = new PrelimDistortionCorrection;
      prelim_distcorr->set_pp_mode(TRACKING::pp_mode);
      prelim_distcorr->Verbosity(0);
      se->registerSubsystem(prelim_distcorr);
    }

  }

  {
    // matching to silicons
    auto silicon_match = new PHSiliconTpcTrackMatching;
    silicon_match->Verbosity(0);
    silicon_match->set_x_search_window(2.);
    silicon_match->set_y_search_window(2.);
    silicon_match->set_z_search_window(5.);
    silicon_match->set_phi_search_window(0.2);
    silicon_match->set_eta_search_window(0.1);
    silicon_match->set_pp_mode(TRACKING::pp_mode);
    se->registerSubsystem(silicon_match);
  }

  if( true )
  {
    // matching with micromegas
    auto mm_match = new PHMicromegasTpcTrackMatching;
    mm_match->Verbosity(0);
    mm_match->set_rphi_search_window_lyr1(0.4);
    mm_match->set_rphi_search_window_lyr2(13.0);
    mm_match->set_z_search_window_lyr1(26.0);
    mm_match->set_z_search_window_lyr2(0.4);

    mm_match->set_min_tpc_layer(38);             // layer in TPC to start projection fit
    mm_match->set_test_windows_printout(false);  // used for tuning search windows only
    se->registerSubsystem(mm_match);
  }

  {
    // track fit
    se->registerSubsystem(new PHTpcDeltaZCorrection);

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

  {
    auto cleaner = new PHTrackCleaner();
    cleaner->Verbosity(0);
    se->registerSubsystem(cleaner);
  }

  {
    // vertex finding
    auto finder = new PHSimpleVertexFinder;
    finder->Verbosity(0);
    finder->setDcaCut(0.5);
    finder->setTrackPtCut(-99999.);
    finder->setBeamLineCut(1);
    finder->setTrackQualityCut(100);
    finder->setNmvtxRequired(3);
    finder->setOutlierPairCut(1);

    se->registerSubsystem(finder);
  }

  if (G4TRACKING::SC_CALIBMODE)
  {
    /*
    * in calibration mode, calculate residuals between TPC and fitted tracks,
    * store in dedicated structure for distortion correction
    */
    auto residuals = new PHTpcResiduals;
    residuals->setOutputfile("PhTpcResiduals.root");
    residuals->setUseMicromegas(G4TRACKING::SC_USE_MICROMEGAS);

    // matches Tony's analysis
    residuals->setMinPt( 0.2 );

    // reconstructed distortion grid size (phi, r, z)
    residuals->setGridDimensions(36, 48, 80);
    se->registerSubsystem(residuals);
  }

  // residual tree from tony
  if( true )
  {
    auto resid = new TrackResiduals("TrackResiduals");
    resid->outfileName(residualFile);
    resid->Verbosity(0);
    resid->alignment(false);

    // resid->set_rejectLaserEvent(false);

    // adjust track map made
    if( G4TRACKING::SC_CALIBMODE )
    { resid->trackmapName("SvtxSiliconMMTrackMap"); }

    // discard all tracks that do not have micromegas hits
    resid->set_doMicromegasOnly(true);

    // resid->clusterTree();
    // resid->hitTree();
    // resid->convertSeeds(G4TRACKING::convert_seeds_to_svtxtracks);
    // resid->linefitAll();
    se->registerSubsystem(resid);
  }

  // official TPOT evaluator
  if( false )
  {
    auto micromegasCombinedDataEvaluation = new MicromegasCombinedDataEvaluation;
    micromegasCombinedDataEvaluation->set_evaluation_outputfile( evaluationFile );
    se->registerSubsystem(micromegasCombinedDataEvaluation);
  }

  // own evaluator
  if( true )
  {
    auto trackingEvaluator = new TrackingEvaluator_hp;
    trackingEvaluator->set_flags(
      TrackingEvaluator_hp::EvalEvent
      |TrackingEvaluator_hp::EvalTracks
      |TrackingEvaluator_hp::MicromegasOnly
      );

    if( G4TRACKING::SC_CALIBMODE )
    { trackingEvaluator->set_trackmapname( "SvtxSiliconMMTrackMap" ); }

    se->registerSubsystem(trackingEvaluator);
  }

  // own evaluator
  if( true )
  {

    auto micromegasTrackEvaluator = new MicromegasTrackEvaluator_hp;

    if( G4TRACKING::SC_CALIBMODE )
    { micromegasTrackEvaluator->set_trackmapname( "SvtxSiliconMMTrackMap" ); }

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
