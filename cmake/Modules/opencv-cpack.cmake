
# find_package( OpenCV 3 REQUIRED core imgproc video highgui features2d )

foreach( lib ${OpenCV_LIBRARIES} )
  get_target_property( loc ${lib} IMPORTED_LOCATION_RELEASE )
  # message( STATUS "## opencv install: " ${lib} " --> " ${loc} )
  install ( PROGRAMS ${loc} DESTINATION ${QTPLATZ_QT5_RUNTIME_INSTALL_DIRECTORY} COMPONENT runtime_libraries )
endforeach()

