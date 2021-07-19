# Don't forget to include output directory, otherwise the UI file won't be wrapped!

# Meta-Objects
set(delegates_MOC_HDRS combolist.hui exponentialspin.hui integerspin.hui intraformcombo.hui isotopecombo.hui)
qt6_wrap_cpp(delegates_MOC_SRCS ${delegates_MOC_HDRS})

# User Interface Files SET(delegates_UIS ) QT6_WRAP_UI(delegates_UIS_H ${delegates_UIS})

# Source files (not Qt UI files or custom .h [uih])
set(delegates_SRCS
    combolist_funcs.cpp
    exponentialspin_funcs.cpp
    integerspin_funcs.cpp
    isotopecombo_funcs.cpp
    null.cpp
    customcombodelegate.h
    null.h
)

# Target 'delegates'
add_library(
  delegates
  # ${delegates_UIS_H}
  ${delegates_SRCS} ${delegates_MOC_SRCS}
)
target_include_directories(delegates PRIVATE ${PROJECT_SOURCE_DIR}/src ${PROJECT_BINARY_DIR}/src ${Qt6Widgets_INCLUDE_DIRS})
target_link_libraries(delegates PRIVATE base)
