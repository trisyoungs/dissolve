// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Team Dissolve and contributors

#include "gui/datamanagerdialog.h"
#include "gui/helpers/tablewidgetupdater.h"
#include "main/dissolve.h"
#include "templates/variantpointer.h"
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QRegularExpression>

DataManagerDialog::DataManagerDialog(QWidget *parent, Dissolve &dissolve, std::vector<ReferencePoint> &referencePoints,
                                     GenericList &items)
    : QDialog(parent), dissolve_(dissolve), referencePoints_(referencePoints), refModel_(dissolve, referencePoints_),
      simModel_(dissolve, items)
{
    ui_.setupUi(this);
    ui_.ReferencePointsTable->setModel(&refModel_);

    simProxy_.setSourceModel(&simModel_);
    ui_.SimulationDataTable->setModel(&simProxy_);
    ui_.SimulationDataTable->setSortingEnabled(true);

    updateControls();
}

DataManagerDialog::~DataManagerDialog() {}

/*
 * UI
 */

// Update the specified table of GenericItems, optionally filtering them by name and description
void DataManagerDialog::filterTable(QString filterText)
{
    simProxy_.setFilterRegularExpression(
        QRegularExpression(filterText.replace("*", ".*"), QRegularExpression::CaseInsensitiveOption));
    simProxy_.setFilterKeyColumn(0);
}

// Return currently-selected ReferencePoint
ReferencePoint *DataManagerDialog::currentReferencePoint() const
{
    // Get current item from tree, and check the parent item
    auto index = ui_.ReferencePointsTable->currentIndex();
    if (!index.isValid())
        return nullptr;
    return VariantPointer<ReferencePoint>(refModel_.data(index, Qt::UserRole));
}

// Update controls
void DataManagerDialog::updateControls()
{
    // Clear and re-populate simulation data table
    ui_.SimulationDataTable->resizeColumnsToContents();

    // Populate reference points table
    refModel_.update();
    ui_.ReferencePointsTable->resizeColumnsToContents();
}

// Simulation Data
void DataManagerDialog::on_SimulationDataFilterEdit_textChanged(const QString &text) { filterTable(text); }

// Refernce Points
void DataManagerDialog::on_ReferencePointRemoveButton_clicked(bool checked)
{
    ReferencePoint *refPoint = currentReferencePoint();
    if (!refPoint)
        return;

    dissolve_.processingModuleData().pruneWithSuffix(std::get<0>(*refPoint));

    updateControls();
}

void DataManagerDialog::on_ReferencePointOpenButton_clicked(bool checked)
{
    // Get file to open
    QString restartFile =
        QFileDialog::getOpenFileName(this, "Choose restart file to open", QDir().absolutePath(), "Restart Files (*.restart)");
    if (restartFile.isEmpty())
        return;

    // Get suffix for data - base this on the filename
    QString suffix = QFileInfo(restartFile).baseName();
    bool ok;
    suffix = QInputDialog::getText(this, "Reference Point Suffix", "Enter suffix to apply to the reference point data",
                                   QLineEdit::Normal, suffix, &ok);
    if (!ok)
        return;

    auto err = refModel_.addFile(qPrintable(suffix), qPrintable(restartFile));
    if (err)
        QMessageBox::warning(this, "Error loading reference point", QString((*err).c_str()));

    updateControls();
}

void DataManagerDialog::on_ReferencePointCreateButton_clicked(bool checked)
{
    // Get filename for reference point
    QString filename =
        QFileDialog::getSaveFileName(this, "Select Reference Point File", QDir::currentPath(), "Restart Files (*.restart)");
    if (filename.isEmpty())
        return;

    // Get suffix for data - base this on the filename
    QString suffix = QFileInfo(filename).baseName();
    bool ok;
    suffix = QInputDialog::getText(this, "Reference Point Suffix", "Enter suffix to apply to the reference point data",
                                   QLineEdit::Normal, suffix, &ok);
    if (!ok)
        return;

    if (dissolve_.saveRestart(qPrintable(filename)))
        Messenger::print("Saved reference point to '{}'.\n", qPrintable(filename));
    else
    {
        Messenger::error("Failed to save reference point to '{}'.\n", qPrintable(filename));
        return;
    }

    auto err = refModel_.addFile(qPrintable(suffix), qPrintable(filename));
    if (err)
        QMessageBox::warning(this, "Error loading reference point", QString((*err).c_str()));

    updateControls();
}

// Dialog
void DataManagerDialog::on_CloseButton_clicked(bool checked) { accept(); }
