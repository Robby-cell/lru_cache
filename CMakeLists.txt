cmake_minimum_required(VERSION 3.15)
project(lru_cache VERSION 0.1.0 LANGUAGES CXX)

# Options for consumers, done perfectly.
option(LRU_CACHE_BUILD_TESTS "Build the tests for lru_cache" OFF)
option(LRU_CACHE_BUILD_EXAMPLES "Build the examples for lru_cache" OFF)

# =============================================================================
# 1. LIBRARY TARGET DEFINITION
# =============================================================================

# Create the INTERFACE library target. This is a "virtual" target for a
# header-only library, used to attach properties to.
add_library(lru_cache_lru_cache INTERFACE)

# (Best Practice) Create a namespaced alias. Consumers should link to lru_cache::lru_cache.
# This prevents target name collisions in larger projects.
add_library(lru_cache::lru_cache ALIAS lru_cache_lru_cache)

# Attach properties to the target. These will be inherited by anything that links to it.
target_include_directories(lru_cache_lru_cache INTERFACE
    # This path is used when building via FetchContent or add_subdirectory()
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>

    # This path is used when the installed package is found via find_package()
    $<INSTALL_INTERFACE:include>
)

# Propagate the C++ standard requirement to consumers.
target_compile_features(lru_cache_lru_cache INTERFACE cxx_std_11)

include(FetchContent)
FetchContent_Declare(
    hmm
    GIT_REPOSITORY https://github.com/Robby-cell/hmm.git
    GIT_TAG    v0.3.2
)
FetchContent_MakeAvailable(hmm)

target_link_libraries(lru_cache_lru_cache INTERFACE hmm::hmm)

# =============================================================================
# 2. INSTALLATION AND PACKAGE EXPORT
# =============================================================================

# Use standard GNU install directories (e.g., lib, include, bin).
include(GNUInstallDirs)
# Include helpers for generating package config files.
include(CMakePackageConfigHelpers)

# Install the headers.
# This single command correctly installs the 'include/lru_cache' directory
# into the standard include destination (e.g., /usr/local/include/lru_cache).
install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Generate the version file for find_package(lru_cache 1.2.3 ...)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/lru_cache-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

# Use the template to generate the main config file
configure_package_config_file(
    "cmake/lru_cache-config.cmake.in" # Assumes you have this template file
    "${CMAKE_CURRENT_BINARY_DIR}/lru_cache-config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/lru_cache
)

# Install the generated config files
install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/lru_cache-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/lru_cache-configVersion.cmake"
    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/lru_cache
)

# =============================================================================
# 3. OPTIONAL SUBDIRECTORIES (Tests, Examples)
# =============================================================================

if (LRU_CACHE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif ()

if (LRU_CACHE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()
