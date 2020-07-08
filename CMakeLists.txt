cmake_minimum_required(VERSION 3.14...3.17)

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

# The whole project requires at least C++11.
set(CMAKE_CXX_STANDARD 11)

# Generate config files.
configure_file("sys.h.in" "sys.h" @ONLY)

#==============================================================================
# DEPENDENCIES
#

find_package(PkgConfig REQUIRED)

pkg_check_modules(giomm REQUIRED IMPORTED_TARGET giomm-2.4)
pkg_check_modules(glibmm REQUIRED IMPORTED_TARGET glibmm-2.4)
pkg_check_modules(gtkmm REQUIRED IMPORTED_TARGET gtkmm-3.0)

#==============================================================================
# BUILD OBJECT LIBRARY
#

# Create the chessposition OBJECT-library, used by the main project.
add_library(chessposition_ObjLib OBJECT)

# Set SOURCE_DIR to the current directory.
set_property(
  TARGET chessposition_ObjLib
  PROPERTY SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
)

# The list of source files.
target_sources(chessposition_ObjLib
  PRIVATE
    # ChessPosition
    "Direction.cxx"
    "ChessNotation.cxx"
    "MoveIterator.cxx"
    "ChessPosition.cxx"
    "Code.cxx"
    "CastleFlags.cxx"
    # ChessboardWidget
#    "ChessboardWidget.cxx"
    # ChessPositionWidget
#    "ChessPositionWidget.cxx"
#    "CwChessboard.cxx"
#    "Referenceable.cxx"
#    "MemoryBlockList.cxx"
)

# Add optionial sources.
if (OptionEnableLibcwd)
target_sources(chessposition_ObjLib
  PRIVATE
    "debug.cxx"
    "debug_ostream_operators.cxx"
)
endif ()

# Required include search-paths.
get_target_property(CWDS_INTERFACE_INCLUDE_DIRECTORIES AICxx::cwds INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(chessposition_ObjLib
  PUBLIC
    "${top_objdir}"                         # For sys.h.
    "${CWDS_INTERFACE_INCLUDE_DIRECTORIES}" # For debug.h.
)

target_link_libraries(chessposition_ObjLib
  PUBLIC
    PkgConfig::giomm
)

# Set link dependencies.
if (OptionEnableLibcwd)
  target_link_libraries(chessposition_ObjLib
    PUBLIC
      ${libcwd_r_TARGET}
)
endif ()

# Create an ALIAS target.
add_library(AICxx::cwchessboard ALIAS chessposition_ObjLib)

#==============================================================================
# GENERATED SOURCE FILES
#

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
    "${CMAKE_CURRENT_SRC_DIR}"          # For local header files.
)

add_library(generated::sources ALIAS generated_ObjLib)

#==============================================================================
# TEST EXECUTABLES
#

add_executable(tstchessposition tstchessposition.cxx)
target_link_libraries(tstchessposition PRIVATE AICxx::cwchessboard AICxx::cwds)

add_executable(tstbenchmark tstbenchmark.cxx)
target_link_libraries(tstbenchmark PRIVATE AICxx::cwchessboard AICxx::cwds)

add_executable(tstpgnread tstpgnread.cxx PgnDatabase.cxx MemoryBlockList.cxx)
target_link_libraries(tstpgnread PRIVATE generated_ObjLib AICxx::cwchessboard AICxx::cwds)

add_executable(tsticonv tsticonv.cxx)
target_link_libraries(tsticonv PRIVATE PkgConfig::glibmm)

add_executable(tstpgn tstpgn.cxx PgnDatabase.cxx MemoryBlockList.cxx)
target_link_libraries(tstpgn PRIVATE generated_ObjLib AICxx::cwchessboard AICxx::cwds)

add_executable(tstspirit tstspirit.cxx)
target_link_libraries(tstspirit PRIVATE generated_ObjLib)
target_include_directories(tstspirit PUBLIC "${top_objdir}")
