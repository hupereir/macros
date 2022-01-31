#include <frog/FROG.h>

#include <string>

void FindFile()
{
  FROG frog;
  const std::string short_filename = "G4Hits_sHijing_0_20fm-0000000003-00002.root";
  const auto long_filename = frog.location( short_filename );
  std::cout << "short_filename: " << short_filename << std::endl;
  std::cout << "long_filename: " << long_filename << std::endl;
}
