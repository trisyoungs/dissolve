// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Team Dissolve and contributors

#include "gui/selectelementdialog.h"
#include "templates/variantpointer.h"

SelectElementDialog::SelectElementDialog(QWidget *parent) { ui_.setupUi(this); }

SelectElementDialog::~SelectElementDialog() {}

void SelectElementDialog::on_ElementWidget_elementSelected(bool dummy) { ui_.SelectButton->setEnabled(true); }

void SelectElementDialog::on_ElementWidget_elementDoubleClicked()
{
    if (!ui_.ElementWidget->currentElement())
        return;

    accept();
}

void SelectElementDialog::on_SelectButton_clicked(bool checked) { accept(); }

void SelectElementDialog::on_CancelButton_clicked(bool checked) { reject(); }

// Run the dialog, returning the selected Element
Elements::Element SelectElementDialog::selectElement(Elements::Element currentElement)
{
    ui_.ElementWidget->setCurrentElement(currentElement);

    show();

    if (exec() == QDialog::Accepted)
        return ui_.ElementWidget->currentElement();
    else
        return Elements::Unknown;
}
