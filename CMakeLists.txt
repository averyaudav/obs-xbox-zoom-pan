cmake_minimum_required(VERSION 3.20)
project(obs-xbox-zoom-pan LANGUAGES C CXX)

# -------- Basics --------
set(PLUGIN_NAME "obs-xbox-zoom-pan")
set(PLUGIN_DESCRIPTION "Xbox controller zoom/pan dock for OBS")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_AUTOMOC ON)   # Qt signals/slots

# -------- Sources --------
add_library(${PLUGIN_NAME} MODULE
  src/plugin-main.cpp
  src/controller.cpp
  src/transformer.cpp
  src/settings.cpp
)

target_include_directories(${PLUGIN_NAME} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# -------- Dependencies --------
# libobs & frontend API (from OBS install or obs-deps kit)
find_package(libobs REQUIRED)
find_package(obs-frontend-api REQUIRED)

# Qt Widgets (provided by obs-deps on Windows; system/brew on macOS/Linux)
find_package(Qt6 COMPONENTS Widgets QUIET)
if (Qt6_FOUND)
  target_link_libraries(${PLUGIN_NAME} PRIVATE Qt6::Widgets)
else()
  find_package(Qt5 COMPONENTS Widgets REQUIRED)
  target_link_libraries(${PLUGIN_NAME} PRIVATE Qt5::Widgets)
endif()

# SDL2 — fetched automatically (no manual install needed)
include(FetchContent)
FetchContent_Declare(
  SDL2
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG        release-2.30.7
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(SDL2)
# Exports target SDL2::SDL2
target_link_libraries(${PLUGIN_NAME} PRIVATE SDL2::SDL2)

# Link OBS libs last
target_link_libraries(${PLUGIN_NAME} PRIVATE
  libobs
  obs-frontend-api
)

# -------- Platform output names --------
if (WIN32)
  set_target_properties(${PLUGIN_NAME} PROPERTIES
    OUTPUT_NAME ${PLUGIN_NAME}
    PREFIX ""   # -> obs-xbox-zoom-pan.dll
  )
elseif(APPLE)
  set_target_properties(${PLUGIN_NAME} PROPERTIES
    BUNDLE OFF
    MACOSX_BUNDLE OFF   # -> obs-xbox-zoom-pan.dylib
  )
endif()

# -------- Install (handy for local testing) --------
if (WIN32)
  set(OBS_PLUGIN_DIR "obs-plugins/64bit")
elseif(APPLE)
  set(OBS_PLUGIN_DIR "OBS.app/Contents/Plugins")
else()
  set(OBS_PLUGIN_DIR "lib/obs-plugins")
endif()

install(TARGETS ${PLUGIN_NAME}
  RUNTIME DESTINATION ${OBS_PLUGIN_DIR}
  LIBRARY DESTINATION ${OBS_PLUGIN_DIR}
)
