set(KIT VTKFiltering)

set(KIT_TEST_SRCS
  msvVTKPolyDataBoundaryEdgeCapsTest1.cxx
  )

create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  ${KIT_TEST_SRCS}
  )

set(LIBRARY_NAME msv${KIT})

add_executable(msv${KIT}CxxTests ${Tests})
target_link_libraries(msv${KIT}CxxTests ${LIBRARY_NAME})

# macro(SIMPLE_TEST TESTNAME)
#   add_test(NAME ${TESTNAME} COMMAND $<TARGET_FILE:${KIT}CxxTests>
#     ${TESTNAME}
#     ${ARGN}
#     -D "${PROJECT_SOURCE_DIR}/Testing/Data/")
# endmacro()

#
# Add Tests
#
simple_test( msvVTKPolyDataBoundaryEdgeCapsTest1 )

