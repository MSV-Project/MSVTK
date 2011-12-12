project(msvVTKLibs)

# --------------------------------------------------------------------------
#                      ----] W A R N I N G [----
# --------------------------------------------------------------------------
# Order of project directory matters. Project should be topologically ordered
# --------------------------------------------------------------------------
set(dirs
# FileWatcher
  )

list(APPEND dirs
  Parallel
  Widgets
  )

#-----------------------------------------------------------------------------
# Loop over list of directories
#-----------------------------------------------------------------------------
foreach(dir ${dirs})
  message(STATUS "Configuring library: msvVTK${dir}")
  add_subdirectory(${dir})
endforeach()
