// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Team Dissolve and contributors

#include "gui/widgets/exponentialspin.hui"
#include "math/doubleexp.h"
#include <QLineEdit>

ExponentialSpin::ExponentialSpin(QWidget *parent) : QDoubleSpinBox(parent)
{
    // Set up validator
    validator_.setNotation(QDoubleValidator::ScientificNotation);
    lineEdit()->setValidator(&validator_);

    // Set local values
    limitMinValue_ = false;
    limitMaxValue_ = false;

    // Set QDoubleSpinBox limits to be present but effectively 'off', and number of decimals
    setMinimum(-1.0e99);
    setMaximum(1.0e99);
    setDecimals(3);
}

/*
 * Data
 */

// Set minimum limit
void ExponentialSpin::setMinimumLimit(double value)
{
    limitMinValue_ = true;

    setMinimum(value);
}

// Set minimum limit
void ExponentialSpin::setMaximumLimit(double value)
{
    limitMaxValue_ = true;

    setMaximum(value);
}

// Remove range limits
void ExponentialSpin::removeLimits()
{
    limitMinValue_ = false;
    limitMaxValue_ = false;
}

/*
 * Reimplementations
 */

// Convert supplied value to text
QString ExponentialSpin::textFromValue(double value) const
{
    const auto exponentFormatThreshold = 3;
    DoubleExp de(value);
    return QString::fromStdString(de.asString(exponentFormatThreshold, decimals()));
}

// Validate supplied text
QValidator::State ExponentialSpin::validate(QString &text, int &pos) const
{
    // Set validator
    static QRegularExpression regExp("[-+]?[0-9]*[.]?[0-9]+([eE][-+]?[0-9]+)?");
    return (regExp.match(text).hasMatch() ? QValidator::Acceptable : QValidator::Invalid);
}

// Interpret text into value
double ExponentialSpin::valueFromText(const QString &text) const { return text.toDouble(); }
