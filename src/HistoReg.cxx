#include <cmath>
#include <fstream>
#include "cbicaUtilities.h"
#include <iostream>
#include "opencv2/opencv.hpp"
#include "opencv2/xfeatures2d.hpp"
//#include "opencv2/optflow.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>

#include <numeric>
#include "cbicaITKSafeImageIO.h"

#include "itkRGBPixel.h"

struct DrawMatchesFlags
{
    enum
    {
        DEFAULT = 0, // Output image matrix will be created (Mat::create),
                     // i.e. existing memory of output image may be reused.
                     // Two source images, matches, and single keypoints
                     // will be drawn.
                     // For each keypoint, only the center point will be
                     // drawn (without a circle around the keypoint with the
                     // keypoint size and orientation).
        DRAW_OVER_OUTIMG = 1, // Output image matrix will not be
                       // created (using Mat::create). Matches will be drawn
                       // on existing content of output image.
        NOT_DRAW_SINGLE_POINTS = 2, // Single keypoints will not be drawn.
        DRAW_RICH_KEYPOINTS = 4 // For each keypoint, the circle around
                       // keypoint with keypoint size and orientation will
                       // be drawn.
    };
};

//! A helper struct to store some variables
struct TranslationUnit
{
  // NOT SURE IF IMPORTANT
  //cv::KeyPoint source;
  //cv::KeyPoint target;
  cv::Point_<float> source;
  cv::Point_<float> target;

  float x; //! translation along x
  float y; //! translation along y
  float absoluteDistance; //! absolute distance between x and y - does NOT account for direction
  void Update()
  {
    absoluteDistance = std::sqrt(std::pow(x, 2) + std::pow(y, 2));
  }
};

// READ ONLY LANDMARK CSV
std::vector <std::vector<float>> Read_CSV_landmark (std::string PATH_to_CSV)
{
  std::ifstream file (PATH_to_CSV);
  std::string line;
  std::vector <std::vector<float>> Tab;
  std::vector <float> LineTab;


  //skip frist line 
  getline(file,line);


  while (file.good())
  {
    getline(file,line);
    //std::cout << line << '\n';
    

    // Only consider the two coordinates x,y, so between the 1st and the 2nd coma and the 2nd coma and the end of the line 
    int posComa1 = line.find(',',0);
    int posComa2 = line.find(',',posComa1+1);
    
    std::string xlandmarkStr = line.substr(posComa1+1, posComa2-(posComa1+1));
    std::string ylandmarkStr = line.substr(posComa2+1, line.size()-(posComa2+1));


    // if coordinates exist, convert them into int and add them to the Tab matrix.
    if ( xlandmarkStr.length() !=0 && ylandmarkStr.length() !=0 )
    {
      int xlandmark = std::stoi(xlandmarkStr);
      int ylandmark = std::stoi(ylandmarkStr);

      //Add the coordinates to a 2,1 matrix and then add this matrix to the matrix containing all the different coordinates
      LineTab.push_back(xlandmark);
      LineTab.push_back(ylandmark);

      Tab.push_back(LineTab);

      // clear LineTab
      LineTab.pop_back();
      LineTab.pop_back();
    }

  }

  return Tab;

}

// READ ONLY 3*3 and 4*4 Matrix
std::vector <std::vector<float>> Read_MAT_files (std::string PATH_to_MAT, int Dim)
{
  std::vector <std::vector<float>> Mat;
  std::vector<float> LineMat;
  std::string line;
  std::ifstream myfile (PATH_to_MAT);
  if (myfile.is_open())
  {
    while ( getline(myfile,line))
      {
        if ( Dim == 3)
        {
          int posSpace1 = line.find(' ',0);
          int posSpace2 = line.find(' ',posSpace1+1);
          
          std::string astr = line.substr(0, posSpace1);
          std::string bstr = line.substr(posSpace1+1, posSpace2-(posSpace1+1));
          std::string cstr = line.substr(posSpace2+1, line.size()-(posSpace2+1));

          float a = std::stof(astr);
          float b = std::stof(bstr);
          float c = std::stof(cstr);

          LineMat.push_back(a);
          LineMat.push_back(b);
          LineMat.push_back(c);

          Mat.push_back(LineMat);

          LineMat.pop_back();
          LineMat.pop_back();
          LineMat.pop_back();
        }
        // useless ?? no need to change coordinates to NIFTI's ones ?
        if ( Dim == 4)
        {
          int posSpace1 = line.find(' ',0);
          int posSpace2 = line.find(' ',posSpace1+1);
          int posSpace3 = line.find(' ',posSpace2+1);
          
          std::string astr = line.substr(0, posSpace1);
          std::string bstr = line.substr(posSpace1+1, posSpace2-(posSpace1+1));
          std::string cstr = line.substr(posSpace2+1, posSpace3-(posSpace2+1));
          std::string dstr = line.substr(posSpace3+1, line.size()-(posSpace3+1));

          float a = std::stof(astr);
          float b = std::stof(bstr);
          float c = std::stof(cstr);
          float d = std::stof(dstr);

          LineMat.push_back(a);
          LineMat.push_back(b);
          LineMat.push_back(c);
          LineMat.push_back(d);

          Mat.push_back(LineMat);

          LineMat.pop_back();
          LineMat.pop_back();
          LineMat.pop_back();
          LineMat.pop_back();

        }
      }
    myfile.close();
  }
  return Mat;
}
/*
std::vector<TranslationUnit> Transformation(std::vector <std::vector<float>> Landmarks, std::vector< TranslationUnit > translationVector, float radius)
{
  std::vector<std::vector<float>> NeighborsInsideCircle;
  std::vector<float> LineNeighborInsideCircle;
  std::vector<TranslationUnit> translationVectorLandmarks;
  float xl; float yl; float xm; float ym;

  // For each landmark
  for (int i = 0; i < Landmarks.size(); ++i)
  {
    // Get landmark coordinate
    xl = Landmarks[i][0];
    yl = Landmarks[i][1];

    // For each duo keypoints from robust_match (each element of translationVector)
    for (int j = 0; j < translationVector.size(); ++j)
    {
      // Get the point in the source image
      auto point = translationVector[j].source;
      float xm = point.x;
      float ym = point.y;

      // compute euclidean distance with landmark
      float xdiff = xl - xm;
      float ydiff = yl - ym;
      float dist = std::sqrt(std::pow(xdiff,2) + std::pow(ydiff,2));
      
      // keep in memory the 3 closest keypoint
      if (dist < radius)
      {
        LineNeighborInsideCircle.push_back(dist);
        LineNeighborInsideCircle.push_back(translationVector[j].x);
        LineNeighborInsideCircle.push_back(translationVector[j].y);

        NeighborsInsideCircle.push_back(LineNeighborInsideCircle);

        LineNeighborInsideCircle.pop_back();
        LineNeighborInsideCircle.pop_back();
        LineNeighborInsideCircle.pop_back();
      }
    }

    // compute the total of distances for average
    float totalDistance = ClosestNeighbor[0][0] + ClosestNeighbor[1][0] + ClosestNeighbor[2][0];

    // Not sure about this way to do weighted average !! NOT GOOD !!\
    // USE INVERSE DISTANCE WEIGHTING
    //float xtranslation_mean = (totalDistance - ClosestNeighbor[0][0] / totalDistance ) * ClosestNeighbor[0][1] 
                              + (totalDistance - ClosestNeighbor[1][0] / totalDistance ) * ClosestNeighbor[1][1] 
                              + (totalDistance - ClosestNeighbor[2][0] / totalDistance ) * ClosestNeighbor[2][1];
    //float ytranslation_mean = (totalDistance - ClosestNeighbor[0][0] / totalDistance ) * ClosestNeighbor[0][2] 
                              + (totalDistance - ClosestNeighbor[1][0] / totalDistance ) * ClosestNeighbor[1][2] 
                              + (totalDistance - ClosestNeighbor[2][0] / totalDistance ) * ClosestNeighbor[2][2];
    float xtranslation_mean = 0;
    float ytranslation_mean = 0;

    // fill translationVectorLandmarks with the coordinate of the landmarks and the vector it should follow.
    TranslationUnit temp;
    cv::Point_<float> sourceXY;
    sourceXY = cv::Point_<float>(xl,yl);
    temp.source = sourceXY;

    std::cout << "Point : " << sourceXY << '\n';
    std::cout << "Vecteur : " << xtranslation_mean << "  " << ytranslation_mean << '\n';

    temp.x = xtranslation_mean;
    temp.y = ytranslation_mean;
    temp.Update();
    translationVectorLandmarks.push_back(temp);

    // Remove previous result in ClosestNeighbor because we'll use a new landmark
    ClosestNeighbor.pop_back();
    ClosestNeighbor.pop_back();
    ClosestNeighbor.pop_back();
  }

  return translationVectorLandmarks;

}

// Apply transformation field to landmarks 
std::vector<TranslationUnit> Transformation(std::vector <std::vector<float>> Landmarks, std::vector< TranslationUnit > translationVector)
{
  std::vector<std::vector<float>> ClosestNeighbor;
  std::vector<float> LineClosestNeighbor;
  LineClosestNeighbor = {1000,0,0}; // Dist, x, y
  std::vector<TranslationUnit> translationVectorLandmarks;
  float xl; float yl; float xm; float ym;

  // For each landmark
  for (int i = 0; i < Landmarks.size(); ++i)
  {
    // Initialize Closest neighbor with point 0,0 and distance 1000
    ClosestNeighbor.push_back(LineClosestNeighbor);
    ClosestNeighbor.push_back(LineClosestNeighbor);
    ClosestNeighbor.push_back(LineClosestNeighbor);

    // Get landmark coordinate
    xl = Landmarks[i][0];
    yl = Landmarks[i][1];

    // For each duo keypoints from robust_match (each element of translationVector)
    for (int j = 0; j < translationVector.size(); ++j)
    {
      // Get the point in the source image
      auto point = translationVector[j].source;
      float xm = point.x;
      float ym = point.y;

      // compute euclidean distance with landmark
      float xdiff = xl - xm;
      float ydiff = yl - ym;
      float dist = std::sqrt(std::pow(xdiff,2) + std::pow(ydiff,2));

      std::cout << "Distance " << ClosestNeighbor[0][0] << "   " << ClosestNeighbor[1][0] << "   " << ClosestNeighbor[2][0] << '\n';
      
      // keep in memory the 3 closest keypoint
      if (dist < ClosestNeighbor[0][0])
      {
        ClosestNeighbor[2][0] = ClosestNeighbor[1][0];
        ClosestNeighbor[2][1] = ClosestNeighbor[1][1];
        ClosestNeighbor[2][2] = ClosestNeighbor[1][2];

        ClosestNeighbor[1][0] = ClosestNeighbor[0][0];
        ClosestNeighbor[1][1] = ClosestNeighbor[0][1];
        ClosestNeighbor[1][2] = ClosestNeighbor[0][2];

        ClosestNeighbor[0][0] = dist;
        ClosestNeighbor[0][1] = translationVector[j].x;
        ClosestNeighbor[0][2] = translationVector[j].y;
      }
      if (dist > ClosestNeighbor[0][0] && dist < ClosestNeighbor[1][0])
      {
        ClosestNeighbor[2][0] = ClosestNeighbor[1][0];
        ClosestNeighbor[2][1] = ClosestNeighbor[1][1];
        ClosestNeighbor[2][2] = ClosestNeighbor[1][2];

        ClosestNeighbor[1][0] = dist;
        ClosestNeighbor[1][1] = translationVector[j].x;
        ClosestNeighbor[1][2] = translationVector[j].y;
      }
      if (dist > ClosestNeighbor[0][0] && dist > ClosestNeighbor[1][0] && dist < ClosestNeighbor[2][0])
      {
        ClosestNeighbor[2][0] = dist;
        ClosestNeighbor[2][1] = translationVector[j].x;
        ClosestNeighbor[2][2] = translationVector[j].y;
      }
    }

    // compute the total of distances for average
    float totalDistance = ClosestNeighbor[0][0] + ClosestNeighbor[1][0] + ClosestNeighbor[2][0];

    // Not sure about this way to do weighted average !! NOT GOOD !!\
    // USE INVERSE DISTANCE WEIGHTING
    //float xtranslation_mean = (totalDistance - ClosestNeighbor[0][0] / totalDistance ) * ClosestNeighbor[0][1] 
                              + (totalDistance - ClosestNeighbor[1][0] / totalDistance ) * ClosestNeighbor[1][1] 
                              + (totalDistance - ClosestNeighbor[2][0] / totalDistance ) * ClosestNeighbor[2][1];
    //float ytranslation_mean = (totalDistance - ClosestNeighbor[0][0] / totalDistance ) * ClosestNeighbor[0][2] 
                              + (totalDistance - ClosestNeighbor[1][0] / totalDistance ) * ClosestNeighbor[1][2] 
                              + (totalDistance - ClosestNeighbor[2][0] / totalDistance ) * ClosestNeighbor[2][2];
    float xtranslation_mean = 0;
    float ytranslation_mean = 0;

    // fill translationVectorLandmarks with the coordinate of the landmarks and the vector it should follow.
    TranslationUnit temp;
    cv::Point_<float> sourceXY;
    sourceXY = cv::Point_<float>(xl,yl);
    temp.source = sourceXY;

    std::cout << "Point : " << sourceXY << '\n';
    std::cout << "Vecteur : " << xtranslation_mean << "  " << ytranslation_mean << '\n';

    temp.x = xtranslation_mean;
    temp.y = ytranslation_mean;
    temp.Update();
    translationVectorLandmarks.push_back(temp);

    // Remove previous result in ClosestNeighbor because we'll use a new landmark
    ClosestNeighbor.pop_back();
    ClosestNeighbor.pop_back();
    ClosestNeighbor.pop_back();
  }

  return translationVectorLandmarks;

}
*/
std::vector <double> Evaluation(cv::Mat MovingImages, std::vector <std::vector<float>> MovedLandmarks, std::vector <std::vector<float>> TargetLandmark)
{
  std::vector <double> rTRE;
  int H = MovingImages.size().height;
  int W = MovingImages.size().width;


  // compute euclidean distance normalized by image diagonal
  for (int i=0; i<MovedLandmarks.size(); ++i)
  {
    double xdiff = TargetLandmark[i][0] - MovedLandmarks[i][0];
    double ydiff = TargetLandmark[i][1] - MovedLandmarks[i][1];
    double euclDist = std::sqrt(std::pow(xdiff,2) + std::pow(ydiff,2));

    // FOR NOW just euclidean distance, because it's more meaningfull
    //double euclDistNorm = euclDist/std::sqrt(std::pow(H,2)+std::pow(W,2));

    rTRE.push_back(euclDist);
  }

  // For now return euclidean distance normalized
  // !!! NOT FINAL CRITERIA OF EVALUATION !!!

  return rTRE;

  // compute median of rTRE
  /*
  sort(rTRE.begin(), rTRE.end());
  if (size % 2 == 0)
  {
    double median_rTRE = (rTRE[size / 2 - 1] + rTRE[size / 2]) / 2;
  }
  else 
  {
    double median_rTRE = rTRE[size / 2];
  }
  */

  // compute rank ????

  //sum_rTRE = std::accumulate();
  //mean = sum_rTRE/ size();

  // Score = rTRE - mean(rank(median(rTRE)))
  // !!!! NOT SURE OF MY UNDERSTANDING !!!! 
  // Speak with Spyros about the ranking score to be sure 
 
}

int main(int argc, char **argv)
{
  if (argc != 6)
  {
    std::cerr << "HistoReg Usage:\n  HistoReg.exe C:/path/to/sourceImage.jpg C:/path/to/targetImage.jpg C:/path/to/outputDir C:/path/to/landmark/source C:/path/to/landmark/target\n";
    return EXIT_FAILURE;
  }

  const std::string sourceImageFile = argv[1];
  const std::string targetImageFile = argv[2];
  const std::string outputDir = argv[3];
  const std::string sourceLandmarkCSV = argv[4];
  const std::string targetLandmarkCSV = argv[5];

  if (!cbica::isDir(outputDir))
  {
    cbica::createDir(outputDir);
  }

  auto sourceImage = cv::imread(sourceImageFile);
  auto targetImage = cv::imread(targetImageFile);
  
  /*
  //std::cout << sourceImage.size() << '\n';

  //int H = sourceImage.size().height;
  //int W = sourceImage.size().width;


  //std::vector <std::vector<float>> MovingLandmarks = Read_CSV_landmark (sourceLandmarkCSV);
  using PixelType = itk::RGBPixel < uchar>;
  typedef itk::Image< PixelType, 3 > ExpectedImageType;
  using ReaderType = itk::ImageFileReader< ExpectedImageType >;
  auto reader = ReaderType::New();
  reader->SetFileName( sourceImageFile );
  try
  {
    reader->Update();
  }
  catch (itk::ExceptionObject& e)
  {
    std::cerr << "Exception caught while reading the image '" << sourceImageFile << "':\n" << e.what() << "\n";
    return EXIT_FAILURE;
  }

  auto sourceImage_itk = reader->GetOutput();

  itk::OrientImageFilter<ImageType,ImageType>::Pointer orienter = itk::OrientImageFilter<ImageType,ImageType>::New();
  orienter->UseImageDirectionOn();
  orienter->SetDesiredCoordinateOrientation(itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RIP);
  orienter->SetInput(sourceImage_itk);
  orienter->Update();
  auto rval = orienter->GetOutput();

  imwrite("/home/venetl/Documents/HistoReg/src/OutputHomo.png",rval);


  
  itk::ImageConstIteratorWithIndex<ExpectedImageType> imageIterator2 (sourceImage_itk, sourceImage_itk->GetLargestPossibleRegion());

  std::cout << sourceImage_itk->GetLargestPossibleRegion().GetSize() << '\n';

  for (int a=0; a < 6 ; ++a)
  {
    float i=MovingLandmarks[a*10][0];
    float j=MovingLandmarks[a*10][1];

    std::cout << "Point : " << i << "  " << j << '\n';
   
    //auto valueAtIJ = sourceImage.at<int> (j,i);

    //unsigned char * valueAtIJ = sourceImage.ptr(j,i);

    //auto valueAtIJ = *p;

    //auto p = sourceImage.ptr(j,i);
    //std::cout << "Channel_0: " << valueAtIJ[0] << "\n";
    //std::cout << "Channel_1: " << valueAtIJ[1] << "\n";
    //std::cout << "Channel_2: " << valueAtIJ[2] << "\n";

    auto temp2 = sourceImage.at< cv::Vec3b >(j, i);
    std::cout << "CV : " << temp2 << "\n";

    //std::cout << valueAtIJ.size() <<'\n';
    
    imageIterator2.SetIndex(itk::Index<3> {{i,j,0}});
    auto valueAtIJ_ITK = imageIterator2.Get();

    std::cout << "ITK : " << valueAtIJ_ITK << '\n';
  }
  */
  //For point detector, uncomment from here 

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
  std::vector<std::vector< cv::DMatch>> knnmatches;

  /// https://docs.opencv.org/3.4.3/d8/d9b/group__features2d__match.html
  cv::BFMatcher matcher;
  //cv::FlannBasedMatcher matcher;
  matcher.match(descriptors_1, descriptors_2, matches);
  matcher.knnMatch(descriptors_1,descriptors_2,knnmatches,2);

  /*
  auto robust_matches = matches;
  robust_matches.clear();
  /// accessing points in robust_matches and making that into a translation vector
  std::vector< TranslationUnit > translationVector;
  for (size_t i = 0; i < matches.size(); i++)
  {

    // WRONG : 
    //if (std::abs(matches[i].distance) < 0.6) // lowe's ratio to be changed
    //{
      robust_matches.push_back(matches[i]);
      TranslationUnit temp;
      temp.target = keypoints_target[matches[i].imgIdx];
      temp.source = keypoints_source[matches[i].imgIdx];
      temp.x = keypoints_target[matches[i].imgIdx].pt.x - keypoints_source[matches[i].imgIdx].pt.x;
      temp.y = keypoints_target[matches[i].imgIdx].pt.y - keypoints_source[matches[i].imgIdx].pt.y;
      temp.Update();
      translationVector.push_back(temp);
      std::cout << keypoints_target[matches[i].imgIdx].pt.x << "    " << keypoints_target[matches[i].imgIdx].pt.y << '\n';
    //}
  }
  */
  
  std::cout << knnmatches.size() << '\n';
  std::vector< cv::DMatch > robust_matches;
  std::vector<cv::Point_<float>> pts_src;
  std::vector<cv::Point_<float>> pts_trgt;
  robust_matches.clear();
  /// accessing points in robust_matches and making that into a translation vector
  std::vector< TranslationUnit > translationVector;
  for (size_t i = 0; i < knnmatches.size(); i++)
  {
    if (std::abs(knnmatches[i][0].distance / knnmatches[i][1].distance) < 0.8) // lowe's ratio to be changed
    {
      robust_matches.push_back(knnmatches[i][0]);

      // test
      int source_idx = knnmatches[i][0].queryIdx;
      int target_idx = knnmatches[i][0].trainIdx;

      auto sourceXY = keypoints_source[source_idx].pt;
      auto targetXY = keypoints_target[target_idx].pt;

      pts_src.push_back(sourceXY);
      pts_trgt.push_back(targetXY);

      TranslationUnit temp;
      temp.source = sourceXY;
      temp.target = targetXY;
      temp.x = targetXY.x - sourceXY.x;
      temp.y = targetXY.y - sourceXY.y;
      temp.Update();
      translationVector.push_back(temp);
      
      std::cout << "Match number : " << i << '\n';
      std::cout << "Keypoint coordinates in source and target images : " << sourceXY << "    " << targetXY << '\n';
      std::cout << "Vectors : x " << temp.x << " y " << temp.y << '\n';
      //std::cout << knnmatches[i][0].distance << "    " << knnmatches[i][1].distance << '\n';
      //std::cout << keypoints_target[knnmatches[i][0].imgIdx].pt.x << "    " << keypoints_target[knnmatches[i][0].imgIdx].pt.y << '\n';
    }
  }
  std::cout << "Total robust matches : " << robust_matches.size() << '\n';
  
  // Test
  /*
  cv::Mat h = cv::findHomography(pts_src,pts_trgt);

  cv::Mat im_out;
  warpPerspective(sourceImage,im_out,h,targetImage.size());

  imwrite("/home/venetl/Documents/HistoReg/src/OutputHomo.png",im_out);
  */

  // NOT WORKING FOR NOW, DUNNO WHY

  imshow("Source : ", sourceImage);

  /*
  cv::Mat OutputImage;
  const std::vector<std::vector<char>>& matchesMask=std::vector<std::vector<char> >(); 
  drawMatches(sourceImage,keypoints_source,targetImage,keypoints_target,matches,OutputImage,cv::Scalar::all(-1),cv::Scalar::all(-1),std::vector<char>(),DrawMatchesFlags::DEFAULT);
  
  imshow("DetectedImage", OutputImage );
  cv::waitKey(0);
  */

  //std::vector <std::vector<float>> MovingLandmarks = Read_CSV_landmark (sourceLandmarkCSV);
  //std::vector<TranslationUnit> Result = Transformation(MovingLandmarks, translationVector);
  // evaluation ?
  /*
  std::cout << "Resultats " << Result.size() << '\n';
  for (int i = 0; i < Result.size(); ++i)
  {
    cv::Point_<float> Point;
    float xl; float yl;

    xl = Result[i].x;
    yl = Result[i].y;
    Point = Result[i].source;

    std::cout << "Point : " << Point << "  Vecteurs : x " << xl << "  y " << yl << '\n';



  }
  */
  // GREEDY

  //system("./MYSCRIPT.sh ");

  
  /*
  std::vector <std::vector<float>> MovingLandmarks = Read_CSV_landmark (sourceLandmarkCSV);
  std::vector <std::vector<float>> TargetLandmarks = Read_CSV_landmark (targetLandmarkCSV);

  // Evaluation before registration 

  std::vector <double> rTRE_before = Evaluation(sourceImage, MovingLandmarks, TargetLandmarks);

  // Apply Greddy registration to the landmarks
  // Read Matrix for affine registration
  
  //std::string Test = sourceImageFile.substr(31);
  //std::string Test2 = outputDir;
  //Test2.append(Test);
  //std::cout << Test2 << '\n';
  

  std::vector <std::vector<float>> MatAffine = Read_MAT_files("/home/venetl/Documents/HistoReg/Output/dataset_small/lung-lesion_1/29-041-Izd2-w35-Cc10-5-les1/Affine.mat",3);

  // Read Matrix for 
  //std::vector <std::vector<float>> XYtoNIFTI = Read_MAT_files("/home/venetl/Documents/HistoReg/Output/dataset_small/lung-lesion_1/29-041-Izd2-w35-Cc10-5-les1/XYtoNIFTI.mat",4);

  // Open NIFTI image
  using PixelType = itk::Vector< float, 2 >;
  typedef itk::Image< PixelType, 3 > ExpectedImageType;
  std::string inputFileName = "/home/venetl/Documents/HistoReg/Output/dataset_small/lung-lesion_1/29-041-Izd2-w35-Cc10-5-les1/warp.nii.gz";
  using ReaderType = itk::ImageFileReader< ExpectedImageType >;
  auto reader = ReaderType::New();
  reader->SetFileName( inputFileName );
  try
  {
    reader->Update();
  }
  catch (itk::ExceptionObject& e)
  {
    std::cerr << "Exception caught while reading the image '" << inputFileName << "':\n" << e.what() << "\n";
    return EXIT_FAILURE;
  }

  auto inputImage = reader->GetOutput();

  // display size of the image
  std::cout << "Size NIFTI Image: " << inputImage->GetLargestPossibleRegion().GetSize() << "\n";
  
  // Define iterator 
  itk::ImageConstIteratorWithIndex<ExpectedImageType> imageIterator (inputImage, inputImage->GetLargestPossibleRegion());

  // display number of landmarks
  int NumberLandmarks = MovingLandmarks.size();
  
  std::cout << "Size Matrix Landmarks: " << NumberLandmarks << " " << MovingLandmarks[0].size()<< "\n";

  // For each landmark
  for (int i=0; i < NumberLandmarks; ++i)
  {
    std::cout << "Point : " << MovingLandmarks[i][0] << "   " << MovingLandmarks[i][1] << '\n';

    float x=MovingLandmarks[i][0];
    float y=MovingLandmarks[i][1];

    // apply affine registration
    MovingLandmarks[i][0] = MatAffine[0][0]*x + MatAffine[0][1]*y + MatAffine[0][2];
    MovingLandmarks[i][1] = MatAffine[1][0]*x + MatAffine[1][1]*y + MatAffine[1][2];

    std::cout << "Point after Affine : " << MovingLandmarks[i][0] << "   " << MovingLandmarks[i][1] << '\n';

    // Finding transformation vector's coordinate in the transformation field ( from the NIFTI image)
    imageIterator.SetIndex(itk::Index<3> {{MovingLandmarks[i][0],MovingLandmarks[i][1],0}});
    auto Val = imageIterator.Get();
    std::cout << "Vecteur NIFTI : " << Val[0] << "   " << Val[1] << '\n';

    // Apply the vector
    MovingLandmarks[i][0] = MovingLandmarks[i][0] + Val[0];
    MovingLandmarks[i][1] = MovingLandmarks[i][1] + Val[1];

    std::cout << "New Point after Non-rigid : " << MovingLandmarks[i][0] << "   " << MovingLandmarks[i][1] << '\n';

    std::cout << "Target Landmarks : " << TargetLandmarks[i][0]  << "   " << TargetLandmarks[i][1] << '\n' << '\n';

  }
  
  // Evaluation after registration
  
  std::vector <double> rTRE_after = Evaluation(sourceImage, MovingLandmarks, TargetLandmarks);

  for (int i=0; i < rTRE_before.size(); ++i)
  {
    std::cout << "Before : " << rTRE_before[i] << "   After : " << rTRE_after[i] << '\n';
  }

  */
  

/*
  // TEST FOR TOYEXAMPLES //
  //std::vector <std::vector<float>> MovingLandmarks = Read_CSV_landmark ("");
  //std::vector <std::vector<float>> TargetLandmarks = Read_CSV_landmark (targetLandmarkCSV);

  // Evaluation before registration 

  std::vector <double> rTRE_before = Evaluation(sourceImage, MovingLandmarks, TargetLandmarks);



  std::vector <std::vector<float>> MatAffine = Read_MAT_files("/home/venetl/Documents/HistoReg/Output/dataset_small/lung-lesion_1/29-041-Izd2-w35-Cc10-5-les1/Affine.mat",3);

  // Read Matrix for 
  //std::vector <std::vector<float>> XYtoNIFTI = Read_MAT_files("/home/venetl/Documents/HistoReg/Output/dataset_small/lung-lesion_1/29-041-Izd2-w35-Cc10-5-les1/XYtoNIFTI.mat",4);

  // Open NIFTI image
  using PixelType = itk::Vector< float, 2 >;
  typedef itk::Image< PixelType, 3 > ExpectedImageType;
  std::string inputFileName = "/home/venetl/Documents/HistoReg/Output/dataset_small/lung-lesion_1/29-041-Izd2-w35-Cc10-5-les1/warp.nii.gz";
  using ReaderType = itk::ImageFileReader< ExpectedImageType >;
  auto reader = ReaderType::New();
  reader->SetFileName( inputFileName );
  try
  {
    reader->Update();
  }
  catch (itk::ExceptionObject& e)
  {
    std::cerr << "Exception caught while reading the image '" << inputFileName << "':\n" << e.what() << "\n";
    return EXIT_FAILURE;
  }

  auto inputImage = reader->GetOutput();

  // display size of the image
  std::cout << "Size NIFTI Image: " << inputImage->GetLargestPossibleRegion().GetSize() << "\n";
  
  // Define iterator 
  itk::ImageConstIteratorWithIndex<ExpectedImageType> imageIterator (inputImage, inputImage->GetLargestPossibleRegion());

  float x=MovingLandmarks[i][0];
  float y=MovingLandmarks[i][1];
  
  std::cout << "Point : " << MovingLandmarks[i][0] << "   " << MovingLandmarks[i][1] << '\n';



  // apply affine registration
  MovingLandmarks[i][0] = MatAffine[0][0]*x + MatAffine[0][1]*y + MatAffine[0][2];
  MovingLandmarks[i][1] = MatAffine[1][0]*x + MatAffine[1][1]*y + MatAffine[1][2];

  std::cout << "Point after Affine : " << MovingLandmarks[i][0] << "   " << MovingLandmarks[i][1] << '\n';

  // Finding transformation vector's coordinate in the transformation field ( from the NIFTI image)
  imageIterator.SetIndex(itk::Index<3> {{MovingLandmarks[i][0],MovingLandmarks[i][1],0}});
  auto Val = imageIterator.Get();
  std::cout << "Vecteur NIFTI : " << Val[0] << "   " << Val[1] << '\n';


  // Apply the vector
  MovingLandmarks[i][0] = MovingLandmarks[i][0] + Val[0];
  MovingLandmarks[i][1] = MovingLandmarks[i][1] + Val[1];

  std::cout << "New Point after Non-rigid : " << MovingLandmarks[i][0] << "   " << MovingLandmarks[i][1] << '\n';

  std::cout << "Target Landmarks : " << TargetLandmarks[i][0]  << "   " << TargetLandmarks[i][1] << '\n' << '\n';
*/
  
  
  // Evaluation after registration
  /*
  std::vector <double> rTRE_after = Evaluation(sourceImage, MovingLandmarks, TargetLandmarks);

  for (int i=0; i < rTRE_before.size(); ++i)
  {
    std::cout << "Before : " << rTRE_before[i] << "   After : " << rTRE_after[i] << '\n';
  }
  */

  /*
  // Read Data from a Matrix
  std::cout << XYtoNIFTI.size() << '\n';
  std::cout << XYtoNIFTI[0].size() << '\n';
  int j;
  for ( int i =0; i < XYtoNIFTI.size(); ++i)
  {
    for (j=0; j < XYtoNIFTI[0].size(); ++j)
    {
      std::cout << XYtoNIFTI[i][j] << "    ";
      if (j == XYtoNIFTI[0].size()-1)
      {
        std::cout << std::endl;
      }
    } 
  }
  */
  
  return EXIT_SUCCESS;
}