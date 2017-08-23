
project(advision)

cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0020 NEW)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

set( CMAKE_CXX_STANDARD 14 )

if ( OpenCV_FOUND )
  find_package( OpenCV 3 REQUIRED core imgproc video highgui features2d )
  add_definitions( "-DOPENCV=1" )
  include_directories ( ${OpenCV_INCLUDE_DIRS} )
endif()

if ( ArrayFire_FOUND )
  add_definitions( "-DARRAYFIRE=1" )
  include_directories ( ${ArrayFire_INCLUDE_DIRS} )
endif()

if ( CUDA_FOUND )
  list( APPEND CUDA_NVCC_FLAGS "-std=c++11;-O2;-DVERBOSE;-Wno-deprecated-gpu-targets")
  set ( CUDA_PROPAGATE_HOST_FLAGS OFF )
  include_directories (
    ${CUDA_TOOLKIT_ROOT_DIR}/samples/common/inc
    ${CUDA_INCLUDE_DIRS}
  )
endif()

find_package( Qt5 CONFIG REQUIRED Core Widgets ) #Gui )

include_directories(
  ${Qt5Widgets_INCLUDE_DIRS}
  )
		     
add_definitions(-DADVISION_LIBRARY)

if ( MSVC )
  add_definitions(-D_SCL_SECURE_NO_WARNINGS -wd4996 )
endif()

set ( SOURCES
  advision_global.hpp
  aftypes.hpp
  applycolormap.cpp
  applycolormap.hpp
  bgr2rgb.cpp
  bgr2rgb.hpp
  cvtypes.hpp  
  plot.cpp
  plot.hpp
  transform.cpp
  transform.hpp
  )

set ( CUDA_SOURCES
  afcolormap.hpp
  afcolormap.cu
  cvcolormap.hpp
  cvcolormap.cu
  )

if ( CUDA_FOUND )
  cuda_add_library( ${PROJECT_NAME} SHARED
    ${SOURCES}
    ${CUDA_SOURCES}
    )
  set ( LINK_KEYWORD ${CUDA_LINK_LIBRARIES_KEYWORD} )
else()
  add_library( ${PROJECT_NAME} SHARED
    ${SOURCES}
    )
  set ( LINK_KEYWORD "LINK_PUBLIC" )
endif()

target_link_libraries( ${PROJECT_NAME} ${LINK_KEYWORD}
  adportable
  ${ArrayFire_LIBRARIES}
  ${ArrayFire_CUDA_LIBRARIES}  
  ${Boost_LIBRARIES}
  ${CUDA_LIBRARIES}
  ${OpenCV_LIBRARIES}
  Qt5::Core
  Qt5::Widgets
  )

if( APPLE )
  install( TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin COMPONENT applications
    LIBRARY DESTINATION Contents/PlugIns COMPONENT libraries
    ARCHIVE DESTINATION Contents/PlugIns COMPONENT libraries )
else()
  install( TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin COMPONENT applications
    LIBRARY DESTINATION lib/qtplatz COMPONENT runtime_libraries
    ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
endif()
