set(KIT ECG)
set(KIT_TEST_SRCS
  ecgTest1.cxx
  msvQECGMainWindowTest1.cxx
  msvVTKECGCartoPointsReaderTest1.cxx
  )

create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  ${KIT_TEST_SRCS}
  )

#set(TestsToRun ${Tests})
#list(REMOVE_ITEM TestsToRun ${KIT}CxxTests.cxx)

add_definitions( -DDATA_TESTING_PATH="${PROJECT_SOURCE_DIR}/Testing/Data/" )

set(LIBRARY_NAME msv${KIT})

add_executable(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests ${LIBRARY_NAME})

macro(SIMPLE_TEST TESTNAME)
  add_test(NAME ${TESTNAME} COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${KIT}CxxTests>
    ${TESTNAME}
    ${ARGN}
    -D "${PROJECT_SOURCE_DIR}/Testing/Data/")
endmacro()

#
# Add Tests
#

SIMPLE_TEST( ecgTest1 )
SIMPLE_TEST( msvQECGMainWindowTest1 )
SIMPLE_TEST( msvVTKECGCartoPointsReaderTest1 )
