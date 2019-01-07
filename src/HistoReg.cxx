#include "cbicaUtilities.h"

int main(int argc, char **argv)
{
  if (argc < 5)
  {
    std::cerr << "HistoReg Usage:\n  HistoReg.exe -d $pathToDataDir -o $pathToOutputDir \n";
    return EXIT_FAILURE;
  }
  std::string dataDir = argv[2];
  std::string outputDir = argv[4];
  return EXIT_SUCCESS;
}