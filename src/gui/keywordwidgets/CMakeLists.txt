# Don't forget to include output directory, otherwise the UI file won't be wrapped!

# Meta-Objects
set(keywordwidgets_MOC_HDRS
    atomtypeselection.h
    configurationvector.h
    dialog.h
    dropdown.h
    expressionvariablevector.h
    fileandformat.h
    function1d.h
    isotopologueset.h
    module.h
    modulegroups.h
    modulevector.h
    node.h
    nodeandinteger.h
    nodevalue.h
    nodevalueenumoptions.h
    nodevector.h
    range.h
    speciessite.h
    speciessitevector.h
    speciesvector.h
    vec3double.h
    vec3integer.h
    vec3labels.h
    vec3nodevalue.h
    bool.hui
    double.hui
    dropwidget.hui
    enumoptions.hui
    integer.hui
    species.hui
    stdstring.hui
    widget.hui
)
qt6_wrap_cpp(keywordwidgets_MOC_SRCS ${keywordwidgets_MOC_HDRS})

# User Interface Files
set(keywordwidgets_UIS
    atomtypeselection.ui
    configurationvector.ui
    dialog.ui
    dropdown.ui
    expressionvariablevector.ui
    fileandformat.ui
    function1d.ui
    isotopologueset.ui
    module.ui
    modulegroups.ui
    modulevector.ui
    node.ui
    nodeandinteger.ui
    nodevalue.ui
    nodevalueenumoptions.ui
    nodevector.ui
    range.ui
    speciessite.ui
    speciessitevector.ui
    speciesvector.ui
    vec3double.ui
    vec3integer.ui
    vec3nodevalue.ui
)
qt6_wrap_ui(keywordwidgets_UIS_H ${keywordwidgets_UIS})

# Source files (not Qt UI files or custom .h [uih])
set(keywordwidgets_SRCS
    base.cpp
    atomtypeselection_funcs.cpp
    bool_funcs.cpp
    configurationvector_funcs.cpp
    dialog_funcs.cpp
    double_funcs.cpp
    dropdown_funcs.cpp
    dropwidget_funcs.cpp
    enumoptions_funcs.cpp
    expressionvariablevector_funcs.cpp
    fileandformat_funcs.cpp
    function1d_funcs.cpp
    integer_funcs.cpp
    isotopologueset_funcs.cpp
    module_funcs.cpp
    modulegroups_funcs.cpp
    modulevector_funcs.cpp
    node_funcs.cpp
    nodeandinteger_funcs.cpp
    nodevalue_funcs.cpp
    nodevalueenumoptions_funcs.cpp
    nodevector_funcs.cpp
    range_funcs.cpp
    species_funcs.cpp
    speciessite_funcs.cpp
    speciessitevector_funcs.cpp
    speciesvector_funcs.cpp
    stdstring_funcs.cpp
    widget_funcs.cpp
    vec3double_funcs.cpp
    vec3integer_funcs.cpp
    vec3labels.cpp
    vec3nodevalue_funcs.cpp
    base.h
)

# Target 'keywordwidgets'
add_library(keywordwidgets ${keywordwidgets_UIS_H} ${keywordwidgets_SRCS} ${keywordwidgets_MOC_SRCS})
target_include_directories(keywordwidgets PRIVATE ${PROJECT_SOURCE_DIR}/src ${PROJECT_BINARY_DIR}/src ${Qt6Widgets_INCLUDE_DIRS})
target_link_libraries(keywordwidgets PRIVATE base)
