// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Team Dissolve and contributors

#include "gui/charts/proceduremetrics.h"
#include "gui/charts/procedurenode.h"
#include "procedure/nodes/node.h"
#include "templates/variantpointer.h"
#include <QPainter>
#include <QPainterPath>

ProcedureChartNodeBlock::ProcedureChartNodeBlock(QWidget *parent, ProcedureNode *node, const CoreData &coreData)
    : QWidget(parent), ChartBlock()
{
    // Set up user interface
    ui_.setupUi(this);

    refreshing_ = false;

    // Set necessary values on the widget itself
    ProcedureChartMetrics metrics;
    setContentsMargins(metrics.blockMargins());
    dragHandleRect_ = QRect(0, 0, metrics.dragHandleWidth(), 1);

    // Set the node pointer
    node_ = node;

    // Hide the keywords control frame to start with
    ui_.KeywordsControlWidget->setVisible(false);

    // Hide the name label if it is not required
    if (!node_->mustBeNamed())
        ui_.BottomLabel->setVisible(false);

    // Set up our keywords widget
    ui_.NodeKeywordsWidget->setUp(node->keywords(), coreData);
    connect(ui_.NodeKeywordsWidget, SIGNAL(dataModified()), this, SLOT(keywordDataModified()));

    // Update our controls
    ui_.TopLabel->setText(QString::fromStdString(std::string(ProcedureNode::nodeTypes().keyword(node_->type()))));
    updateControls();

    updateGeometry();
}

/*
 * Node Target
 */

// Return displayed node
ProcedureNode *ProcedureChartNodeBlock::node() const { return node_; }

/*
 * Controls
 */

// Set display colour for widget
void ProcedureChartNodeBlock::setDisplayColour(QColor colour) { displayColour_ = colour; }

// Set whether the keywords widget is expanded or not, and whether this is permanent
void ProcedureChartNodeBlock::setKeywordsExpanded(bool expanded, bool permanent)
{
    on_ToggleKeywordsButton_clicked(expanded);

    ui_.ToggleKeywordsButton->setDisabled(permanent);
}

// Hide the remove button
void ProcedureChartNodeBlock::hideRemoveButton() { ui_.RemoveButton->setVisible(false); }

// Return RefList of widgets that exist in the branch of our Procedure node
RefList<ProcedureChartNodeBlock> &ProcedureChartNodeBlock::branchWidgets() { return branchWidgets_; }

void ProcedureChartNodeBlock::on_ToggleKeywordsButton_clicked(bool checked)
{
    ui_.KeywordsControlWidget->setVisible(checked);

    adjustSize();
    updateGeometry();

    emit(keywordsToggled());
}

void ProcedureChartNodeBlock::on_RemoveButton_clicked(bool checked) { emit(remove(node_)); }

/*
 * QWidget Reimplementations
 */

// Paint event
void ProcedureChartNodeBlock::paintEvent(QPaintEvent *event)
{
    if (!node_)
        return;

    ProcedureChartMetrics metrics;

    QPainter painter(this);

    // Set up the basic pen
    QPen pen;
    pen.setWidth(metrics.blockBorderWidth());
    pen.setColor(displayColour_);
    painter.setPen(pen);

    QPainterPath borderPath;
    borderPath.moveTo(metrics.blockBorderMidPoint(), metrics.blockBorderMidPoint());
    borderPath.lineTo(metrics.blockBorderMidPoint(), height() - metrics.blockBorderMidPoint());
    borderPath.lineTo(width() - metrics.blockBorderWidth(), height() - metrics.blockBorderMidPoint());
    borderPath.lineTo(width() - metrics.blockBorderWidth(), metrics.blockBorderMidPoint());
    borderPath.closeSubpath();

    // Ready - draw the border + fill!
    painter.setBrush(QGuiApplication::palette().base());
    painter.drawPath(borderPath);

    // Draw the drag handle, updating its height first
    painter.setBrush(displayColour_);
    pen.setWidth(1);
    painter.setPen(pen);
    dragHandleRect_.setHeight(height());
    painter.drawRect(dragHandleRect_);
}

/*
 * Type (ChartBlock Reimplementations)
 */

// Return type of this block
const QString ProcedureChartNodeBlock::blockType() const { return "Node"; }

/*
 * Widget (ChartBlock Reimplementations)
 */

// Return underlying widget
QWidget *ProcedureChartNodeBlock::widget() { return this; }

// Return width of underlying widget
int ProcedureChartNodeBlock::widgetWidth() const { return sizeHint().width(); }

// Return height of underlying widget
int ProcedureChartNodeBlock::widgetHeight() const { return sizeHint().height(); }

// Return whether the supplied point (on the parent chart) allows a drag operation to begin
bool ProcedureChartNodeBlock::isDragPoint(QPoint point) const
{
    if (dragHandleRect_.translated(geometry().left(), geometry().top()).contains(point))
        return true;

    return false;
}

/*
 * Update (ChartBlock Reimplementations)
 */

// Update controls within widget
void ProcedureChartNodeBlock::updateControls()
{
    if (!node_)
        return;

    refreshing_ = true;

    // Set information panel contents
    ui_.BottomLabel->setText(QString::fromStdString(std::string(node_->name())));

    // Update keywords
    ui_.NodeKeywordsWidget->updateControls();

    refreshing_ = false;
}

// Disable sensitive controls
void ProcedureChartNodeBlock::disableSensitiveControls()
{
    ui_.KeywordsControlWidget->setEnabled(false);
    ui_.RemoveButton->setEnabled(false);
}

// Enable sensitive controls
void ProcedureChartNodeBlock::enableSensitiveControls()
{
    ui_.KeywordsControlWidget->setEnabled(true);
    ui_.RemoveButton->setEnabled(true);
}

/*
 * Signals / Slots
 */

// Keyword data for node has been modified
void ProcedureChartNodeBlock::keywordDataModified() { emit(dataModified()); }
