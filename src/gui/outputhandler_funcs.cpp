// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Team Dissolve and contributors

#include "gui/outputhandler.hui"
#include <QColor>
#include <QGuiApplication>
#include <QPalette>

/*
 * Styling
 */

// Set styling for warning message
void GUIOutputHandler::styleForWarning() { emit(setColour(Qt::blue)); }

// Set styling for error message
void GUIOutputHandler::styleForError() { emit(setColour(Qt::red)); }

// Reset styling for normal text
void GUIOutputHandler::resetStyling() { emit(setColour(QGuiApplication::palette().text().color())); }

/*
 * Output
 */

// Print text
void GUIOutputHandler::outputText(std::string s) { emit(printText(QStringLiteral("%1\n").arg(s.c_str()))); }
