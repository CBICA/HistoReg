# parent CMakeLists file

CMAKE_MINIMUM_REQUIRED(VERSION 3.7.2)
#CMAKE_POLICY(VERSION 2.8)
#IF(COMMAND cmake_policy)
#  CMAKE_POLICY(SET CMP0003 NEW)
#  CMAKE_POLICY(SET CMP0020 NEW)
#  CMAKE_POLICY(SET CMP0010 NEW)
CMAKE_POLICY(SET CMP0048 NEW) # only with CMake 3
#ENDIF(COMMAND cmake_policy)
SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED YES)
SET( CMAKE_COLOR_MAKEFILE ON )

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake_modules)

#INCLUDE(FindSubversion)
#IF(Subversion_FOUND) 
#  IF(EXISTS "${CMAKE_SOURCE_DIR}/../../.svn")
#    Subversion_WC_INFO("${CMAKE_SOURCE_DIR}/../../" MY)
#    SET(SVN_REVISION "${MY_WC_REVISION}")
#  ELSEIF(EXISTS "${CMAKE_SOURCE_DIR}/../.svn")
#    Subversion_WC_INFO("${CMAKE_SOURCE_DIR}/../" MY)
#    SET(SVN_REVISION "${MY_WC_REVISION}")
#  ELSEIF(EXISTS "${CMAKE_SOURCE_DIR}/.svn")
#    Subversion_WC_INFO("${CMAKE_SOURCE_DIR}/" MY)
#    SET(SVN_REVISION "${MY_WC_REVISION}")
#  ELSE ()
#    SET(SVN_REVISION "-1")
#  ENDIF()
#ELSE(Subversion_FOUND)
#  SET(SVN_REVISION "-1")
#ENDIF(Subversion_FOUND) 

# set project name. The exe name is taken from this
PROJECT( HistoReg )
SET (PROJECT_VERSION_MAJOR 1 )
SET (PROJECT_VERSION_MINOR 0 )
SET (PROJECT_VERSION_PATCH 0 )
#SET (SVN_REVISION "${SVN_REVISION}")

FIND_PACKAGE(OpenCV)
IF(NOT EXISTS ${OpenCV_DIR})

  IF( NOT EXISTS ${PROJECT_BINARY_DIR}/OpenCV-build/OpenCVConfig.cmake )

    OPTION( HISTOREG_SUPERBUILD "Do a Superbuild of the HistoReg dependencies or Specify ITK install directory with the defined components" ON )
    IF( HISTOREG_SUPERBUILD )
      PROJECT(${PROJECT_NAME}_Depends)
      INCLUDE( ${PROJECT_SOURCE_DIR}/cmake_modules/Superbuild.cmake )
      return()
    ENDIF()

  ELSE()
    SET( $ENV{CMAKE_PREFIX_PATH} "$ENV{CMAKE_PREFIX_PATH};${PROJECT_BINARY_DIR}/OpenCV-build" )
  ENDIF()  
  
ENDIF()
    
IF (APPLE)
  SET (EXE_NAME "${PROJECT_NAME}_${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
ELSE()
  SET( EXE_NAME "${PROJECT_NAME}" )
ENDIF()

SET( PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
#IF( NOT( ${SVN_REVISION} STREQUAL "" ) )
#  SET( PROJECT_VERSION "${PROJECT_VERSION}_${SVN_REVISION}")
#ENDIF()
SET( PROJECT_VENDOR "CBICA - UPenn" )
SET( LICENSE_FILE "${PROJECT_SOURCE_DIR}/License" )
SET( README_FILE  "${PROJECT_SOURCE_DIR}/README.txt" )
SET( PROJECT_CONTACT "software@cbica.upenn.edu" )
set(IDENTIFIER "")
ADD_DEFINITIONS(-DPROJECT_NAME="${PROJECT_NAME}" )
ADD_DEFINITIONS(-DPROJECT_VERSION="${PROJECT_VERSION}" )
ADD_DEFINITIONS(-DEXE_NAME="${EXE_NAME}")
ADD_DEFINITIONS(-DPROJECT_NAME_EXTENDED="${PROJECT_NAME_EXTENDED}")
ADD_DEFINITIONS(-DCONTACT="${PROJECT_CONTACT}")
ADD_DEFINITIONS(-DPROJECT_CONTACT="${PROJECT_CONTACT}")
ADD_DEFINITIONS(-DPROJECT_DOMAIN="${PROJECT_DOMAIN}")
ADD_DEFINITIONS(-DPROJECT_LICENSE="${LICENSE_FILE}")
ADD_DEFINITIONS(-DLICENSE="${LICENSE_FILE}")
ADD_DEFINITIONS(-DPROJECT_README="${README_FILE}")
ADD_DEFINITIONS(-DREADME="${README_FILE}")
ADD_DEFINITIONS(-DPROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}/")
ADD_DEFINITIONS(-DPROJECT_VARIANT="${${PROJECT_NAME}_Variant}")

### Building the documentation
## The following 4 lines are for generation of public documentation
SET( BUILD_DOCUMENTATION OFF CACHE BOOL "Do you want to build Documentation (needs Doxygen)?" )
MARK_AS_ADVANCED(BUILD_DOCUMENTATION)
IF( BUILD_DOCUMENTATION )
  ADD_SUBDIRECTORY( docs )
ENDIF ()

## [ITK] dependency start
#FIND_PACKAGE( ITK REQUIRED ) #[ITK] dependency
#INCLUDE(${ITK_USE_FILE})

# [ITK] dependency end
FIND_PACKAGE(OpenCV REQUIRED )
include_directories(${OpenCV_INCLUDE_DIRS})

SET( THIRDPARTY_LIB_PATH ${PROJECT_SOURCE_DIR}/src/thirdparty )
INCLUDE_DIRECTORIES( ${THIRDPARTY_LIB_PATH} )


# macro to find all sub-directories
MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children
    RELATIVE ${curdir} ${curdir}/*
    PATTERN "svn" EXCLUDE
  )
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
      LIST(APPEND dirlist ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()

### third party library processing
# find and include all third party libraries
SUBDIRLIST(SUBDIRECTORIES ${THIRDPARTY_LIB_PATH})

FOREACH(thirdParty_include ${SUBDIRECTORIES})
  INCLUDE_DIRECTORIES( ${THIRDPARTY_LIB_PATH}/${thirdParty_include} )
  IF( EXISTS "${THIRDPARTY_LIB_PATH}/${thirdParty_include}/include" )
    INCLUDE_DIRECTORIES( ${THIRDPARTY_LIB_PATH}/${thirdParty_include}/include )
  ENDIF()
ENDFOREACH()

SET_PROPERTY( GLOBAL PROPERTY USE_FOLDERS ON )

# for YAML-CPP
FILE(GLOB_RECURSE YAMLCPP_Headers "${PROJECT_SOURCE_DIR}/CaPTk/src/thirdparty/yaml-cpp/include/yaml-cpp/*.h")
FILE(GLOB_RECURSE YAMLCPP_Sources "${PROJECT_SOURCE_DIR}/CaPTk/src/thirdparty/yaml-cpp/src/*.cpp")
SET( YAMLCPP_Headers ${YAMLCPP_Headers} CACHE STRING "YAML-CPP headers" FORCE )
SET( YAMLCPP_Sources ${YAMLCPP_Sources} CACHE STRING "YAML-CPP sources" FORCE )
SOURCE_GROUP( YAMLCPP_Headers FILES ${YAMLCPP_Headers} )
SOURCE_GROUP( YAMLCPP_Sources FILES ${YAMLCPP_Sources} )

FIND_PACKAGE(OpenMP REQUIRED)
SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}" )
SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}" )

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED YES) 

FILE( GLOB sources "${PROJECT_SOURCE_DIR}/src/depends/*.*" )
FILE( GLOB cbicatk "${PROJECT_SOURCE_DIR}/src/cbica_toolkit/*.*" )

INCLUDE_DIRECTORIES( 
  ${PROJECT_SOURCE_DIR}/src/depends/ 
  ${PROJECT_SOURCE_DIR}/CaPTk/src/cbica_toolkit/src/
  ${PROJECT_SOURCE_DIR}/CaPTk/src/thirdparty/yaml-cpp/include/
)

# Add sources to executable
ADD_EXECUTABLE(
  ${PROJECT_NAME} 
  ${PROJECT_SOURCE_DIR}/src/HistoReg.cxx
  ${PROJECT_SOURCE_DIR}/src/HistoReg.h
  ${PROJECT_SOURCE_DIR}/src/HistoReg.cpp
  ${sources}
  #${cbicatk}
  #${YAMLCPP_Sources}
)

#ADD_SUBDIRECTORY( CaPTk/src/cbica_toolkit/ )

SET( LIBNAME_CBICATK "CaPTk_CBICATK" )
ADD_DEPENDENCIES( ${PROJECT_NAME} ${LIBNAME_CBICATK} )

ADD_LIBRARY( 
  ${LIBNAME_CBICATK} 
  ${PROJECT_SOURCE_DIR}/CaPTk/src/cbica_toolkit/src/cbicaStatistics.h
  ${PROJECT_SOURCE_DIR}/CaPTk/src/cbica_toolkit/src/cbicaUtilities.h
  ${PROJECT_SOURCE_DIR}/CaPTk/src/cbica_toolkit/src/cbicaUtilities.cpp
  ${PROJECT_SOURCE_DIR}/CaPTk/src/cbica_toolkit/src/cbicaCmdParser.h
  ${PROJECT_SOURCE_DIR}/CaPTk/src/cbica_toolkit/src/cbicaCmdParser.cpp
  ${YAMLCPP_Headers}
  ${YAMLCPP_Sources}
)
  

# Link the libraries to be used
TARGET_LINK_LIBRARIES(
  ${PROJECT_NAME}
  ${LIBNAME_CBICATK}
  #${ITK_LIBRARIES}
  ${OpenCV_LIBRARIES}
)

#--------------------------------------------------------------------------------
# Source file specification
#--------------------------------------------------------------------------------

INSTALL( TARGETS ${PROJECT_NAME} 
  #DESTINATION bin  
  BUNDLE DESTINATION .
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  CONFIGURATIONS "${CMAKE_CONFIGURATION_TYPES}"
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)
