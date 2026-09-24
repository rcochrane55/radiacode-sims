
#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 3.5)
project(simulazione_marinelli)

#----------------------------------------------------------------------------
# Find Geant4 package, activating all available UI and Vis drivers by default
# You can set WITH_GEANT4_UIVIS to OFF via the command line or ccmake/cmake-gui
# to build a batch mode only executable
#
option(WITH_GEANT4_UIVIS "Build example with Geant4 UI and Vis drivers" ON)
if(WITH_GEANT4_UIVIS)
  find_package(Geant4 REQUIRED ui_all vis_all)
else()
  find_package(Geant4 REQUIRED)
endif()

#----------------------------------------------------------------------------
# Setup Geant4 include directories and compile definitions
# Setup include directory for this project
#
include(${Geant4_USE_FILE})
include_directories(${PROJECT_SOURCE_DIR}/include)


#----------------------------------------------------------------------------
# Locate sources and headers for this project
# NB: headers are included so they will show up in IDEs
#
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

#----------------------------------------------------------------------------
# Add the executable, and link it to the Geant4 libraries
#
add_executable(marinelli marinelli.cc ${sources} ${headers})
target_link_libraries(marinelli ${Geant4_LIBRARIES})

#----------------------------------------------------------------------------
# Copy all scripts to the build directory, i.e. the directory in which we
# build marinelli. This is so that we can run the executable directly because it
# relies on these scripts being in the current working directory.
#
set(marinelli_FILES
  init_vis.mac
  vis.mac
  make_emissions_dat.py
  optical_map.mac
  absorption_length_data.csv
  emission_spectrum_data.csv
  create_macro.py
  merge_outputs.py
  )

foreach(_file ${marinelli_FILES})
  add_custom_command(
    TARGET marinelli POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${PROJECT_SOURCE_DIR}/${_file}
        $<TARGET_FILE_DIR:marinelli>
    )
endforeach()

#----------------------------------------------------------------------------
# For internal Geant4 use - but has no effect if you build this
# example standalone
#
#add_custom_target(marinelli DEPENDS marinelli)

#----------------------------------------------------------------------------
# Install the executable to 'bin' directory under CMAKE_INSTALL_PREFIX
#
install(TARGETS marinelli DESTINATION bin)


