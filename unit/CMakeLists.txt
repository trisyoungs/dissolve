# CMakeLists.txt for Unit Tests All tests must be single source file.

# Set policy for GoogleTest
cmake_policy(SET CMP0054 NEW)

# Get list of sources, from which we can construct the exe / test list.
file(GLOB tests_SRC "*.cpp")
find_package(GTest REQUIRED)

# Add executables and tests
foreach(SRCFILE ${tests_SRC})
  # Strip path and extension from source file to get test name
  get_filename_component(NAME ${SRCFILE} NAME_WE)

  # Register executable target
  add_executable(${NAME} ${SRCFILE})

  # Configure target
  target_include_directories(
    ${NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src ${PROJECT_BINARY_DIR}/src ${ANTLR4_INCLUDE_DIRS} ${ANTLR_OUTPUT_DIRS}
                    ${Qt6Widgets_INCLUDE_DIRS} ${CONAN_INCLUDE_DIRS_GTEST}
  )
  target_link_libraries(
    ${NAME}
    PUBLIC ${WHOLE_ARCHIVE_FLAG} ${BASIC_LINK_LIBS}
           # Module 'nogui' libs
           ${MODULENOGUI_LINK_LIBS} ${NO_WHOLE_ARCHIVE_FLAG}
    PRIVATE antlr4-runtime ${EXTRA_LINK_LIBS} GTest::gtest_main
    INTERFACE CONAN_PKG::fmt CONAN_PKG::CLI11
  )
  add_test(NAME ${NAME} COMMAND ${NAME})

  # Register the test
  gtest_discover_tests(${NAME} WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests")
endforeach(SRCFILE)

if(GUI)
  add_subdirectory(gui)
endif(GUI)
