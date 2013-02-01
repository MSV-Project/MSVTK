set(KIT VTKGraphics)

set(KIT_TEST_SRCS
  msvVTKEmbeddedProbeFilterTest1.cxx
  )

create_test_sourcelist(Tests msv${KIT}CxxTests.cxx
  ${KIT_TEST_SRCS}
  )

set(LIBRARY_NAME msv${KIT})

add_executable(msv${KIT}CxxTests ${Tests})
target_link_libraries(msv${KIT}CxxTests ${LIBRARY_NAME})

#
# Add Tests
#
simple_test_with_data( msvVTKEmbeddedProbeFilterTest1 )
