#include "cbicaUtilities.h"

int main(int argc, char **argv)
{
  if (argc != 5)
  {
    std::cerr << "HistoReg Usage:\n  HistoReg.exe -d C:/path/to/dataDir -o C:/path/to/outputDir \n";
    return EXIT_FAILURE;
  }
  const std::string dataDir = argv[2];
  const std::string outputDir = argv[4];

  if (!cbica::isDir(outputDir))
  {
    cbica::createDir(outputDir);
  }

  return EXIT_SUCCESS;
}