set(KIT QtWidgets)

set(KIT_TEST_SRCS
  msvQTimePlayerWidgetTest.cxx
  )

include_directories(${CMAKE_CURRENT_BINARY_DIR})

create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  ${KIT_TEST_SRCS}
  )

set(LIBRARY_NAME msv${KIT})

QT4_GENERATE_MOCS(
  msvQTimePlayerWidgetTest.cxx
  )

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

SIMPLE_TEST( msvQTimePlayerWidgetTest )
