set(KIT FFS)
set(KIT_TEST_SRCS
  msvFFSTest1.cxx
  msvFFSTest2.cxx
  )

create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  ${KIT_TEST_SRCS}
  )

set(LIBRARY_NAME msv${KIT})

add_executable(${KIT}CxxTests ${KIT}CxxTests.cxx ${KIT_TEST_SRCS})
set_target_properties(${KIT}CxxTests PROPERTIES COMPILE_DEFINITIONS NDIM=3)
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
  NAME ffsTest1
  COMMAND $<TARGET_FILE:${KIT}CxxTests> ffsTest1
  --ffs $<TARGET_FILE_DIR:${KIT}CxxTests>
)

SIMPLE_TEST( msvQFFSMainWindowTest1 )
