/*
    *** Dissolve GUI - Simulation Menu Functions
    *** src/gui/menu_simulation.cpp
    Copyright T. Youngs 2012-2020

    This file is part of Dissolve.

    Dissolve is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Dissolve is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Dissolve.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "gui/datamanagerdialog.h"
#include "gui/gui.h"
#include "main/dissolve.h"
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>

void DissolveWindow::on_SimulationRunAction_triggered(bool checked)
{
    // Prepare the simulation
    if (!dissolve_.prepare())
        return;

    // Prepare the GUI
    disableSensitiveControls();
    Renderable::setSourceDataAccessEnabled(false);
    dissolveState_ = DissolveWindow::RunningState;

    // Update the controls
    updateControlsFrame();

    emit iterate(-1);
}

void DissolveWindow::on_SimulationRunForAction_triggered(bool checked)
{
    // Get the number of iterations to run
    bool ok;
    int nIterations =
        QInputDialog::getInt(this, "Iterate Simulation...", "Enter the number of iterations to run", 10, 1, 1000000, 10, &ok);
    if (!ok)
        return;

    // Prepare the simulation
    if (!dissolve_.prepare())
        return;

    // Prepare the GUI
    disableSensitiveControls();
    Renderable::setSourceDataAccessEnabled(false);
    dissolveState_ = DissolveWindow::RunningState;

    // Update the controls
    updateControlsFrame();

    emit iterate(nIterations);
}

void DissolveWindow::on_SimulationStepAction_triggered(bool checked)
{
    // Prepare the simulation
    if (!dissolve_.prepare())
        return;

    // Prepare the GUI
    disableSensitiveControls();
    Renderable::setSourceDataAccessEnabled(false);
    dissolveState_ = DissolveWindow::RunningState;

    // Update the controls
    updateControlsFrame();

    emit iterate(1);
}

void DissolveWindow::on_SimulationStepFiveAction_triggered(bool checked)
{
    // Prepare the simulation
    if (!dissolve_.prepare())
        return;

    // Prepare the GUI
    disableSensitiveControls();
    Renderable::setSourceDataAccessEnabled(false);
    dissolveState_ = DissolveWindow::RunningState;

    // Update the controls
    updateControlsFrame();

    emit iterate(5);
}

void DissolveWindow::on_SimulationPauseAction_triggered(bool checked)
{
    // Set run icon button to the 'pausing' icon (it will be set back to normal by setWidgetsAfterRun())
    ui_.ControlRunButton->setIcon(QIcon(":/control/icons/control_waiting.svg"));

    // Send the signal to stop
    emit(stopIterating());

    // Disable the pause button
    ui_.ControlPauseButton->setEnabled(false);
    Renderable::setSourceDataAccessEnabled(true);
}

void DissolveWindow::on_SimulationSaveRestartPointAction_triggered(bool checked)
{
    // Get filename for restart point
    QString filename =
        QFileDialog::getSaveFileName(this, "Select Output File", QDir::currentPath(), "Restart Files (*.restart)");
    if (filename.isEmpty())
        return;

    if (dissolve_.saveRestart(qPrintable(filename)))
        Messenger::print("Saved restart point to '%s'.\n", qPrintable(filename));
    else
        Messenger::error("Failed to save restart point to '%s'.\n", qPrintable(filename));
}

void DissolveWindow::on_SimulationDataManagerAction_triggered(bool checked)
{
    DataManagerDialog dataManagerDialog(this, dissolve_, referencePoints_);
    dataManagerDialog.exec();
}

void DissolveWindow::on_SimulationClearModuleDataAction_triggered(bool checked)
{
    QMessageBox queryBox;
    queryBox.setText("This will delete all data generated by modules, and reset the iteration counter to zero. "
                     "Configuration contents will remain as-is.");
    queryBox.setInformativeText("This cannot be undone. Proceed?");
    queryBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    queryBox.setDefaultButton(QMessageBox::No);
    int ret = queryBox.exec();

    if (ret == QMessageBox::Yes)
    {
        // Invalidate all renderables before we clear the data
        Renderable::invalidateAll();

        // Clear main processing data
        dissolve_.processingModuleData().clear();

        // Clear local processing data in configurations
        ListIterator<Configuration> configIterator(dissolve_.configurations());
        while (Configuration *cfg = configIterator.iterate())
            cfg->moduleData().clear();

        // Set iteration counter to zero
        dissolve_.resetIterationCounter();

        // Regenerate pair potentials
        dissolve_.regeneratePairPotentials();

        fullUpdate();
    }
}

void DissolveWindow::on_SimulationSetRandomSeedAction_triggered(bool checked)
{
    // Create an input dialog to get the new seed
    bool ok;
    dissolve_.seed();
    int newSeed =
        QInputDialog::getInt(this, "Set random seed", "Enter the new value of the random seed, or -1 to remove set value",
                             dissolve_.seed(), -1, 2147483647, 1, &ok);

    if (!ok)
        return;

    // Set and initialise random seed
    dissolve_.setSeed(newSeed);

    if (dissolve_.seed() == -1)
        srand((unsigned)time(NULL));
    else
        srand(dissolve_.seed());
}
