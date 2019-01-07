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

  // get all the sub-directories in the folder
  auto foldersInDir = cbica::subdirectoriesInDirectory(dataDir);
  
  for (size_t i = 0; i < foldersInDir.size(); i++)
  {
    // get all the files in the sub-directory
    auto filesInSubDir = cbica::filesInDirectory(foldersInDir[i]);

    // process each file in the sud-directory
    for (size_t j = 0; j < filesInSubDir.size(); j++)
    {
      std::string path, base, ext;
      cbica::splitFileName(filesInSubDir[j], path, base, ext);
      if (ext == ".png")
      {
        // do something with the image
      }
      else if (ext == ".csv")
      {
        // do something with the landmark
      }
    }
  }

  return EXIT_SUCCESS;
}