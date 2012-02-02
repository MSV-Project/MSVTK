set(KIT VTKParallel)

set(KIT_TEST_SRCS
  msvVTKFileSeriesReaderTest1.cxx
  msvVTKPolyDataFileSeriesReaderTest1.cxx
  msvVTKXMLMultiblockLODReaderTest1.cxx
#  msvVTKCompositeFileSeriesReaderTest1.cxx
  )

create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  ${KIT_TEST_SRCS}
  )

set(LIBRARY_NAME msv${KIT})

add_executable(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests ${LIBRARY_NAME})

#
# Add Tests
#
simple_test_with_data( msvVTKFileSeriesReaderTest1 )
simple_test_with_data( msvVTKPolyDataFileSeriesReaderTest1 )
simple_test_with_data( msvVTKXMLMultiblockLODReaderTest1 )
#simple_test_with_data( msvVTKCompositeFileSeriesReaderTest1 )
