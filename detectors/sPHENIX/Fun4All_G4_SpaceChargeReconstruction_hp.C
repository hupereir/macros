#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllDstInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <g4eval/TrackingEvaluator_hp.h>
#include <g4main/PHG4VertexSelection.h>
#include <phool/recoConsts.h>
#include <tpccalib/TpcSpaceChargeReconstruction.h>
#include <tpccalib/TpcSpaceChargeMatrixInversion.h>


#include <TSystemDirectory.h>

// own modules
#include <g4eval/EventCounter_hp.h>

#include "G4_Tracking.C"

R__LOAD_LIBRARY(libfun4all.so)

//________________________________________________________________________________________________
std::vector<TString> GetFiles( const char* pathname )
{
  std::set<TString> filenames;

  static constexpr int max_files = 1;
  
  TSystemDirectory dir( pathname, pathname );
  auto files = dir.GetListOfFiles();
  TIter next( files );
  while( auto file = static_cast<TSystemFile*>(next()) )
  {
    // get filename
    const TString filename( file->GetName() );
    if( !filename.EndsWith( ".root" ) ) continue;
    filenames.insert( TString( pathname ) + filename );

    // stop as soon as enough files have been found
    if( max_files > 0 && filenames.size() == max_files ) break;
  }

  for( const auto& filename: filenames )
  { std::cout << "GetFiles - adding " << filename << std::endl; }

  // copy to vector and return
  std::vector<TString> out;
  std::copy( filenames.begin(), filenames.end(), std::back_inserter( out ) );
  return out;
}

//________________________________________________________________________________________________
int Fun4All_G4_SpaceChargeReconstruction_hp(
  const int nEvents = 0,
  // const char* inputDirectory = "DST/CONDOR_realistic_micromegas/dst_reco_truth_notpc_distortions/",
  const char* inputDirectory = "DST/CONDOR_realistic_micromegas/dst_reco_truth_notpc_distortions-coarse-test/",
  const char* outputFile = "Rootfiles/Distortions_full_realistic_micromegas_mm_extrapolated-test-new.root"
  )
{

  // print inputs
  std::cout << "Fun4All_G4_SpaceChargeReconstruction_hp - nEvents: " << nEvents << std::endl;
  std::cout << "Fun4All_G4_SpaceChargeReconstruction_hp - inputDirectory: " << inputDirectory << std::endl;
  std::cout << "Fun4All_G4_SpaceChargeReconstruction_hp - outputFile: " << outputFile << std::endl;

  // temporary matrix filename
  const std::string& tmpfile = "TpcSpaceChargeMatrices.root";
  
  // server
  auto se = Fun4AllServer::instance();
  se->Verbosity();

  // reco const
  auto rc = recoConsts::instance();

  // event counter
  se->registerSubsystem( new EventCounter_hp( "EventCounter_hp", 100 ) );

  // space charge reconstruction
  auto spaceChargeReconstruction = new TpcSpaceChargeReconstruction;
  spaceChargeReconstruction->set_double_param( "spacecharge_max_talpha", 0.6 );
  spaceChargeReconstruction->set_double_param( "spacecharge_max_drphi", 0.5 );
  spaceChargeReconstruction->set_double_param( "spacecharge_max_tbeta", 1.5 );
  spaceChargeReconstruction->set_double_param( "spacecharge_max_dz", 0.5 );

  spaceChargeReconstruction->set_use_micromegas( true );
  spaceChargeReconstruction->set_outputfile( tmpfile );
  spaceChargeReconstruction->Verbosity(1);
  se->registerSubsystem( spaceChargeReconstruction );

  // input manager
  auto in = new Fun4AllDstInputManager("DSTin");
  auto files = GetFiles( inputDirectory );
  std::cout << "Fun4All_G4_SpaceChargeReconstruction_hp - file count: " << files.size() << std::endl;
  for( const auto filename:files ) { in->AddFile( filename.Data() ); }

  se->registerInputManager(in);

  // process events
  se->run(nEvents);

  // terminate
  se->End();
  se->PrintTimer();
  
  // perform matrix inversion
  auto spaceChargeMatrixInversion = new TpcSpaceChargeMatrixInversion;
  spaceChargeMatrixInversion->set_outputfile( outputFile );
  spaceChargeMatrixInversion->Verbosity(1);
  if( spaceChargeMatrixInversion->add_from_file( tmpfile ) )
  { spaceChargeMatrixInversion->calculate_distortions(); }
  
  std::cout << "All done" << std::endl;
  delete se;
  gSystem->Exit(0);
  return 0;
}

// This function is only used to test if we can load this as root6 macro
// without running into unresolved libraries and include files
void RunFFALoadTest() {}
