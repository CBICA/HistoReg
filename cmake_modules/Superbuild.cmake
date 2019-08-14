## The Superbuild script is used to automatically download and build project dependencies. 

# Using GIT to download third party libraries. An SVN/BitBucket URL will also work the same way
FIND_PACKAGE( Git REQUIRED )

OPTION( USE_GIT_PROTOCOL "If behind a firewall turn this off to use https instead." OFF )

SET(git_protocol "git")
IF(NOT USE_GIT_PROTOCOL)
	SET(git_protocol "https")
ENDIF()

SET( 
  CMAKE_MODULE_PATH
  ${PROJECT_SOURCE_DIR}/cmake_modules
  ${CMAKE_MODULE_PATH}
)

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED YES) 

IF(MSVC)
  SET( CMAKE_BUILD_TYPE "Debug;Release")
ELSEIF(UNIX)
  SET( CMAKE_BUILD_TYPE "Release")
ENDIF()

INCLUDE( ExternalProject )

## Compute -G arg for configuring external projects with the same CMake generator:
#IF(CMAKE_EXTRA_GENERATOR)
#	SET(gen "${CMAKE_EXTRA_GENERATOR} - ${CMAKE_GENERATOR}")
#ELSE()
#	SET(gen "${CMAKE_GENERATOR}" )
#ENDIF()

#MESSAGE( STATUS "Adding DCMTK-3.6.3 ...")
#INCLUDE( ${PROJECT_SOURCE_DIR}/cmake_modules/External-DCMTK.cmake )

#MESSAGE( STATUS "Adding YAML-CPP 0.6.2 ...")
#INCLUDE( ${PROJECT_SOURCE_DIR}/cmake_modules/External-yaml-cpp.cmake )

#MESSAGE( STATUS "Adding EIGEN-3.3.4 ...")
#INCLUDE( ${PROJECT_SOURCE_DIR}/cmake_modules/External-Eigen.cmake )

#MESSAGE( STATUS "Adding OpenCV-3.4.1 ...")
#INCLUDE( ${PROJECT_SOURCE_DIR}/cmake_modules/External-OpenCV.cmake )

MESSAGE( STATUS "Adding ITK-4.13.2 ...")
INCLUDE( ${PROJECT_SOURCE_DIR}/cmake_modules/External-ITK.cmake )