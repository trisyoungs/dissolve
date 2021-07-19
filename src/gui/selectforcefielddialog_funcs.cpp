// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Team Dissolve and contributors

#include "data/ff/ff.h"
#include "gui/selectforcefielddialog.h"
#include "gui/selectforcefieldwidget.h"
#include "templates/variantpointer.h"

SelectForcefieldDialog::SelectForcefieldDialog(QWidget *parent, const List<Forcefield> &forcefields) { ui_.setupUi(this); }

SelectForcefieldDialog::~SelectForcefieldDialog() {}

void SelectForcefieldDialog::on_ForcefieldWidget_forcefieldSelectionChanged(bool isValid)
{
    ui_.SelectButton->setEnabled(isValid);
}

void SelectForcefieldDialog::on_ForcefieldWidget_forcefieldDoubleClicked()
{
    if (!ui_.ForcefieldWidget->currentForcefield())
        return;

    accept();
}

void SelectForcefieldDialog::on_SelectButton_clicked(bool checked) { accept(); }

void SelectForcefieldDialog::on_CancelButton_clicked(bool checked) { reject(); }

// Run the dialog, returning the selected Forcefield
std::shared_ptr<Forcefield> SelectForcefieldDialog::selectForcefield(std::shared_ptr<Forcefield> currentFF)
{
    ui_.ForcefieldWidget->setCurrentForcefield(currentFF);

    show();

    if (exec() == QDialog::Accepted)
        return ui_.ForcefieldWidget->currentForcefield();
    else
        return nullptr;
}
