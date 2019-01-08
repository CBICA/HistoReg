#include "cbicaUtilities.h"


#include "opencv2/opencv.hpp"
#include "opencv2/xfeatures2d.hpp"

int main(int argc, char **argv)
{
  if (argc != 7)
  {
    std::cerr << "HistoReg Usage:\n  HistoReg.exe -s C:/path/to/sourceImage.jpg -t C:/path/to/targetImage.jpg -o C:/path/to/outputDir \n";
    return EXIT_FAILURE;
  }

  const std::string sourceImageFile = argv[2];
  const std::string targetImageFile = argv[4];
  const std::string outputDir = argv[6];

  if (!cbica::isDir(outputDir))
  {
    cbica::createDir(outputDir);
  }
  
  auto sourceImage = cv::imread(sourceImageFile);
  auto targetImage = cv::imread(targetImageFile);

  auto f2d = cv::xfeatures2d::SIFT::create();
  //auto f2d = cv::xfeatures2d::SURF::create();
  //auto f2d = cv::ORB::create();
  //auto f2d = cv::xfeatures2d::DAISY::create();

  // Detect the keypoints:
  std::vector< cv::KeyPoint > keypoints_1, keypoints_2;
  f2d->detect(sourceImage, keypoints_1);
  f2d->detect(targetImage, keypoints_2);

  // Calculate descriptors (feature vectors)    
  cv::Mat descriptors_1, descriptors_2;
  f2d->compute(sourceImage, keypoints_1, descriptors_1);
  f2d->compute(targetImage, keypoints_2, descriptors_2);

  // Matching descriptor vectors
  std::vector< cv::DMatch > matches;
  
  cv::BFMatcher matcher;
  //cv::FlannBasedMatcher matcher;
  matcher.match(descriptors_1, descriptors_2, matches);

  //-- Quick calculation of max and min distances between keypoints
  double max_dist = 0; double min_dist = 100;
  for (int i = 0; i < descriptors_1.rows; i++)
  {
    double dist = matches[i].distance;
    if (dist < min_dist) 
      min_dist = dist;
    if (dist > max_dist) 
      max_dist = dist;
  }

  //if (argc != 5)
  //{
  //  std::cerr << "HistoReg Usage:\n  HistoReg.exe -d C:/path/to/dataDir -o C:/path/to/outputDir \n";
  //  return EXIT_FAILURE;
  //}
  //const std::string dataDir = argv[2];
  //const std::string outputDir = argv[4];

  //if (!cbica::isDir(outputDir))
  //{
  //  cbica::createDir(outputDir);
  //}

  //// get all the sub-directories in the folder
  //auto foldersInDir = cbica::subdirectoriesInDirectory(dataDir);
  //
  //for (size_t i = 0; i < foldersInDir.size(); i++)
  //{
  //  // get all the files in the sub-directory
  //  auto filesInSubDir = cbica::filesInDirectory(foldersInDir[i]);

  //  // process each file in the sud-directory
  //  for (size_t j = 0; j < filesInSubDir.size(); j++)
  //  {
  //    std::string path, base, ext;
  //    cbica::splitFileName(filesInSubDir[j], path, base, ext);
  //    if (ext == ".png")
  //    {
  //      // do something with the image
  //    }
  //    else if (ext == ".csv")
  //    {
  //      // do something with the landmark
  //    }
  //  }
  //}



  return EXIT_SUCCESS;
}