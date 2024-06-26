
project( adcv )

cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0020 NEW)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

if ( OpenCV_FOUND )
  include_directories ( ${OpenCV_INCLUDE_DIRS} )
  set ( cuda_cv_sources
    cvcolormap.hpp
    cvcolormap.cu
    )
endif()

if ( ArrayFire_FOUND )
  add_definitions( "-DARRAYFIRE=1" )
  include_directories ( ${ArrayFire_INCLUDE_DIRS} )
  set ( cuda_af_sources
    afcolormap.hpp
    afcolormap.cu
    )
endif()

if ( CUDA_FOUND )
  #list( APPEND CUDA_NVCC_FLAGS "-std=c++11;-O2;-DVERBOSE;-Wno-deprecated-gpu-targets")
  list( APPEND CUDA_NVCC_FLAGS "-std=c++17;-O2;-DVERBOSE")
  list( APPEND CUDA_NVCC_FLAGS "-arch=sm_37")
  list( APPEND CUDA_NVCC_FLAGS "--default-stream per-thread")
  set ( CUDA_PROPAGATE_HOST_FLAGS OFF )
  include_directories (
    ${CUDA_TOOLKIT_ROOT_DIR}/samples/common/inc
    ${CUDA_INCLUDE_DIRS}
  )
endif()

#find_package( Qt${QT_VERSION_MAJOR} CONFIG REQUIRED Core Widgets OpenGL ) #Gui )
find_package( qwt )

include_directories(
  ${QWT_INCLUDE_DIRS}
  ${QtWidgets_INCLUDE_DIRS}
  "${Qt${QT_VERSION_MAJOR}OpenGL_INCLUDE_DIRS}"
  )

add_definitions(-DADCV_LIBRARY)

if ( MSVC )
  add_definitions(-D_SCL_SECURE_NO_WARNINGS -wd4996 )
endif()

set ( SOURCES
  adcv_global.hpp
  aftypes.hpp
  applycolormap.cpp
  applycolormap.hpp
  bgr2rgb.cpp
  bgr2rgb.hpp
  cvtypes.hpp
  deviceinfo.cpp
  deviceinfo.hpp
  imagewidget.cpp
  imagewidget.hpp
  imfilter.cpp
  imfilter.hpp
  moments.cpp
  moments.hpp
  utils.hpp
  plot.cpp
  plot.hpp
  spectrogramplot.cpp
  spectrogramplot.hpp
  transform.cpp
  transform.hpp
  dft2d.cpp
  dft2d.hpp
#  graphicsview.cpp
#  graphicsview.hpp
  )

if ( WIN32 )
  set_source_files_properties( applycolormap.cpp imfilter.cpp PROPERTIES COMPILE_FLAGS -wd5037 )
  set_source_files_properties( applycolormap.cpp imfilter.cpp PROPERTIES COMPILE_FLAGS -wd5037 )
endif()

set ( CUDA_SOURCES
  ${cuda_af_sources}
  ${cuda_cv_sources}
  colormap.hpp
  colormap.cu
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
  set ( CUDA_LIBRARIES )
endif()

target_link_libraries( ${PROJECT_NAME} ${LINK_KEYWORD}
  adportable
  adcontrols
  adplot
  ${QWT_LIBRARIES}
  ${ArrayFire_LIBRARIES}
  ${ArrayFire_CUDA_LIBRARIES}
  ${Boost_LIBRARIES}
  ${CUDA_LIBRARIES}
  ${OpenCV_LIBRARIES}
  Qt::Core
  Qt::Widgets
  )

runtime_install_path( _runtime_destination ${PROJECT_NAME} )
library_install_path( _library_destination ${PROJECT_NAME} )
archive_install_path( _archive_destination ${PROJECT_NAME} )

if( APPLE )
  install( TARGETS ${PROJECT_NAME}
    EXPORT ${QTPLATZ_TARGETS}
    RUNTIME DESTINATION ${_runtime_destination} COMPONENT applications
    LIBRARY DESTINATION Contents/PlugIns COMPONENT libraries
    ARCHIVE DESTINATION Contents/PlugIns COMPONENT libraries )
else()
  install( TARGETS ${PROJECT_NAME}
    EXPORT ${QTPLATZ_TARGETS}
    RUNTIME DESTINATION ${_runtime_destination} COMPONENT applications
    LIBRARY DESTINATION ${_library_destination} COMPONENT runtime_libraries
    ARCHIVE DESTINATION ${_archive_destination} COMPONENT libraries
    )
endif()
