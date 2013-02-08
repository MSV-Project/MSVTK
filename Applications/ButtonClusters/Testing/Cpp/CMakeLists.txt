set(KIT ButtonClusters)
set(KIT_TEST_SRCS
  buttonclustersTest1.cxx
  msvQButtonClustersMainWindowTest1.cxx
  )

create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  ${KIT_TEST_SRCS}
  )

set(LIBRARY_NAME msv${KIT})

add_executable(msv${KIT}CxxTests ${Tests})
target_link_libraries(msv${KIT}CxxTests ${LIBRARY_NAME})

#
# Add Tests
#
add_test(
  NAME buttonclustersTest1
  COMMAND $<TARGET_FILE:msv${KIT}CxxTests> buttonclustersTest1
  --buttonclusters $<TARGET_FILE_DIR:msv${KIT}CxxTests>
)

SIMPLE_TEST( msvQButtonClustersMainWindowTest1 )
