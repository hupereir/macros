#include <frog/FROG.h>

#include <string>

void FindFile()
{
  FROG frog;
  const std::string short_filename = "DST_TRUTH_G4HIT_sHijing_0_20fm_50kHz_bkg_0_20fm-0000000001-00000.root";
  const auto long_filename = frog.location( short_filename );
  std::cout << "short_filename: " << short_filename << std::endl;
  std::cout << "long_filename: " << long_filename << std::endl;
}
