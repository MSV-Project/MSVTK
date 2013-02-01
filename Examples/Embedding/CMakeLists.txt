project(msvEmbeddingExamples)
set(KIT EmbeddingExamples)

set(include_dirs
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${MSVTK_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}
  ${CTK_INCLUDE_DIRS}
  ${msvVTKGraphics_INCLUDE_DIRS}
  ${msvVTKParallel_INCLUDE_DIRS}
  ${msvVTKWidgets_INCLUDE_DIRS}
  ${msvQtWidgets_INCLUDE_DIRS}
  )

include_directories(${include_dirs})

add_definitions( -DDATA_TESTING_PATH="${PROJECT_SOURCE_DIR}/../Data/Embedding/" )

# msvVTKEmbeddedProbeFilter
add_executable(EmbeddedProbeFilterExample msvVTKEmbeddedProbeFilterExample.cxx)
target_link_libraries(EmbeddedProbeFilterExample msvVTKGraphics msvVTKParallel)
