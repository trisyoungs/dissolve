// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Team Dissolve and contributors

#include "gui/gui.h"
#include "main/dissolve.h"
#include <QFileDialog>
#include <QMessageBox>

// Check whether current input needs to be saved and, if so, if it saved successfully
bool DissolveWindow::checkSaveCurrentInput()
{
    // First, check the modification status of the current session
    if (modified_)
    {
        QMessageBox queryBox;
        queryBox.setText("The current input file is unsaved.");
        queryBox.setInformativeText("Would you like to save it first?");
        queryBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        queryBox.setDefaultButton(QMessageBox::Cancel);
        auto ret = queryBox.exec();

        if (ret == QMessageBox::Cancel)
            return false;
        else if (ret == QMessageBox::Yes)
        {
            // If an input filename has been set, just overwrite it. If not, request a name first
            if (!dissolve_.hasInputFilename())
            {
                QString newFile = QFileDialog::getSaveFileName(this, "Choose input file name to save", QDir().absolutePath(),
                                                               "Dissolve input files (*.txt)");
                if (newFile.isEmpty())
                    return false;

                dissolve_.setInputFilename(qPrintable(newFile));
            }

            // Save the file
            if (!dissolve_.saveInput(dissolve_.inputFilename()))
                return false;

            modified_ = false;
        }
    }

    // All OK
    return true;
}

// Clear all data and start new simulation afresh
void DissolveWindow::startNew()
{
    // Clear any data-related tabs from the UI
    ui_.MainTabs->clearTabs();

    // Clear Dissolve itself
    dissolve_.clear();

    dissolveState_ = DissolveWindow::EditingState;
    localSimulation_ = true;
    modified_ = false;

    // Fully update GUI
    fullUpdate();
}

void DissolveWindow::on_FileNewAction_triggered(bool checked)
{
    if (!checkSaveCurrentInput())
        return;

    startNew();
}

void DissolveWindow::on_FileOpenLocalAction_triggered(bool checked)
{
    if (!checkSaveCurrentInput())
        return;

    // Request a new file to open
    QString inputFile =
        QFileDialog::getOpenFileName(this, "Choose input file to open", QDir().absolutePath(), "Dissolve input files (*.txt)");
    if (inputFile.isEmpty())
        return;

    openLocalFile(qPrintable(inputFile), "", false, false);
}

void DissolveWindow::on_FileConnectAction_triggered(bool checked)
{
    // TODO
}

void DissolveWindow::on_FileOpenRecentAction_triggered(bool checked)
{
    // TODO
}

void DissolveWindow::on_FileCloseAction_triggered(bool checked)
{
    if (!checkSaveCurrentInput())
        return;

    // Clear any data-related tabs from the UI
    ui_.MainTabs->clearTabs();

    // Clear and fully update GUI
    dissolve_.clear();
    dissolveState_ = DissolveWindow::NoState;
    modified_ = false;
    fullUpdate();
}

void DissolveWindow::on_FileSaveAction_triggered(bool checked)
{
    // If the file is not modified, nothing to do.
    if (!modified_)
        return;

    // If an input file name has not been set, get one now
    if (!dissolve_.hasInputFilename())
    {
        QString newFile = QFileDialog::getSaveFileName(this, "Choose input file name to save", QDir().absolutePath(),
                                                       "Dissolve input files (*.txt)");
        if (newFile.isEmpty())
            return;

        dissolve_.setInputFilename(qPrintable(newFile));
    }

    // Attempt to save the file
    if (!dissolve_.saveInput(dissolve_.inputFilename()))
        return;

    modified_ = false;

    updateStatusBar();

    updateWindowTitle();
}

void DissolveWindow::on_FileSaveAsAction_triggered(bool checked)
{
    // Get a suitable input file name
    QString newFile = QFileDialog::getSaveFileName(this, "Choose input file name to save", QDir().absolutePath(),
                                                   "Dissolve input files (*.txt)");
    if (newFile.isEmpty())
        return;

    dissolve_.setInputFilename(qPrintable(newFile));

    // Attempt to save the file
    if (!dissolve_.saveInput(dissolve_.inputFilename()))
        return;

    modified_ = false;

    // Update the current working directory to be local to the new input file
    QFileInfo inputFileInfo(newFile);
    QDir::setCurrent(inputFileInfo.absoluteDir().absolutePath());

    updateStatusBar();

    updateWindowTitle();
}

void DissolveWindow::on_FileQuitAction_triggered(bool checked)
{
    if (!checkSaveCurrentInput())
        return;

    close();
}
