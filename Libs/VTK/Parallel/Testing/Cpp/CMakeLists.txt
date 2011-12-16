set(KIT VTKParallel)

set(KIT_TEST_SRCS
  msvVTKFileSeriesReaderTest1.cxx
  msvVTKPolyDataFileSeriesReaderTest1.cxx
  )

create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  ${KIT_TEST_SRCS}
  )

set(LIBRARY_NAME msv${KIT})

add_executable(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests ${LIBRARY_NAME})

macro(SIMPLE_TEST TESTNAME)
  add_test(NAME ${TESTNAME} COMMAND $<TARGET_FILE:${KIT}CxxTests>
    ${TESTNAME}
    ${ARGN}
    -D "${PROJECT_SOURCE_DIR}/Testing/Data/")
endmacro()

#
# Add Tests
#
SIMPLE_TEST( msvVTKFileSeriesReaderTest1 )
SIMPLE_TEST( msvVTKPolyDataFileSeriesReaderTest1 )
