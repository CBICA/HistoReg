#include "cbicaUtilities.h"

int main(int argc, char **argv)
{
  if (argc < 3)
  {
    std::cerr << "HistoReg Usage:\n  HistoReg.exe -d $pathToDataDir -o $pathToOutputDir \n";
    return EXIT_FAILURE;
  }
  std::string dataDir = argv[1];
  std::string outputDir = argv[2];
  return EXIT_SUCCESS;
}