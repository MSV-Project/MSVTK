set(KIT VTKButtons)
set(KIT_TEST_SRCS
  vtkButtonsTest1.cxx
  msvQVTKButtonsMainWindowTest1.cxx
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

msv_test(buttonsTest1
  $<TARGET_FILE:${KIT}CxxTests> vtkButtonsTest1
    --vtkbuttons $<TARGET_FILE_DIR:${KIT}CxxTests>
)

simple_test( msvQVTKButtonsMainWindowTest1 )
