# Don't forget to include output directory, otherwise the UI file won't be wrapped!

# -----------
# Sub-Windows
# -----------

# Meta-Objects
set(components_MOC_HDRS
    # Gizmos
    graphgizmo.h
    integrator1dgizmo.h
    # Viewers
    configurationwidget.h
    datawidget.h
    selectsymbol.h
    sitewidget.h
    specieseditor.h
    specieswidget.h
    configurationviewer.hui
    dataviewer.hui
    siteviewer.hui
    speciesviewer.hui
    viewer.hui
    # Widgets and Wizards
    selectconfigurationwidget.h
    selectforcefieldwidget.h
    selectspecieswidget.h
    wizard.hui
    # Dialogs
    addforcefieldtermsdialog.h
    datamanagerdialog.h
    editspeciesdialog.h
    getconfigurationnamedialog.h
    getgizmonamedialog.h
    getmodulelayernamedialog.h
    getspeciesnamedialog.h
    gettabnamedialog.h
    importligpargendialog.h
    importspeciesdialog.h
    selectconfigurationdialog.h
    selectelementdialog.h
    selectforcefielddialog.h
    selectgenericitemdialog.h
    selectspeciesdialog.h
)
qt6_wrap_cpp(components_MOC_SRCS ${components_MOC_HDRS})

# User Interface Files
set(components_UIS
    # Gizmos
    graphgizmo.ui
    integrator1dgizmo.ui
    # Viewers
    configurationwidget.ui
    datawidget.ui
    selectsymbol.ui
    sitewidget.ui
    specieseditor.ui
    specieswidget.ui
    # Widgets and Wizards
    selectconfigurationwidget.ui
    selectforcefieldwidget.ui
    selectspecieswidget.ui
    wizardfooter.ui
    wizardheader.ui
    # Dialogs
    addforcefieldtermsdialog.ui
    datamanagerdialog.ui
    editspeciesdialog.ui
    getconfigurationnamedialog.ui
    getgizmonamedialog.ui
    getmodulelayernamedialog.ui
    getspeciesnamedialog.ui
    gettabnamedialog.ui
    importligpargendialog.ui
    importspeciesdialog.ui
    selectconfigurationdialog.ui
    selectelementdialog.ui
    selectforcefielddialog.ui
    selectgenericitemdialog.ui
    selectspeciesdialog.ui
)
qt6_wrap_ui(components_UIS_H ${components_UIS})

# Source files (not Qt UI files or custom .h [uih])
set(components_SRCS
    # Gizmos
    gizmo.cpp
    graphgizmo_funcs.cpp
    integrator1dgizmo_funcs.cpp
    gizmo.h
    # Viewers
    configurationviewer_funcs.cpp
    configurationviewer_input.cpp
    configurationviewer_interaction.cpp
    configurationviewer_render.cpp
    configurationwidget_funcs.cpp
    dataviewer_funcs.cpp
    dataviewer_input.cpp
    dataviewer_interaction.cpp
    dataviewer_contextmenu.cpp
    dataviewer_render.cpp
    datawidget_funcs.cpp
    selectsymbol_funcs.cpp
    siteviewer_funcs.cpp
    siteviewer_input.cpp
    siteviewer_interaction.cpp
    siteviewer_render.cpp
    sitewidget_funcs.cpp
    specieseditor_funcs.cpp
    speciesviewer_funcs.cpp
    speciesviewer_input.cpp
    speciesviewer_interaction.cpp
    speciesviewer_render.cpp
    specieswidget_funcs.cpp
    viewer_funcs.cpp
    viewer_input.cpp
    viewer_interaction.cpp
    viewer_options.cpp
    viewer_query.cpp
    viewer_render.cpp
    viewer_renderables.cpp
    viewer_view.cpp
    # Widgets and Wizards
    selectconfigurationwidget_funcs.cpp
    selectforcefieldwidget_funcs.cpp
    selectspecieswidget_funcs.cpp
    wizard_funcs.cpp
    # Dialogs
    addforcefieldtermsdialog_funcs.cpp
    datamanagerdialog_funcs.cpp
    editspeciesdialog_funcs.cpp
    getconfigurationnamedialog_funcs.cpp
    getgizmonamedialog_funcs.cpp
    getmodulelayernamedialog_funcs.cpp
    getspeciesnamedialog_funcs.cpp
    gettabnamedialog_funcs.cpp
    importligpargendialog_funcs.cpp
    importspeciesdialog_funcs.cpp
    selectconfigurationdialog_funcs.cpp
    selectelementdialog_funcs.cpp
    selectforcefielddialog_funcs.cpp
    selectgenericitemdialog_funcs.cpp
    selectspeciesdialog_funcs.cpp
)

# Target 'components'
add_library(components ${components_UIS_H} ${components_SRCS} ${components_MOC_SRCS})
target_include_directories(
  components PRIVATE ${PROJECT_SOURCE_DIR}/src ${PROJECT_BINARY_DIR}/src ${Qt6Widgets_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS}
                     ${Qt6OpenGLWidgets_INCLUDE_DIRS} ${CONAN_INCLUDE_DIRS_PUGIXML}
)

target_link_libraries(components widgets base)

# -----------------
# Main Dissolve GUI
# -----------------

# Meta-Objects
set(gui_MOC_HDRS
    # Editors
    modulecontrolwidget.h
    modulelisteditor.h
    modulewidget.h
    procedureeditor.h
    # Main Tabs
    configurationtab.h
    forcefieldtab.h
    layertab.h
    maintab.h
    speciestab.h
    workspacetab.h
    # Main UI
    gui.h
    stockcolours.h
    outputhandler.hui
    tmdiarea.hui
    thread.hui
    maintabsbar.hui
    maintabswidget.hui
)
qt6_wrap_cpp(gui_MOC_SRCS ${gui_MOC_HDRS})

# User Interface Files
set(gui_UIS
    # Editors
    modulecontrolwidget.ui
    modulelisteditor.ui
    procedureeditor.ui
    # Main Tabs
    configurationtab.ui
    forcefieldtab.ui
    layertab.ui
    speciestab.ui
    workspacetab.ui
    # Main UI
    gui.ui
)
qt6_wrap_ui(gui_UIS_H ${gui_UIS})

# Source files (not Qt UI files or custom .h [uih])
set(gui_SRCS
    # Editors
    modulecontrolwidget_funcs.cpp
    modulelisteditor_funcs.cpp
    modulewidget.cpp
    procedureeditor_funcs.cpp
    # Main Tabs
    configurationtab_funcs.cpp
    forcefieldtab_funcs.cpp
    layertab_funcs.cpp
    maintab.cpp
    maintabsbar_funcs.cpp
    maintabswidget_funcs.cpp
    speciestab_funcs.cpp
    speciestab_geometry.cpp
    speciestab_isotopologues.cpp
    speciestab_sites.cpp
    workspacetab_funcs.cpp
    # Main UI
    gui_funcs.cpp
    gui_io.cpp
    gui_simulation.cpp
    gui_start.cpp
    menu_configuration.cpp
    menu_file.cpp
    menu_help.cpp
    menu_layer.cpp
    menu_simulation.cpp
    menu_species.cpp
    menu_workspace.cpp
    outputhandler_funcs.cpp
    stockcolours.cpp
    thread_funcs.cpp
    tmdiarea_funcs.cpp
)

# Resources
set(gui_RES_QRC main.qrc)
qt6_add_resources(gui_RES ${gui_RES_QRC})

# Target 'gui'
add_library(gui ${gui_RES} ${gui_UIS_H} ${gui_SRCS} ${gui_MOC_SRCS})
target_include_directories(
  gui PRIVATE ${PROJECT_SOURCE_DIR}/src ${PROJECT_BINARY_DIR}/src ${Qt6Widgets_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS} Qt6::OpenGL
              Qt6::OpenGLWidgets
)
target_link_libraries(
  gui
  keywordwidgets
  charts
  delegates
  widgets
  components
  models
  base
  Qt6::OpenGL
  Qt6::OpenGLWidgets
)

# Subdirectories
add_subdirectory(keywordwidgets)
add_subdirectory(charts)
add_subdirectory(delegates)
add_subdirectory(render)
add_subdirectory(widgets)
add_subdirectory(models)
