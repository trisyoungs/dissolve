// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Team Dissolve and contributors

#include "data/ff/ff.h"
#include "gui/selectconfigurationdialog.h"
#include "gui/selectconfigurationwidget.h"
#include "templates/variantpointer.h"

SelectConfigurationDialog::SelectConfigurationDialog(QWidget *parent, const CoreData &coreData, QString dialogTitle)
{
    ui_.setupUi(this);

    setWindowTitle(dialogTitle);

    ui_.ConfigurationWidget->setCoreData(&coreData);
}

SelectConfigurationDialog::~SelectConfigurationDialog() {}

void SelectConfigurationDialog::on_ConfigurationWidget_speciesSelectionChanged(bool isValid)
{
    ui_.SelectButton->setEnabled(isValid);
}

void SelectConfigurationDialog::on_ConfigurationWidget_speciesDoubleClicked()
{
    // Check current selection size for validity
    if (ui_.ConfigurationWidget->currentConfiguration().size() != 1)
        return;

    accept();
}

void SelectConfigurationDialog::on_SelectButton_clicked(bool checked) { accept(); }

void SelectConfigurationDialog::on_CancelButton_clicked(bool checked) { reject(); }

// Run the dialog, returning a single selected Configuration
Configuration *SelectConfigurationDialog::selectConfiguration()
{
    ui_.ConfigurationWidget->reset(1, 1);

    show();

    if (exec() == QDialog::Accepted)
        return ui_.ConfigurationWidget->currentConfiguration().front();
    else
        return nullptr;
}

// Run the dialog, returning a list of selected Configuration
std::vector<Configuration *> SelectConfigurationDialog::selectConfiguration(int minConfiguration, int maxConfiguration)
{
    ui_.ConfigurationWidget->reset(minConfiguration, maxConfiguration);

    show();

    if (exec() == QDialog::Accepted)
        return ui_.ConfigurationWidget->currentConfiguration();
    else
        return {};
}
