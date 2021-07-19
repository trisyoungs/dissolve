# Macros
macro(dissolve_add_module header modulename)
  get_filename_component(MODULE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
  message(STATUS "... Module '${modulename}' in directory '${MODULE_DIR}'")

  # Create source file list
  set(MODULE_SRC "")
  file(
    GLOB MODULE_SRC
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "*.cpp"
  )
  list(APPEND MODULE_SRC ${header})
  message(STATUS "...    ${MODULE_SRC}")

  # Add GUI component if requested and it exists
  if(GUI AND IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/gui)
    add_subdirectory(gui)

    # Add dummy GUI target, so we can link both GUI and non-GUI versions of the code
    list(REMOVE_ITEM MODULE_SRC nogui.cpp)
    add_library(module_${modulename}_nogui nogui.cpp)
    set(MODULENOGUI_LINK_LIBS
        ${MODULENOGUI_LINK_LIBS} "module_${modulename}_nogui"
        CACHE INTERNAL "" FORCE
    )
    target_link_libraries(
      module_${modulename}_nogui
      PRIVATE base Qt6::OpenGL Qt6::OpenGLWidgets
      PUBLIC ${THREADING_LINK_LIBS}
    )
  endif()

  # Add library target
  add_library(module_${modulename} ${MODULE_SRC})
  set(MODULE_LINK_LIBS
      ${MODULE_LINK_LIBS} "module_${modulename}"
      CACHE INTERNAL "" FORCE
  )

  # Set common include dirs
  include_directories(
    module_${modulename}
    PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_BINARY_DIR}/src
    ${PROJECT_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/src
  )
  target_link_libraries(
    module_${modulename}
    PRIVATE base
    PUBLIC ${THREADING_LINK_LIBS}
  )
endmacro()
macro(dissolve_add_module_gui modulename)
  message(STATUS "...    GUI component 'module_${modulename}_gui'")

  # Create source file lists
  set(MODULEGUI_MOC_HDRS "modulewidget.h")
  message(STATUS "...       ${MODULEGUI_MOC_HDRS}")
  set(MODULEGUI_UIS "")
  file(
    GLOB MODULEGUI_UIS
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "*.ui"
  )
  message(STATUS "...       ${MODULEGUI_UIS}")
  set(MODULEGUI_SRC "")
  file(
    GLOB MODULEGUI_SRC
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    "*.cpp"
  )
  message(STATUS "...       ${MODULEGUI_SRC}")

  # Call Qt wrappers
  qt6_wrap_cpp(MODULEGUI_MOC_SRCS ${MODULEGUI_MOC_HDRS})
  qt6_wrap_ui(MODULEGUI_UIS_H ${MODULEGUI_UIS})

  # Add library target
  add_library(module_${modulename}_gui ${MODULEGUI_MOC_SRCS} ${MODULEGUI_UIS_H} ${MODULEGUI_SRC})
  set(MODULEGUI_LINK_LIBS
      ${MODULEGUI_LINK_LIBS} "module_${modulename}_gui"
      CACHE INTERNAL "" FORCE
  )

  # Add dependency on main GUI components
  add_dependencies(module_${modulename}_gui gui)

  # Set common include dirs
  include_directories(
    module_${modulename}_gui
    PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_BINARY_DIR}/src
    ${Qt6Core_INCLUDE_DIRS}
    ${Qt6Gui_INCLUDE_DIRS}
    ${Qt6Widgets_INCLUDE_DIRS}
    ${Qt6OpenGL_INCLUDE_DIRS}
    ${Qt6OpenGLWidgets_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS}
  )
  target_link_libraries(
    module_${modulename}_gui
    PRIVATE base
    PUBLIC ${THREADING_LINK_LIBS}
  )
endmacro()

# Module subdirectories
message(STATUS "Looking for modules in ${CMAKE_CURRENT_SOURCE_DIR}...")
file(
  GLOB children
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  "*"
)
set(MODULE_DIRS "")
foreach(child ${children})
  if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${child} AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${child}/CMakeLists.txt)
    list(APPEND MODULE_DIRS ${child})
  endif()
endforeach()

foreach(subdir ${MODULE_DIRS})
  add_subdirectory(${subdir})
endforeach()
