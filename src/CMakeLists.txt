INCLUDE(${CMAKE_SOURCE_DIR}/IJMacros.txt)

#Change PROJECT_NAME to the name of your project
SET(TOOL_NAME "PRIDE tool name" CACHE STRING "Specify the name of your PRIDE tool")
PROJECT(${TOOL_NAME})

#Declare any external dependencies that your project may have here.
#examples include: ITK, VTK, JPEG, PNG, OpenGL, ZLIB, Perl, Java
#If you're not sure what name to use, look in the Modules directory of your
#cmake install and check that a file named Find(Package).cmake exists
SET(Required_Packages
ITK
)

#this foreach loads all of the packages that you specified as required.
#It shouldn't need to be modified.
FOREACH(Package ${Required_Packages})
  LOADPACKAGE(${Package})
ENDFOREACH(Package)

#Set any libraries that your project depends on.
#examples: ITKCommon, VTKRendering, etc
 SET(Libraries
ITKIO
 ITKCommon
 ITKIOXML
)

#the following block of code is an example of how to build an executable in
#cmake.  Unmodified, it will add an executable called "MyExe" to the project.
#MyExe will be built using the files MyClass.h and MyClass.cxx, and it will
#be linked to all the libraries you specified above. 
#You can build more than one executable per project

#Add a definition for making programs single threaded (as reqired for PRIDE tools
  ADD_DEFINITIONS(-DNO_MULTI_THREADING)

SET(CurrentExe "${TOOL_NAME}")
SET(CurrentSrc MainCxxFile.cxx)

#SET(EXECUTABLE_OUTPUT_PATH "G:/Patch/pride/${TOOL_NAME}")
cmake_minimum_required(VERSION 3.13)
SET(CMAKE_CONFIGURATION_TYPES Release CACHE STRING "" FORCE)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:G:/Patch/pride/${TOOL_NAME}>)

ADD_EXECUTABLE(${CurrentExe} ${CurrentSrc}) 
#TARGET_LINK_LIBRARIES(${CurrentExe} ${Libraries})
TARGET_LINK_LIBRARIES(${CurrentExe} ${ITK_LIBRARIES})

