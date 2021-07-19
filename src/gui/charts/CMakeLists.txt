# Don't forget to include output directory, otherwise the UI file won't be wrapped!

# Meta-Objects
set(charts_MOC_HDRS moduleblock.h moduleinsertionblock.h procedurenode.h chart.hui chartscrollarea.hui)
qt6_wrap_cpp(charts_MOC_SRCS ${charts_MOC_HDRS})

# User Interface Files
set(charts_UIS moduleblock.ui moduleinsertionblock.ui procedurenode.ui)
qt6_wrap_ui(charts_UIS_H ${charts_UIS})

# Source files (not Qt UI files or custom .h [uih])
set(charts_SRCS
    chart_funcs.cpp
    chartblock.cpp
    charthotspot.cpp
    chartscrollarea_funcs.cpp
    moduleblock_funcs.cpp
    moduleinsertionblock_funcs.cpp
    modulelist.cpp
    procedure.cpp
    procedurenode_funcs.cpp
    chartblock.h
    charthotspot.h
    modulelist.h
    modulelistmetrics.h
    procedure.h
    proceduremetrics.h
)

# Target 'charts'
add_library(charts ${charts_UIS_H} ${charts_SRCS} ${charts_MOC_SRCS})
target_include_directories(charts PRIVATE ${PROJECT_SOURCE_DIR}/src ${PROJECT_BINARY_DIR}/src ${Qt6Widgets_INCLUDE_DIRS})
target_link_libraries(charts PRIVATE base)
