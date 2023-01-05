cmake_minimum_required(VERSION 3.14...3.17)

# Compile all subdirectories with the same standard.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(cwchessboard
        LANGUAGES CXX C
        DESCRIPTION "C++ chess widget, using vector graphics, and chess board utilities.")

# Begin of gitache configuration.
set(GITACHE_PACKAGES libcwd_r)
include(cwm4/cmake/StableGitache)
# End of gitache configuration.

# This project uses aicxx modules.
include(cwm4/cmake/AICxxProject)

# Exit if someone tries to contaminates the source directory with an in-source build.
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "Please do out-of-source builds.\nCleanup: \"rm -rf CMake CMakeCache.txt CMakeFiles/\"")
endif ()

include(AICxxSubmodules)

# Add the local cmake directory to CMAKE_MODULE_PATH.
list(APPEND CMAKE_MODULE_PATH
     "${CMAKE_CURRENT_SOURCE_DIR}/cwm4/cmake" )

include(AICxxProject)

# Generate config files.
configure_file("sys.h.in" "sys.h" @ONLY)

#==============================================================================
# DEPENDENCIES
#

find_package(PkgConfig REQUIRED)

pkg_check_modules(gtk REQUIRED IMPORTED_TARGET gtk+-3.0)
pkg_check_modules(giomm REQUIRED IMPORTED_TARGET giomm-2.4)
pkg_check_modules(glibmm REQUIRED IMPORTED_TARGET glibmm-2.4)
pkg_check_modules(gtkmm REQUIRED IMPORTED_TARGET gtkmm-3.0)
pkg_check_modules(cppunit REQUIRED IMPORTED_TARGET cppunit)

#==============================================================================
# BUILD OBJECT LIBRARIES
#

#------------------------------------------------------------------------------
# Create the chess position OBJECT-library, used by the main project.
add_library(position_ObjLib OBJECT)

# Set SOURCE_DIR to the current directory.
set_property(
  TARGET position_ObjLib
  PROPERTY SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
)

# The list of source files.
target_sources(position_ObjLib
  PRIVATE
    "Direction.cxx"
    "ChessNotation.cxx"
    "MoveIterator.cxx"
    "ChessPosition.cxx"
    "Code.cxx"
    "CastleFlags.cxx"
)

# Add optionial debug source files.
if (OptionEnableLibcwd)
target_sources(position_ObjLib
  PRIVATE
    "debug.cxx"
    "debug_ostream_operators.cxx"
)
endif ()

# Required include search-paths.
get_target_property(CWDS_INTERFACE_INCLUDE_DIRECTORIES AICxx::cwds INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(position_ObjLib
  PUBLIC
    "${top_objdir}"                         # For sys.h.
    "${CWDS_INTERFACE_INCLUDE_DIRECTORIES}" # For debug.h.
)

target_link_libraries(position_ObjLib
  PUBLIC
    PkgConfig::giomm
)

# Set link dependencies.
if (OptionEnableLibcwd)
  target_link_libraries(position_ObjLib
    PUBLIC
      ${libcwd_r_TARGET}
)
endif ()

# Create an ALIAS target.
add_library(CWChessboard::position ALIAS position_ObjLib)

#------------------------------------------------------------------------------
# Create the chess position_widget OBJECT-library, used by the main project.
add_library(positionwidget_ObjLib OBJECT)

# Set SOURCE_DIR to the current directory.
set_property(
  TARGET positionwidget_ObjLib
  PROPERTY SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
)

# The list of source files.
target_sources(positionwidget_ObjLib
  PRIVATE
    "ChessboardWidget.cxx"
    "ChessPositionWidget.cxx"
    "Referenceable.cxx"
    "MemoryBlockList.cxx"
)

# Add optionial debug source files.
if (OptionEnableLibcwd)
target_sources(positionwidget_ObjLib
  PRIVATE
    "debug_ostream_operators_gtkmm.cxx"
)
endif ()

# Required include search-paths.
get_target_property(CWDS_INTERFACE_INCLUDE_DIRECTORIES AICxx::cwds INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(positionwidget_ObjLib
  PUBLIC
    "${top_objdir}"                         # For sys.h.
    "${CWDS_INTERFACE_INCLUDE_DIRECTORIES}" # For debug.h.
)

target_link_libraries(positionwidget_ObjLib
  PRIVATE
    CWChessboard::position
  PUBLIC
    PkgConfig::gtkmm
)

# Create an ALIAS target.
add_library(CWChessboard::position_widget ALIAS positionwidget_ObjLib)

#==============================================================================
# GENERATED SOURCE FILES
#

#------------------------------------------------------------------------------
# generated::cpp_sources : chattr.tab.cpp and PgnGrammar.h.

add_executable(table_gen table_gen.cxx)

add_custom_command(OUTPUT chattr.tab.cpp
  DEPENDS table_gen
  COMMAND ./table_gen > chattr.tab.cpp
)

add_custom_command(OUTPUT PgnGrammar.h
  DEPENDS generate_PgnGrammar.h.sh
  COMMAND ${CMAKE_CURRENT_LIST_DIR}/generate_PgnGrammar.h.sh
)

add_library(generated_ObjLib OBJECT)
target_sources(generated_ObjLib
  PRIVATE
    "chattr.tab.cpp"
    "PgnGrammar.h"
)

target_include_directories(generated_ObjLib
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"          # For local header files.
)

add_library(generated::cpp_sources ALIAS generated_ObjLib)

#==============================================================================
# TEST EXECUTABLES
#

add_executable(tstchessposition tstchessposition.cxx)
target_link_libraries(tstchessposition PRIVATE CWChessboard::position AICxx::cwds)

add_executable(tstbenchmark tstbenchmark.cxx)
target_link_libraries(tstbenchmark PRIVATE CWChessboard::position AICxx::cwds)

add_executable(tstpgnread tstpgnread.cxx PgnDatabase.cxx MemoryBlockList.cxx)
target_link_libraries(tstpgnread PRIVATE generated::cpp_sources CWChessboard::position AICxx::cwds)

add_executable(tsticonv tsticonv.cxx)
target_link_libraries(tsticonv PRIVATE PkgConfig::glibmm)

add_executable(tstpgn tstpgn.cxx PgnDatabase.cxx MemoryBlockList.cxx)
target_link_libraries(tstpgn PRIVATE generated::cpp_sources CWChessboard::position AICxx::cwds)

add_executable(tstspirit tstspirit.cxx PgnGrammar.h)
target_include_directories(tstspirit PUBLIC "${top_objdir}" AICxx::cwds)

add_executable(testsuite testsuite.cxx)
target_link_libraries(testsuite PRIVATE CWChessboard::position AICxx::cwds PkgConfig::cppunit)

add_executable(linuxchess LinuxChessApplication.cxx LinuxChessboardWidget.cxx LinuxChess.cxx LinuxChessWindow.cxx LinuxChessMenuBar.cxx LinuxChessIconFactory.cxx)
target_link_libraries(linuxchess PRIVATE CWChessboard::position_widget CWChessboard::position AICxx::cwds)
