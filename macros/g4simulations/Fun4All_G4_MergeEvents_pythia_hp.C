#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllDstInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <phool/recoConsts.h>

#include <g4main/Fun4AllDstPileupInputManager.h>
#include <g4main/PHG4VertexSelection.h>

// own modules
#include <g4eval/EventCounter_hp.h>
#include <g4eval/SimEvaluator_hp.h>

R__ADD_INCLUDE_PATH( $SPHENIX/src/macros/macros/g4simulations )
#include "G4Setup_sPHENIX.C"
#include "G4_Bbc.C"

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libg4testbench.so)

//________________________________________________________________________________________________
int Fun4All_G4_MergeEvents_pythia_hp(
const int nEvents = 15,
const int eventOffset = 0,
const char* inputFile = "DST/dst_sim_hepmc.root", 
const char* outputFile = "DST/dst_sim_hepmc_merged.root"
)
{

  std::cout << "Fun4All_G4_MergeEvents_hijing_hp - nEvents: " << nEvents << std::endl;
  std::cout << "Fun4All_G4_MergeEvents_hijing_hp - eventOffset: " << eventOffset << std::endl;
  std::cout << "Fun4All_G4_MergeEvents_hijing_hp - inputFile: " << inputFile << std::endl;
  std::cout << "Fun4All_G4_MergeEvents_hijing_hp - outputFile: " << outputFile << std::endl;

  // server
  auto se = Fun4AllServer::instance();
  se->Verbosity(1);

  auto rc = recoConsts::instance();
  rc->set_IntFlag("RANDOMSEED", 1);

  // subsysreco
  se->registerSubsystem(new EventCounter_hp( "EventCounter_hp",1));
  se->registerSubsystem(new SimEvaluator_hp);

  // input manager
  auto in = new Fun4AllDstPileupInputManager("DSTin");
  in->registerSubsystem( new PHG4VertexSelection() );
  in->setEventOffset(eventOffset);
  
  // load timestamps
  const std::string filename( "timestamps.txt" );
  std::vector<int64_t> bunchcrossings;
  std::ifstream ifstream( filename );
  std::string line;
  while( std::getline( ifstream, line, '\n' ) )
  { 
    std::istringstream linein( line );
    int64_t bunchcrossing;
    double timestamp;    
    linein >> bunchcrossing >> timestamp;
    if( linein ) 
    { bunchcrossings.push_back( bunchcrossing ); }
  }
  
  std::cout << "Fun4All_G4_MergeEvents_hijing_hp - loaded " << bunchcrossings.size() << " bunchcrossings." << std::endl;
  in->setBunchCrossingList( bunchcrossings );
  
  // open file
  in->fileopen(inputFile);
  se->registerInputManager(in);

  // output manager
  /* all the nodes from DST and RUN are saved to the output */
  auto out = new Fun4AllDstOutputManager("DSTOUT", outputFile);
  se->registerOutputManager(out);

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
