cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

# Project information
project(
  "libtorrent"
  DESCRIPTION "BitTorrent library"
  VERSION 0.13.8
  LANGUAGES CXX C)

# ABI version information
set(INTERFACE_CURRENT 21)
set(INTERFACE_REVISION 0)
set(INTERFACE_AGE 0)
set(INTERFACE_VERSION
    ${INTERFACE_CURRENT}.${INTERFACE_REVISION}.${INTERFACE_AGE})

# Options
include(CMakeDependentOption)
option(BUILD_SHARED_LIBS "Build shared libraries (.dll/.so)" ON)
option(BUILD_TESTS "Build test suite (libtorrent_test)" ON)
option(BUILDINFO_ONLY "Generate buildinfo.h only" OFF)
option(USE_EXTRA_DEBUG "Enable extra debugging checks" OFF)
option(USE_INSTRUMENTATION "Enable instrumentation" OFF)
cmake_dependent_option(DISCOVER_TESTS "Discover ctest entries from gtest" ON BUILD_TESTS OFF)

# Include CMake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Compiler options - Common
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options("-Wall" "-Wextra" "-Wpedantic")

# Compiler options - Optimizations
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_options("-Og")
else()
  if(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    add_compile_options("-Os")
  else()
    add_compile_options("-O3")
  endif()
endif()

# Compiler options - Debug information
if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL
                                        "RelWithDebInfo")
  add_compile_options("-g")
else()
  add_compile_options("-flto")
  add_link_options("-flto" "-s")
endif()

# Use GNU install destinations
include(GNUInstallDirs)

# Generate buildinfo.h
include(GenerateLibtorrentBuildinfo)
install(FILES ${CMAKE_BINARY_DIR}/include/torrent/buildinfo.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/torrent/)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include ${CMAKE_BINARY_DIR}/include)

if(NOT BUILDINFO_ONLY)
  # Generate libtorrent.pc
  configure_file("libtorrent.pc.in" ${CMAKE_BINARY_DIR}/libtorrent.pc @ONLY)
  install(FILES ${CMAKE_BINARY_DIR}/libtorrent.pc
          DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig/)

  # Required packages
  set(CMAKE_THREAD_PREFER_PTHREAD ON)
  set(THREADS_PREFER_PTHREAD_FLAG ON)

  find_package(OpenSSL REQUIRED)
  include_directories(${OPENSSL_INCLUDE_DIR})

  find_package(Threads REQUIRED)

  find_package(ZLIB REQUIRED)
  include_directories(${ZLIB_INCLUDE_DIRS})

  # headers
  install(DIRECTORY include/torrent DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

  # libtorrent
  file(GLOB_RECURSE LIBTORRENT_SRCS "${PROJECT_SOURCE_DIR}/src/*.cc")
  add_library(torrent ${LIBTORRENT_SRCS})
  target_link_libraries(torrent ${CMAKE_THREAD_LIBS_INIT}
                        ${OPENSSL_CRYPTO_LIBRARY} ${ZLIB_LIBRARIES})
  target_compile_definitions(torrent PRIVATE EXPORT_LIBTORRENT_SYMBOLS=1)
  set_target_properties(torrent PROPERTIES VERSION ${INTERFACE_VERSION}
                                           SOVERSION ${INTERFACE_CURRENT})
  install(
    TARGETS torrent
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

  # tests
  if(BUILD_TESTS)
    find_package(GTest)
    if(GTest_FOUND)
      include_directories(${GTEST_INCLUDE_DIRS})
      enable_testing()
      file(GLOB_RECURSE LIBTORRENT_TEST_SRCS "${PROJECT_SOURCE_DIR}/test/*.cc")
      add_executable(libtorrent_test ${LIBTORRENT_TEST_SRCS})
      target_link_libraries(libtorrent_test torrent ${GTEST_LIBRARIES})
      if (DISCOVER_TESTS AND NOT CMAKE_SKIP_BUILD_RPATH)
        gtest_discover_tests(libtorrent_test)
      else()
        add_test(NAME libtorrent_test COMMAND libtorrent_test)
      endif()
    endif()
  endif()
endif()
