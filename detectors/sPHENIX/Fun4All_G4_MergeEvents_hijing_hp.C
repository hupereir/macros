#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <g4main/Fun4AllDstPileupInputManager.h>
#include <g4main/PHG4VertexSelection.h>
#include <phool/recoConsts.h>

// own modules
#include <g4eval/EventCounter_hp.h>

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libg4eval.so)
R__LOAD_LIBRARY(libg4testbench.so)

//________________________________________________________________________________________________
int Fun4All_G4_MergeEvents_hijing_hp(
const int nEvents = 0,
const int eventOffset = 0,
const char* inputFile = "/sphenix/sim/sim01/sphnxpro/Micromegas/2/G4Hits_sHijing_0-12fm_000000_001000.root",
const char* outputFile = "DST/dst_g4hits_merged.root"
)
{

  std::cout << "Fun4All_G4_MergeEvents_hijing_hp - new" << std::endl;
  std::cout << "Fun4All_G4_MergeEvents_hijing_hp - nEvents: " << nEvents << std::endl;
  std::cout << "Fun4All_G4_MergeEvents_hijing_hp - eventOffset: " << eventOffset << std::endl;
  std::cout << "Fun4All_G4_MergeEvents_hijing_hp - inputFile: " << inputFile << std::endl;
  std::cout << "Fun4All_G4_MergeEvents_hijing_hp - outputFile: " << outputFile << std::endl;

  // server
  auto se = Fun4AllServer::instance();
  se->Verbosity(1);

  auto rc = recoConsts::instance();
  rc->set_IntFlag("RANDOMSEED", 1);

  // event counter
  se->registerSubsystem(new EventCounter_hp( "EventCounter_hp", 1 ));

  // input manager
  auto in = new Fun4AllDstPileupInputManager("DSTin");
  in->registerSubsystem( new PHG4VertexSelection );
  in->setEventOffset(eventOffset);

  // load timestamps
  const std::string filename( "timestamps_50kHz.txt" );
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
  in->setBunchCrossingList(bunchcrossings);
  // set max trigger rate to 20kHz.
  /* This results in a 15kHz effective rate when convoluted with the verex distribution */
  /* this should give about 300 full events per 1000 single hijing events, at 50kHz */
  in->setMaxTriggerRate( 20e3 );

  // open file
  in->fileopen(inputFile);
  se->registerInputManager(in);

  // output manager
  /* all the nodes from DST and RUN are saved to the output */
  Fun4AllDstOutputManager *out = new Fun4AllDstOutputManager("DSTOUT", outputFile);
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
