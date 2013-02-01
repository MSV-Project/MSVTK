set(KIT FFS)
set(KIT_TEST_SRCS
  msvFFSTest1.cxx
  msvFFSTest2.cxx
  )

create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  ${KIT_TEST_SRCS}
  )

set(LIBRARY_NAME msv${KIT})

add_executable(msv${KIT}CxxTests ${KIT}CxxTests.cxx ${KIT_TEST_SRCS})
set_target_properties(msv${KIT}CxxTests PROPERTIES COMPILE_DEFINITIONS NDIM=3)
target_link_libraries(msv${KIT}CxxTests ${LIBRARY_NAME})

#
# Add Tests
#

add_test(
  NAME ffsTest1
  COMMAND $<TARGET_FILE:msv${KIT}CxxTests> ffsTest1
  --ffs $<TARGET_FILE_DIR:msv${KIT}CxxTests>
)

simple_test( msvQFFSMainWindowTest1 )
