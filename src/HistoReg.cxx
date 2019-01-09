#include "cbicaUtilities.h"


#include "opencv2/opencv.hpp"
#include "opencv2/xfeatures2d.hpp"
//#include "opencv2/optflow.hpp"

//! A helper struct to store some variables
struct TranslationUnit
{
  float x; //! translation along x
  float y; //! translation along y
  float absoluteDistance; //! absolute distance between x and y - does NOT account for direction
  void Update()
  {
    absoluteDistance = std::sqrtf(std::pow(x, 2) + std::pow(y, 2));
  }
};

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
  std::vector< cv::KeyPoint > keypoints_source, keypoints_target;
  f2d->detect(sourceImage, keypoints_source);
  f2d->detect(targetImage, keypoints_target);

  // Calculate descriptors (feature vectors)    
  cv::Mat descriptors_1, descriptors_2;
  f2d->compute(sourceImage, keypoints_source, descriptors_1);
  f2d->compute(targetImage, keypoints_target, descriptors_2);

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

  auto robust_matches = matches;
  robust_matches.clear();
  for (size_t i = 0; i < matches.size(); i++)
  {
    if (std::abs(matches[i].distance) < 0.6) // lowe's ratio to be changed
    {
      robust_matches.push_back(matches[i]);
    }
  }

  /// accessing points in robust_matches and making that into a translation vector
  std::vector< TranslationUnit > translationVector;
  for (size_t i = 0; i < robust_matches.size(); i++)
  {
    TranslationUnit temp;
    temp.x = keypoints_target[robust_matches[i].imgIdx].pt.x - keypoints_source[robust_matches[i].imgIdx].pt.x;
    temp.y = keypoints_target[robust_matches[i].imgIdx].pt.y - keypoints_source[robust_matches[i].imgIdx].pt.y;
    temp.Update();
    translationVector.push_back(temp);
  }

  //robust_points_source[0].pt.x
  ///// https://docs.opencv.org/3.4.3/d9/d0c/group__calib3d.html#ga4abc2ece9fab9398f2e560d53c8c9780
  //auto h = cv::findHomography(keypoints_source, keypoints_target, cv::RANSAC,); // change method of homography to 0, LMEDS or RHO (documentation in above link)
  //cv::imwrite(outputDir + "/homography.jpg", h);

  //auto sourceImage_reg = sourceImage;

  ///// https://docs.opencv.org/3.4.3/da/d54/group__imgproc__transform.html#gaf73673a7e8e18ec6963e3774e6a94b87
  //cv::warpPerspective(sourceImage, sourceImage_reg, h, targetImage.size());

  //cv::imwrite(outputDir + "/sourceImage_reg.jpg", sourceImage_reg);
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