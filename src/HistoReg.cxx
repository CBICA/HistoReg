#include "cbicaUtilities.h"


#include "opencv2/opencv.hpp"
#include "opencv2/xfeatures2d.hpp"

int main(int argc, char **argv)
{
  if (argc != 4)
  {
    std::cerr << "HistoReg Usage:\n  HistoReg.exe C:/path/to/sourceImage.jpg C:/path/to/targetImage.jpg C:/path/to/outputDir \n";
    return EXIT_FAILURE;
  }

  const std::string sourceImageFile = argv[1];
  const std::string targetImageFile = argv[2];
  const std::string outputDir = argv[3];

  if (!cbica::isDir(outputDir))
  {
    cbica::createDir(outputDir);
  }
  
  auto sourceImage = cv::imread(sourceImageFile);
  auto targetImage = cv::imread(targetImageFile);

  /// https://docs.opencv.org/3.4.3/d5/d51/group__features2d__main.html
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

  /// https://docs.opencv.org/3.4.3/d8/d9b/group__features2d__match.html
  cv::BFMatcher matcher;
  //cv::FlannBasedMatcher matcher;
  matcher.match(descriptors_1, descriptors_2, matches);

  //-- Quick calculation of max and min distances between keypoints
  //double max_dist = 0; double min_dist = 100;
  //for (int i = 0; i < descriptors_1.rows; i++)
  //{
  //  double dist = matches[i].distance;
  //  if (dist < min_dist) 
  //    min_dist = dist;
  //  if (dist > max_dist) 
  //    max_dist = dist;
  //}

  /// https://docs.opencv.org/3.4.3/d9/d0c/group__calib3d.html#ga4abc2ece9fab9398f2e560d53c8c9780
  auto h = cv::findHomography(keypoints_1, keypoints_2, cv::RANSAC); // change method of homography to 0, LMEDS or RHO (documentation in above link)
  cv::imwrite(outputDir + "/homography.jpg", h);

  auto sourceImage_reg = sourceImage;

  /// https://docs.opencv.org/3.4.3/da/d54/group__imgproc__transform.html#gaf73673a7e8e18ec6963e3774e6a94b87
  cv::warpPerspective(sourceImage, sourceImage_reg, h, targetImage.size());

  cv::imwrite(outputDir + "/sourceImage_reg.jpg", sourceImage_reg);
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