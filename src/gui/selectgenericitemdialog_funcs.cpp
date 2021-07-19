// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Team Dissolve and contributors

#include "gui/selectgenericitemdialog.h"
#include "main/dissolve.h"
#include <QRegularExpression>

SelectGenericItemDialog::SelectGenericItemDialog(QWidget *parent, Dissolve &dissolve) : dissolve_(dissolve)
{
    ui_.setupUi(this);

    connect(this, SIGNAL(genericItemSelectionChanged(bool)), ui_.SelectButton, SLOT(setEnabled(bool)));
}

SelectGenericItemDialog::~SelectGenericItemDialog() {}

// Update and (optionally) filter the list
void SelectGenericItemDialog::filterItems(QString filterText)
{
    // Loop over rows in the table
    for (auto n = 0; n < ui_.ItemsList->count(); ++n)
    {
        auto *item = ui_.ItemsList->item(n);
        if (!item)
            continue;

        // Check filtering
        if (filterText.isEmpty())
            item->setHidden(false);
        else
            item->setHidden(!item->text().contains(
                QRegularExpression(filterText.replace("*", ".*"), QRegularExpression::CaseInsensitiveOption)));
    }
}

void SelectGenericItemDialog::on_FilterEdit_textChanged(const QString &text) { filterItems(text); }

void SelectGenericItemDialog::on_ItemsList_currentItemChanged(QTableWidgetItem *currentItem, QTableWidgetItem *prevItem)
{
    if (currentItem)
        emit(genericItemSelectionChanged(true));
    else
        emit(genericItemSelectionChanged(false));
}

void SelectGenericItemDialog::on_ItemsList_itemDoubleClicked(QTableWidgetItem *w) { accept(); }

void SelectGenericItemDialog::on_SelectButton_clicked(bool checked) { accept(); }

void SelectGenericItemDialog::on_CancelButton_clicked(bool checked) { reject(); }
