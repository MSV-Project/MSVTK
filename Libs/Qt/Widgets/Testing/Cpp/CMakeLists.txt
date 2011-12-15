set(KIT QtWidgets)

set(KIT_TEST_SRCS
  msvQTimePlayerWidgetTest.cxx
  msvQTimePlayerWidgetTestPlayback.cxx
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

macro(SIMPLE_QTEST TESTNAME)
  add_test(NAME ${TESTNAME} COMMAND $<TARGET_FILE:${KIT}CxxTests>
    ${TESTNAME}
    ${ARGN})
endmacro()

macro(SIMPLE_TEST TESTNAME)
  add_test(NAME ${TESTNAME} COMMAND $<TARGET_FILE:${KIT}CxxTests>
    ${TESTNAME}
    ${ARGN}
    -D "${PROJECT_SOURCE_DIR}/Testing/Data/")
endmacro()

#
# Add Tests
#

SIMPLE_QTEST( msvQTimePlayerWidgetTest )
SIMPLE_TEST( msvQTimePlayerWidgetTestPlayback )
