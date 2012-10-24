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

#
# Add Tests
#

simple_test( msvQTimePlayerWidgetTest )
simple_test_with_data( msvQTimePlayerWidgetTestPlayback )
