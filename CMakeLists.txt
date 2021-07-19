cmake_minimum_required(VERSION 3.15)
project(Dissolve)

set(DESCRIPTION "Dissolve")
set(AUTHOR "Tristan Youngs")
set(VERSION_MAJOR "0")
set(VERSION_MINOR "9")
set(VERSION_PATCH "0")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Make sure CMake won't try to generate rules for moc (we will do it ourselves)
set(CMAKE_AUTOMOC OFF)

# Set policy for automatic linkage of Qt libs to project
cmake_policy(SET CMP0020 NEW)

# Request C++17 Standard Support (Required for std::optional and auto types in lambda parameters)
set(CMAKE_CXX_STANDARD 17)

# Include Conan Packages
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

# Catch defines
if(PARALLEL)
  add_definitions("-DPARALLEL")
  find_package(MPI REQUIRED)
  include_directories(${MPI_INCLUDE_PATH})
  set(EXTRA_LINK_LIBS ${EXTRA_LINK_LIBS} ${MPI_LIBRARIES})
endif(PARALLEL)

# Build with threads
option(MULTI_THREADING "Enable threading using tbb" ON)
if(MULTI_THREADING)
  add_definitions("-DMULTITHREADING")
  set(THREADING_LINK_LIBS ${THREADING_LINK_LIBS} CONAN_PKG::tbb)
endif(MULTI_THREADING)

# Add local Modules dir to cmake search path
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/Modules")

# Find ANTLR4 Java
include(FindANTLR)
find_package(ANTLR REQUIRED)

# Find / build ANTLR4 runtime dependency
if(BUILD_ANTLR_RUNTIME)

  if(BUILD_ANTLR_ZIPFILE)
    set(ANTLR_SOURCE_URL "${BUILD_ANTLR_ZIPFILE}")
  else(BUILD_ANTLR_ZIPFILE)
    set(ANTLR_SOURCE_URL "https://www.antlr.org/download/antlr4-cpp-runtime-4.8-source.zip")
  endif(BUILD_ANTLR_ZIPFILE)

  if(BUILD_ANTLR_LINK_LIB)
    set(ANTLR_LINK_LIB "${BUILD_ANTLR_LINK_LIB}")
  else(BUILD_ANTLR_LINK_LIB)
    set(ANTLR_LINK_LIB "dist/libantlr4-runtime.a")
  endif(BUILD_ANTLR_LINK_LIB)

  include(ExternalProject)
  externalproject_add(
    antlr_runtime
    PREFIX antlr4runtime
    URL "${ANTLR_SOURCE_URL}"
    BUILD_ALWAYS ON
    BUILD_BYPRODUCTS "antlr4runtime/src/antlr_runtime/${ANTLR_LINK_LIB}"
    CMAKE_ARGS "-DCMAKE_BUILD_TYPE:STRING=Release"
    INSTALL_COMMAND ""
  )
  externalproject_get_property(antlr_runtime source_dir)

  # Tell CMake that the ExternalProject generated a library
  add_library(antlr4-runtime STATIC IMPORTED GLOBAL)
  set_property(TARGET antlr4-runtime PROPERTY IMPORTED_LOCATION "${source_dir}/${ANTLR_LINK_LIB}")
  set(ANTLRRUNTIME_INCLUDE_DIRS "${source_dir}/runtime/src")
else(BUILD_ANTLR_RUNTIME)
  include(FindANTLRRUNTIME)
  find_package(ANTLRRUNTIME REQUIRED)
  add_library(antlr4-runtime STATIC IMPORTED GLOBAL)
  set_property(TARGET antlr4-runtime PROPERTY IMPORTED_LOCATION "${ANTLRRUNTIME_LIBRARIES}")
endif(BUILD_ANTLR_RUNTIME)

if(GUI)
  # Add extra functions
  include(FindFTGL)

  if(APPLE)
    set(CMAKE_PREFIX_PATH "${QT_BASE_DIR}")
  endif(APPLE)

  find_package(OpenGL REQUIRED)
  find_package(Qt6 COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets REQUIRED)
  find_package(FTGL REQUIRED)
  find_package(Freetype REQUIRED)
  find_package(Qt6OpenGL 6.0 REQUIRED)
  find_package(Qt6OpenGLWidgets 6.0 REQUIRED)

  # Make sure CMake won't try to generate rules for moc (we will do it ourselves)
  set(CMAKE_AUTOMOC OFF)

  # Set policy for automatic linkage of Qt libs to project
  cmake_policy(SET CMP0020 NEW)
endif(GUI)

# Perform system-specific setup -- Windows
if(WIN32)
  if(PARALLEL)
    set(target_name Dissolve-MPI)
  else(PARALLEL)
    set(target_name Dissolve)
    set(gui_target_name Dissolve-GUI)
  endif(PARALLEL)

  # Adjust external include directories for GUI
  if(GUI)
    include_directories(${FTGL_INCLUDE_DIRS})
  endif(GUI)

  # Add defines for Windows systems - NOMINMAX to prevent conflicts with std::min() and std::max() - NOGDI to prevent conflicts arising from
  # Windows defining ERROR as a global macro (used in ANTLR4)
  add_definitions(-DNOMINMAX -DNOGDI)
endif(WIN32)

# -- Unix
if(UNIX)
  if(PARALLEL)
    set(target_name dissolve-mpi)
  else(PARALLEL)
    set(target_name dissolve)
    set(gui_target_name dissolve-gui)
  endif(PARALLEL)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif(UNIX)

# -- OSX
if(APPLE)
  # Set some specific C++11 related options here (set_property below does not seem to persist)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")

  # Suppress warnings for undefined template vars
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-undefined-var-template")

  if(PARALLEL)
    set(target_name dissolve-mpi)
  else(PARALLEL)
    set(target_name dissolve)
    set(gui_target_name dissolve-gui)
  endif(PARALLEL)

  set(CMAKE_OSX_ARCHITECTURES "x86_64")
  add_definitions(-D_MAC)

  if(MULTI_THREADING)
    set(THREADING_LINK_LIBS ${THREADING_LINK_LIBS} CONAN_PKG::onedpl)
  endif(MULTI_THREADING)

  if(GUI)
    # Find GLUT
    find_package(GLUT REQUIRED)

    # Adjust external include directories for GUI
    include_directories(${FTGL_INCLUDE_DIRS})
  endif(GUI)
endif(APPLE)

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
set(SRCS src/)

# Set bundle info
if(APPLE)
  set(MACOSX_BUNDLE_ICON_FILE "Dissolve.icns")
  set(MACOSX_BUNDLE_GUI_IDENTIFIER "Dissolve")
  set(MACOSX_BUNDLE_LONG_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
  set(MACOSX_BUNDLE_BUNDLE_NAME "Dissolve")
  set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}")
  set(MACOSX_BUNDLE_BUNDLE_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
  set(MACOSX_BUNDLE_COPYRIGHT "${AUTHOR}")
endif(APPLE)

# Process CMakeLists in subdirs
set(MODULE_LINK_LIBS
    ""
    CACHE INTERNAL ""
)
set(MODULEGUI_LINK_LIBS
    ""
    CACHE INTERNAL ""
)
set(MODULENOGUI_LINK_LIBS
    ""
    CACHE INTERNAL ""
)
set(FF_LINK_LIBS
    ""
    CACHE INTERNAL ""
)
add_subdirectory(${SRCS})

# Add executable target(s)
add_executable(${target_name} MACOSX_BUNDLE ${SRCS}/main.cpp)

# Add the ANTLR4 dependency if building locally
if(BUILD_ANTLR_RUNTIME)
  add_dependencies(${target_name} antlr_runtime)
endif(BUILD_ANTLR_RUNTIME)

# Set project-local include directories for target
target_include_directories(
  ${target_name} PRIVATE ${PROJECT_SOURCE_DIR}/src ${PROJECT_BINARY_DIR}/src ${FREETYPE_INCLUDE_DIRS} ${ANTLR4_INCLUDE_DIRS}
                         ${ANTLR_OUTPUT_DIRS}
)

# Add GUI targets
if(GUI)

  # Main Dissolve GUI
  add_executable(${gui_target_name} MACOSX_BUNDLE ${SRCS}/dissolve-gui.cpp)

  # Add the ANTLR4 dependency if building locally
  if(BUILD_ANTLR_RUNTIME)
    add_dependencies(${target_name} antlr_runtime)
  endif(BUILD_ANTLR_RUNTIME)

  # Set project-local include directories for target
  target_include_directories(
    ${gui_target_name}
    PRIVATE ${PROJECT_SOURCE_DIR}/src
            ${PROJECT_BINARY_DIR}/src
            ${FREETYPE_INCLUDE_DIRS}
            ${Qt6Core_INCLUDE_DIRS}
            ${Qt6Gui_INCLUDE_DIRS}
            ${Qt6OpenGL_INCLUDE_DIRS}
            ${Qt6Widgets_INCLUDE_DIRS}
            ${Qt6OpenGL_INCLUDE_DIRS}
            ${Qt6OpenGLWidgets_INCLUDE_DIRS}
  )

endif(GUI)

# Set basic link libs for executables
set(BASIC_LINK_LIBS
    # Main libs
    main
    classes
    module
    io
    export
    import
    neta
    procedure
    procedurenodes
    keywords
    expression
    genericitems
    base
    math
    data
    # Forcefields
    ff
    ${FF_LINK_LIBS}
    # Modules
    ${MODULE_LINK_LIBS}
)

# Set linker options for complete/no archive
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(WHOLE_ARCHIVE_FLAG "-Wl,-all_load")
  set(NO_WHOLE_ARCHIVE_FLAG "-Wl,-noall_load")
else()
  set(WHOLE_ARCHIVE_FLAG "-Wl,-whole-archive")
  set(NO_WHOLE_ARCHIVE_FLAG "-Wl,-no-whole-archive")
endif()

# Enable build of unit / system tests?
option(BUILD_SYSTEM_TESTS "Build system tests" OFF)
option(BUILD_UNIT_TESTS "Build unit tests" OFF)
if(BUILD_SYSTEM_TESTS OR BUILD_UNIT_TESTS)
  enable_testing()
  if(BUILD_SYSTEM_TESTS)
    add_subdirectory(tests)
  endif(BUILD_SYSTEM_TESTS)
  if(BUILD_UNIT_TESTS)
    add_subdirectory(unit)
  endif(BUILD_UNIT_TESTS)
endif(BUILD_SYSTEM_TESTS OR BUILD_UNIT_TESTS)

if(BUILD_BENCHMARKS)
  # The conan package of goooglebenchmark segfaults - so lets use fetch content
  include(FetchContent)
  set(BENCHMARK_ENABLE_TESTING "OFF")
  fetchcontent_declare(
    googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.5.4
  )

  fetchcontent_makeavailable(googlebenchmark)
  add_subdirectory(benchmark)
endif(BUILD_BENCHMARKS)

# Build CLI target executable
target_link_libraries(
  ${target_name}
  PUBLIC ${WHOLE_ARCHIVE_FLAG} ${BASIC_LINK_LIBS}
         # Module 'nogui' libs
         ${MODULENOGUI_LINK_LIBS} ${NO_WHOLE_ARCHIVE_FLAG}
  PRIVATE # External libs
          antlr4-runtime ${EXTRA_LINK_LIBS} ${THREADING_LINK_LIBS}
  INTERFACE CONAN_PKG::fmt CONAN_PKG::CLI11
)

# Build GUI target executables
if(GUI)

  # Main Dissolve GUI
  target_link_libraries(
    ${gui_target_name}
    PUBLIC ${WHOLE_ARCHIVE_FLAG}
           ${BASIC_LINK_LIBS}
           # Module gui libs
           ${MODULEGUI_LINK_LIBS}
           # Main gui libs
           render
           widgets
           keywordwidgets
           delegates
           charts
           components
           models
           gui
           ${NO_WHOLE_ARCHIVE_FLAG}
    PRIVATE # External libs
            antlr4-runtime
            Qt6::Widgets
            Qt6::Core
            OpenGL::GL
            ${FTGL_LIBRARIES}
            ${FREETYPE_LIBRARIES}
            ${EXTRA_LINK_LIBS}
            ${THREADING_LINK_LIBS}
    INTERFACE CONAN_PKG::fmt CONAN_PKG::CLI11 CONAN_PKG::pugixml
  )
endif(GUI)
