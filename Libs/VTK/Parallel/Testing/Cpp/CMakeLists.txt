set(KIT VTKParallel)

set(KIT_TEST_SRCS
  msvVTKFileSeriesReaderTest1.cxx
  msvVTKDataFileSeriesReaderTest1.cxx
  )

create_test_sourcelist(Tests msv${KIT}CxxTests.cxx
  ${KIT_TEST_SRCS}
  )

set(LIBRARY_NAME msv${KIT})

add_executable(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests ${LIBRARY_NAME})

#
# Add Tests
#
simple_test_with_data( msvVTKFileSeriesReaderTest1 )
simple_test_with_data( msvVTKDataFileSeriesReaderTest1 )
