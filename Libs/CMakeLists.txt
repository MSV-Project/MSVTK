project(msvLibs)

# --------------------------------------------------------------------------
#                      ----] W A R N I N G [----
# --------------------------------------------------------------------------
# Order of project directory matters. Project should be topologically ordered
# --------------------------------------------------------------------------
set(dirs
# FileWatcher
  )

list(APPEND dirs
  VTK
  Qt
  )

#-----------------------------------------------------------------------------
# Loop over list of directories
#-----------------------------------------------------------------------------
foreach(dir ${dirs})
  message(STATUS "Configuring library: ${dir}")
  add_subdirectory(${dir})
endforeach()

#-----------------------------------------------------------------------------
# Generate list of include directories
#-----------------------------------------------------------------------------
set(include_dirs)
foreach(dir ${dirs})
  list(APPEND include_dirs ${${dir}_INCLUDE_DIRS})
endforeach()

# Set variable MSVTK_Libs_INCLUDE_DIRS
set(MSVTK_Libs_INCLUDE_DIRS ${include_dirs} CACHE INTERNAL "MSVTK Libs includes" FORCE)

#-----------------------------------------------------------------------------
# Set variable MSVTK_Libs_VTK_WRAPPED_LIBRARIES
#-----------------------------------------------------------------------------
set(MSVTK_Libs_VTK_WRAPPED_LIBRARIES
  VTK
  CACHE INTERNAL "MSVTK Libs VTK wrapped" FORCE)
