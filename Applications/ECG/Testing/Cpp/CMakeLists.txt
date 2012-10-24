set(KIT ECG)
set(KIT_TEST_SRCS
  ecgTest1.cxx
  msvQECGMainWindowTest1.cxx
  msvVTKECGButtonsManagerTest1.cxx
  )

create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  ${KIT_TEST_SRCS}
  )

set(LIBRARY_NAME msv${KIT})

add_executable(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests ${LIBRARY_NAME})

#
# Add Tests
#

msv_test(ecgTest1
  $<TARGET_FILE:${KIT}CxxTests> ecgTest1
    --ecg $<TARGET_FILE_DIR:${KIT}CxxTests>
  )
simple_test( msvQECGMainWindowTest1 )
simple_test( msvVTKECGButtonsManagerTest1 )
