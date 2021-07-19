// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Team Dissolve and contributors

#include "data/elements.h"
#include "gui/widgets/elementselector.hui"
#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>

ElementSelector::ElementSelector(QWidget *parent) : QWidget(parent)
{
    currentElement_ = Elements::Unknown;

    // Create grid layout for widget
    auto *gl = new QGridLayout;
    gl->setSpacing(4);

    // Create periodic table buttons
    QLabel *label;
    int n, m, z;

    // Create element button array (and buttons)
    QToolButton *button;
    QPalette palette = this->palette();
    QColor background, foreground;
    for (n = 0; n < Elements::nElements; ++n)
    {
        button = new QToolButton(this);
        button->setAutoRaise(true);
        button->setCheckable(true);
        button->setAutoExclusive(true);
        buttons_.push_back(button);
        button->setText(QString::fromStdString(std::string(Elements::symbol(Elements::element(n)))));
        button->setMinimumSize(24, 24);
        button->setToolTip(QString("%1 (%2)").arg(QString::fromStdString(std::string(Elements::name(Elements::element(n)))),
                                                  QString::fromStdString(std::string(Elements::symbol(Elements::element(n))))));

        QObject::connect(button, SIGNAL(clicked(bool)), this, SLOT(elementButtonClicked(bool)));
    }

    // Now add widgets to gridlayout
    // First row - Group Number
    for (n = 1; n < 19; n++)
    {
        label = new QLabel(this);
        label->setText(QString::number(n));
        gl->addWidget(label, 0, n);
    }

    // First column - Period number
    for (n = 1; n < 8; n++)
    {
        label = new QLabel(this);
        label->setText(QString::number(n));
        gl->addWidget(label, n, 0);
    }

    // H, He
    gl->addWidget(buttons_[1], 1, 1);
    gl->addWidget(buttons_[2], 1, 18);

    // Groups 1-2 (periods 1-6) [s]
    z = 3;
    for (n = 0; n < 6; n++)
    {
        gl->addWidget(buttons_[z], n + 2, 1);
        gl->addWidget(buttons_[z + 1], n + 2, 2);
        z += 8;
        if (n > 1)
            z += 10;
        if (n > 3)
            z += 14;
    }

    // Groups 13-18 (periods 1-6) [p]
    z = 5;
    for (n = 0; n < 6; n++)
    {
        for (m = 0; m < 6; m++)
            gl->addWidget(buttons_[z + m], n + 2, 13 + m);
        z += 8;
        if (n > 0)
            z += 10;
        if (n > 2)
            z += 14;
    }

    // Groups 3-8 (periods 3-6) [p]
    z = 21;
    for (n = 0; n < 4; n++)
    {
        for (m = 0; m < 10; m++)
            gl->addWidget(buttons_[z + m], n + 4, 3 + m);
        if (n == 1)
            z += 14;
        z += 18;
        if (n > 1)
            z += 14;
    }

    label = new QLabel(this);
    label->setText("Ln");
    gl->addWidget(label, 9, 0);
    label = new QLabel(this);
    label->setText("An");
    gl->addWidget(label, 10, 0);

    // Lanthanoids and Actinoids
    z = 57;
    for (n = 0; n < 14; n++)
    {
        gl->addWidget(buttons_[z + n], 9, 3 + n);
        gl->addWidget(buttons_[z + n + 32], 10, 3 + n);
    }

    // Special elements
    gl->addWidget(buttons_[0], 1, 19);
    buttons_[0]->setVisible(false);

    setLayout(gl);

    // Set up the timer
    doubleClickTimer_.setSingleShot(true);
    doubleClickTimer_.setInterval(QApplication::doubleClickInterval());
}

ElementSelector::~ElementSelector() {}

/*
 * Widget Functions
 */

// Element button clicked
void ElementSelector::elementButtonClicked(bool checked)
{
    // Cast sender
    auto *button = qobject_cast<QToolButton *>(sender());
    if (!button)
        currentElement_ = Elements::Unknown;

    currentElement_ = Elements::element(qPrintable(button->text()));

    // Was this a double-click? Check the timer
    if (doubleClickTimer_.isActive())
        emit(elementDoubleClicked());
    else
    {
        doubleClickTimer_.start();
        emit(elementSelectionChanged());
        emit(elementSelected(true));
    }
}

/*
 * Data
 */

// Set current element
void ElementSelector::setCurrentElement(Elements::Element Z)
{
    // Uncheck any current element button before we change
    if (currentElement_)
    {
        auto *button = buttons_[currentElement_];
        if (button)
            button->setChecked(false);
    }

    currentElement_ = Z;

    // Find and check the related button
    if (currentElement_ != Elements::Unknown)
    {
        QToolButton *button = buttons_[currentElement_];
        if (button)
            button->setChecked(true);
    }
}

// Return current element
Elements::Element ElementSelector::currentElement() const { return currentElement_; }

/*
 * Static Functions
 */

// Get Element from user via input dialog
Elements::Element ElementSelector::getElement(QWidget *parent, QString title, QString labelText, Elements::Element Z, bool *ok,
                                              Qt::WindowFlags flags)
{
    // Create a QDialog for use
    QDialog inputDialog(parent, flags);
    inputDialog.setWindowTitle(title);

    // Create the label
    QLabel *label = new QLabel(labelText, &inputDialog);
    label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    // Create ElementSelector widget
    auto *elementSelector = new ElementSelector(&inputDialog);
    elementSelector->setCurrentElement(Z);
    QObject::connect(elementSelector, SIGNAL(elementDoubleClicked()), &inputDialog, SLOT(accept()));

    // Create button box
    QDialogButtonBox *buttonBox =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &inputDialog);
    QObject::connect(buttonBox, SIGNAL(accepted()), &inputDialog, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), &inputDialog, SLOT(reject()));
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(Z != Elements::Unknown);
    QObject::connect(elementSelector, SIGNAL(elementSelected(bool)), buttonBox->button(QDialogButtonBox::Ok),
                     SLOT(setEnabled(bool)));

    auto *mainLayout = new QVBoxLayout(&inputDialog);
    mainLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    mainLayout->addWidget(label);
    mainLayout->addWidget(elementSelector);
    mainLayout->addWidget(buttonBox);

    const auto ret = inputDialog.exec();
    if (ok)
        *ok = (ret == QDialog::Accepted);
    return (ret == QDialog::Accepted ? elementSelector->currentElement() : Z);
}
