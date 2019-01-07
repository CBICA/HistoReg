#MESSAGE( "External project - OpenCV" )

SET( OpenCV_DEPENDENCIES )

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED YES) 

ExternalProject_Add( 
  OpenCV_Contrib
  DEPENDS Eigen
  URL https://github.com/opencv/opencv_contrib/archive/3.4.1.zip
  SOURCE_DIR OpenCV_Contrib-source
  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""  
)

ExternalProject_Add( 
  OpenCV
  DEPENDS Eigen OpenCV_Contrib
  URL https://github.com/opencv/opencv/archive/3.4.1.zip
  #GIT_REPOSITORY ${git_protocol}://github.com/opencv/opencv.git
  #GIT_TAG 3.4.1
  SOURCE_DIR OpenCV-source
  BINARY_DIR OpenCV-build
  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  INSTALL_COMMAND ""
  #BUILD_COMMAND ""
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
    ${ep_common_args}
    #-DCMAKE_CONFIGURATION_TYPES=${CMAKE_CONFIGURATION_TYPES}
    -DENABLE_CXX11:BOOL=ON 
    -DBUILD_EXAMPLES:BOOL=OFF # examples are not needed
    -DBUILD_opencv_apps:BOOL=OFF 
    -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS} # no static builds
    -DBUILD_WITH_STATIC_CRT:BOOL=OFF
    -DBUILD_TESTS:BOOL=OFF 
    -DBUILD_PERF_TESTS:BOOL=OFF 
    -DBUILD_opencv_python2:BOOL=OFF
    -DBUILD_opencv_python3:BOOL=OFF
    -DBUILD_opencv_python_bindings_generator:BOOL=OFF
    -DWITH_CUDA:BOOL=ON
    -DBUILD_DOCS:BOOL=OFF
    -DWITH_OPENCL_SVM:BOOL=ON
    -DOPENCV_ENABLE_NONFREE:BOOL=ON
    #-DWITH_QT:BOOL=TRUE # [QT] dependency, enables better GUI
    -DWITH_EIGEN:BOOL=TRUE # [Eigen] dependency, enables better matrix operations 
    -DWITH_OPENMP:BOOL=ON
    -DWITH_OPENGL:BOOL=ON
    -DBUILD_JPEG:BOOL=ON
    -DWITH_JPEG:BOOL=ON
    #-DWITH_VTK:BOOL=ON
    -DBUILD_JAVA:BOOL=OFF 
    -DEIGEN_INCLUDE_PATH:STRING=${CMAKE_BINARY_DIR}/Eigen-source
    -DOPENCV_EXTRA_MODULES_PATH:STRING=${CMAKE_BINARY_DIR}/OpenCV_Contrib-source/modules
    #-DVTK_DIR:STRING=${VTK_DIR}
    -DOpenCV_USE_GUISUPPORT:BOOL=FALSE
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}/install
)

SET( OpenCV_DIR ${CMAKE_BINARY_DIR}/OpenCV-build )
#LIST(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/OpenCV-build")
SET( ENV{CMAKE_PREFIX_PATH} "${CMAKE_PREFIX_PATH};${CMAKE_BINARY_DIR}/OpenCV-build" )