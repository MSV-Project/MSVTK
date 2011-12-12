project(msvQtLibs)

# --------------------------------------------------------------------------
#                      ----] W A R N I N G [----
# --------------------------------------------------------------------------
# Order of project directory matters. Project should be topologically ordered
# --------------------------------------------------------------------------
set(dirs
# FileWatcher
  )

list(APPEND dirs
  Widgets
  )

#-----------------------------------------------------------------------------
# Loop over list of directories
#-----------------------------------------------------------------------------
foreach(dir ${dirs})
  message(STATUS "Configuring library: msvQt${dir}")
  add_subdirectory(${dir})
endforeach()
