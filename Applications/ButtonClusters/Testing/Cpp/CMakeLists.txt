set(KIT ButtonClusters)
set(KIT_TEST_SRCS
  buttonclustersTest1.cxx
  msvQButtonClustersMainWindowTest1.cxx
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
    ${ARGN})
endmacro()

#
# Add Tests
#

add_test(
  NAME buttonclustersTest1
  COMMAND $<TARGET_FILE:${KIT}CxxTests> buttonclustersTest1
  --buttonclusters $<TARGET_FILE_DIR:${KIT}CxxTests>
)

SIMPLE_TEST( msvQButtonClustersMainWindowTest1 )
SIMPLE_TEST( msvVTKButtonClustersTest1 )
