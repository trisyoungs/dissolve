# Don't forget to include output directory, otherwise the UI file won't be wrapped!

# Meta-Objects
set(widgets_MOC_HDRS
    mimestrings.h
    mimetreewidgetitem.h
    nocontrols.h
    sampleddoublelabel.h
    checkindicator.hui
    colourbutton.hui
    elementselector.hui
    exponentialspin.hui
    gradientbar.hui
    mimetreewidget.hui
    superstackedwidget.hui
)
qt6_wrap_cpp(widgets_MOC_SRCS ${widgets_MOC_HDRS})

# User Interface Files
set(widgets_UIS nocontrols.ui sampleddoublelabel.ui)
qt6_wrap_ui(widgets_UIS_H ${widgets_UIS})

# Source files (not Qt UI files or custom .h [uih])
set(widgets_SRCS
    checkindicator_funcs.cpp
    colourbutton_funcs.cpp
    elementselector_funcs.cpp
    exponentialspin_funcs.cpp
    gradientbar_funcs.cpp
    mimestrings.cpp
    mimetreewidget_funcs.cpp
    mimetreewidgetitem.cpp
    nocontrols_funcs.cpp
    sampleddoublelabel_funcs.cpp
    superstackedwidget_funcs.cpp
)

# Target 'gui'
add_library(widgets ${widgets_UIS_H} ${widgets_SRCS} ${widgets_MOC_SRCS})
target_include_directories(widgets PRIVATE ${PROJECT_SOURCE_DIR}/src ${PROJECT_BINARY_DIR}/src ${Qt6Widgets_INCLUDE_DIRS})
target_link_libraries(widgets PRIVATE base)
