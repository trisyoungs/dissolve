// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Team Dissolve and contributors

#pragma once

#include "classes/referencepoint.h"
#include "gui/maintab.h"
#include "gui/outputhandler.hui"
#include "gui/thread.hui"
#include "gui/ui_gui.h"
#include "templates/list.h"

// Forward Declarations
class Configuration;
class ConfigurationTab;
class Dissolve;
class ForcefieldTab;
class QMdiSubWindow;
class Species;
class SpeciesTab;
class ModuleLayer;
class LayerTab;
class WorkspaceTab;

class DissolveWindow : public QMainWindow
{
    // All Qt declarations must include this macro
    Q_OBJECT

    public:
    DissolveWindow(Dissolve &dissolve);
    ~DissolveWindow() = default;

    /*
     * UI
     */
    private:
    // Main form declaration
    Ui::DissolveWindow ui_;

    protected:
    void closeEvent(QCloseEvent *event);
    void resizeEvent(QResizeEvent *event);

    /*
     * Dissolve Integration
     */
    private:
    // Dissolve reference
    Dissolve &dissolve_;
    // Whether any data has been modified
    bool modified_;
    // Whether window is currently refreshing
    bool refreshing_;
    // Output handler for messaging in GUI
    GUIOutputHandler outputHandler_;
    // Whether the current simulation is on the local machine
    bool localSimulation_;

    private:
    // Prepare the simulation and run for a set count
    void setupIteration(int count);

    public slots:
    // Flag that data has been modified via the GUI
    void setModified();

    public:
    // Return reference to Dissolve
    Dissolve &dissolve();
    const Dissolve &dissolve() const;
    // Link the Messenger in to the GUI output device
    void addOutputHandler();

    /*
     * StatusBar
     */
    private:
    // Label for local / remote simulation indicator
    QLabel *localSimulationIndicator_;
    // Label for restart file indicator
    QLabel *restartFileIndicator_;
    // Label for heartbeat file indicator
    QLabel *heartbeatFileIndicator_;
    // Label for iteration number
    QLabel *iterationLabel_;
    // Label for simulation ETA (when using RunFor)
    QLabel *etaLabel_;

    /*
     * File
     */
    public:
    // Open specified input file
    bool openLocalFile(std::string_view inputFile, std::string_view restartFile, bool ignoreRestartFile, bool ignoreLayoutFile);

    /*
     * Reference Points
     */
    private:
    // List of ReferencePoints currently loaded
    std::vector<ReferencePoint> referencePoints_;

    /*
     * Update Functions
     */
    public slots:
    // Update window title
    void updateWindowTitle();
    // Update status bar
    void updateStatusBar();
    // Update menus
    void updateMenus();
    // Perform full update of the GUI, including tab reconciliation
    void fullUpdate();
    // Update while running
    void updateWhileRunning(int iterationsRemaining);

    /*
     * Main Menu
     */
    private:
    // Check whether current input needs to be saved and, if so, if it saved successfully
    bool checkSaveCurrentInput();
    // Clear all data and start new simulation afresh
    void startNew();

    private slots:
    // File
    void on_FileNewAction_triggered(bool checked);
    void on_FileOpenLocalAction_triggered(bool checked);
    void on_FileOpenRecentAction_triggered(bool checked);
    void on_FileConnectAction_triggered(bool checked);
    void on_FileCloseAction_triggered(bool checked);
    void on_FileSaveAction_triggered(bool checked);
    void on_FileSaveAsAction_triggered(bool checked);
    void on_FileQuitAction_triggered(bool checked);
    // Simulation
    void on_SimulationRunAction_triggered(bool checked);
    void on_SimulationRunForAction_triggered(bool checked);
    void on_SimulationStepAction_triggered(bool checked);
    void on_SimulationStepFiveAction_triggered(bool checked);
    void on_SimulationPauseAction_triggered(bool checked);
    void on_SimulationSaveRestartPointAction_triggered(bool checked);
    void on_SimulationDataManagerAction_triggered(bool checked);
    void on_SimulationClearModuleDataAction_triggered(bool checked);
    void on_SimulationSetRandomSeedAction_triggered(bool checked);
    // Species
    void on_SpeciesCreateAtomicAction_triggered(bool checked);
    void on_SpeciesCreateDrawAction_triggered(bool checked);
    void on_SpeciesImportFromDissolveAction_triggered(bool checked);
    void on_SpeciesImportFromXYZAction_triggered(bool checked);
    void on_SpeciesImportLigParGenAction_triggered(bool checked);
    void on_SpeciesRenameAction_triggered(bool checked);
    void on_SpeciesAddForcefieldTermsAction_triggered(bool checked);
    void on_SpeciesRegenerateIntraFromConnectivityAction_triggered(bool checked);
    void on_SpeciesSimplifyAtomTypesAction_triggered(bool checked);
    void on_SpeciesReduceToMasterTermsAction_triggered(bool checked);
    void on_SpeciesDeleteAction_triggered(bool checked);
    // Configuration
    void on_ConfigurationCreateEmptyAction_triggered(bool checked);
    void on_ConfigurationCreateSimpleRandomMixAction_triggered(bool checked);
    void on_ConfigurationCreateRelativeRandomMixAction_triggered(bool checked);
    void on_ConfigurationCreateEmptyFrameworkAction_triggered(bool checked);
    void on_ConfigurationCreateFrameworkAdsorbatesAction_triggered(bool checked);
    void on_ConfigurationRenameAction_triggered(bool checked);
    void on_ConfigurationDeleteAction_triggered(bool checked);
    void on_ConfigurationExportToXYZAction_triggered(bool checked);
    // Layer
    void on_LayerCreateEmptyAction_triggered(bool checked);
    void on_LayerCreateEvolveBasicAtomicAction_triggered(bool checked);
    void on_LayerCreateEvolveAtomicAction_triggered(bool checked);
    void on_LayerCreateEvolveMolecularAction_triggered(bool checked);
    void on_LayerCreateEvolveEPSRAction_triggered(bool checked);
    void on_LayerCreateRefineEPSRAction_triggered(bool checked);
    void on_LayerCreateCalculateRDFAction_triggered(bool checked);
    void on_LayerCreateCalculateRDFStructureFactorAction_triggered(bool checked);
    void on_LayerCreateCalculateRDFNeutronAction_triggered(bool checked);
    void on_LayerCreateCalculateRDFNeutronXRayAction_triggered(bool checked);
    void on_LayerCreateAnalyseRDFCNAction_triggered(bool checked);
    void on_LayerCreateAnalyseAvgMolSDFAction_triggered(bool checked);
    void on_LayerRenameAction_triggered(bool checked);
    void on_LayerDeleteAction_triggered(bool checked);
    // Workspace
    void on_WorkspaceCreateEmptyAction_triggered(bool checked);
    void on_WorkspaceRenameCurrentGizmoAction_triggered(bool checked);
    // Help
    void on_HelpOnlineManualAction_triggered(bool checked);
    void on_HelpOnlineTutorialsAction_triggered(bool checked);

    public slots:
    void currentWorkspaceGizmoChanged(QMdiSubWindow *gizmoWindow);

    /*
     * Widgets / UI
     */
    public:
    // Dissolve State Enum
    enum DissolveState
    {
        NoState,         /* Dissolve has no active simulation */
        EditingState,    /* Dissolve is currently editing a file in the GUI */
        RunningState,    /* Dissolve is currently running in the GUI */
        MonitoringState, /* Dissolve is running in the background, and we are monitoring it via the restart file */
        nDissolveStates
    };

    private:
    // Thread controller
    DissolveThreadController threadController_;
    // Current state of Dissolve
    DissolveState dissolveState_;

    private slots:
    void on_MainTabs_currentChanged(int index);
    void on_MessagesIncreaseFontSizeButton_clicked(bool checked);
    void on_MessagesDecreaseFontSizeButton_clicked(bool checked);
    void clearMessages();
    void appendMessage(const QString &msg);

    public:
    // Return list of all current tabs
    const std::vector<std::shared_ptr<MainTab>> allTabs() const;

    public slots:
    // Disable sensitive controls
    void disableSensitiveControls();
    // Enable sensitive controls
    void enableSensitiveControls();
    // All iterations requested are complete
    void iterationsComplete();

    signals:
    void iterate(int);
    void stopIterating();

    /*
     * GUI State
     */
    private:
    // Filename containing current GUI state
    QString stateFilename_;

    public:
    // Save current GUI state
    bool saveState();
    // Load GUI state
    bool loadState();
};
