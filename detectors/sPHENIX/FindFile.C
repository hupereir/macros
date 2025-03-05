#include <frog/FROG.h>

#include <string>

R__LOAD_LIBRARY(libFROG.so)

void FindFile()
{
  FROG frog;
  const std::string short_filename = "pedestal-54256-01982.root";
  // const std::string short_filename = "DST_TRKR_HIT_sHijing_0_20fm_50kHz_bkg_0_20fm-0000000007-00000.root";
  // const std::string short_filename = "DST_TRKR_HIT_sHijing_0_20fm-0000000007-00000.root";
  const auto long_filename = frog.location( short_filename );
  std::cout << "short_filename: " << short_filename << std::endl;
  std::cout << "long_filename: " << long_filename << std::endl;
  auto f = TFile::Open( long_filename );
  auto tree = static_cast<TTree*>( f->Get("T") );
  std::cout << "entries: " << tree->GetEntries() << std::endl;
}
