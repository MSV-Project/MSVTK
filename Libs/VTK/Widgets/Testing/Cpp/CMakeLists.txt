set(KIT VTKWidgets)

set(KIT_TEST_SRCS
  msvVTKProp3DButtonRepresentationTest1.cxx
  msvVTKWidgetClustersTest1.cxx
  )

create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  ${KIT_TEST_SRCS}
  )

#set(TestsToRun ${Tests})
#list(REMOVE_ITEM TestsToRun ${KIT}CxxTests.cxx)

set(LIBRARY_NAME msv${KIT})

add_executable(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests ${LIBRARY_NAME})

#
# Add Tests
#
SIMPLE_TEST( msvVTKProp3DButtonRepresentationTest1 )
SIMPLE_TEST( msvVTKButtonManagerTest1 )

