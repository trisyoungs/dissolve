// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Team Dissolve and contributors

#include "gui/charts/modulelist.h"
#include "gui/charts/moduleblock.h"
#include "gui/charts/moduleinsertionblock.h"
#include "gui/charts/modulelistmetrics.h"
#include "gui/widgets/mimestrings.h"
#include "main/dissolve.h"
#include "module/list.h"
#include "module/module.h"
#include <QApplication>
#include <QDrag>
#include <QMessageBox>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QWidget>

ModuleListChart::ModuleListChart(ModuleList *moduleList, Dissolve &dissolve, Configuration *localConfiguration)
    : ChartBase(), dissolve_(dissolve)
{
    refreshing_ = false;

    // Target ModuleLayer
    moduleList_ = moduleList;
    localConfiguration_ = localConfiguration;

    // Create the insertion widget if we don't already have one
    insertionBlock_ = new ModuleInsertionBlock(this);
    insertionBlock_->setVisible(false);

    updateContentBlocks();

    recalculateLayout();

    updateControls();
}

/*
 * QWidget Reimplementations
 */

// Paint event
void ModuleListChart::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    // Draw the background before we do anything else
    painter.fillRect(QRect(0, 0, width(), height()), QGuiApplication::palette().base());
    painter.drawPixmap(0, 0, QPixmap(":/images/images/cornerhexagons.png"));

    // Set up some QPens
    QPen solidPen(Qt::black);
    solidPen.setWidth(metrics_.blockBorderWidth());

    // Create a metrics object
    ModuleListChartMetrics metrics;

    // Draw lines between module widgets
    painter.setPen(solidPen);
    QPoint p1, p2;
    auto top = 0;
    for (ModuleBlock *block : moduleBlockWidgets_)
    {
        // If this block is not visible, continue
        if (!block->isVisible())
            continue;

        // Draw connecting line between blocks
        p1 = QPoint(width() / 2, top);
        p2 = QPoint(width() / 2, block->geometry().top());
        painter.drawLine(p1, p2);
        painter.setBrush(Qt::black);
        painter.drawEllipse(p2, metrics.blockDentRadius() - metrics.blockBorderWidth() - 1,
                            metrics.blockDentRadius() - metrics.blockBorderWidth() - 1);

        top = block->geometry().bottom();
    }

    // Is there a current selected block?
    if (selectedBlock_)
    {
        // Cast up the selectedBlock_ to a ModuleBlock
        auto *selectedModule = dynamic_cast<ModuleBlock *>(selectedBlock_);
        if ((selectedModule) && (moduleBlockWidgets_.contains(selectedModule)))
        {
            QRect rect = selectedModule->geometry();
            rect.adjust(-metrics.chartMargin(), -15, metrics.chartMargin(), 15);
            painter.fillRect(rect, QColor(49, 0, 73, 80));
        }
        else
            selectedBlock_ = nullptr;
    }

    // Highlight all hotspots
    if (false)
    {
        ListIterator<ChartHotSpot> hotSpotIterator(hotSpots_);
        while (ChartHotSpot *hotSpot = hotSpotIterator.iterate())
            painter.fillRect(hotSpot->geometry(), QBrush(QColor(200, 200, 0, 50)));
    }
}

/*
 * Chart Blocks
 */

// Find ModuleBlock displaying specified Module
ModuleBlock *ModuleListChart::moduleBlock(Module *module)
{
    for (ModuleBlock *block : moduleBlockWidgets_)
        if (block->module() == module)
            return block;

    return nullptr;
}

// Update the content block widgets against the current target data
void ModuleListChart::updateContentBlocks()
{
    if (!moduleList_)
        return;

    // Create a temporary list that will store our widgets to be 'reused'
    RefList<ModuleBlock> newWidgets;

    // Iterate through the nodes in this sequence, searching for their widgets in the oldWidgetsList
    ListIterator<Module> moduleIterator(moduleList_->modules());
    while (Module *module = moduleIterator.iterate())
    {
        // Does this Module have an existing widget?
        ModuleBlock *block = moduleBlock(module);
        if (block)
        {
            // Widget already exists, so remove the reference from nodeWidgets_ and add it to the new list
            newWidgets.append(block);
            moduleBlockWidgets_.remove(block);
        }
        else
        {
            // No current widget, so must create one
            block = new ModuleBlock(this, module, dissolve_);
            connect(block, SIGNAL(dataModified()), this, SLOT(chartDataModified()));
            connect(block, SIGNAL(remove(const QString &)), this, SLOT(blockRemovalRequested(const QString &)));
            newWidgets.append(block);
            chartBlocks_.append(block);
        }
    }

    // Any widgets remaining in moduleBlockWidgets_ are no longer used, and can thus be deleted
    for (ModuleBlock *block : moduleBlockWidgets_)
    {
        chartBlocks_.remove(block);
        delete block;
    }

    // Copy the new list
    moduleBlockWidgets_ = newWidgets;

    // Set the correct number of hotspots (number of block widgets + 1)
    auto nHotSpots = moduleBlockWidgets_.nItems() + 1;
    while (nHotSpots < hotSpots_.nItems())
        hotSpots_.removeLast();
    while (nHotSpots > hotSpots_.nItems())
        hotSpots_.add();
}

// Set the currently-selected Module
void ModuleListChart::setCurrentModule(Module *module)
{
    if (!module)
        selectedBlock_ = nullptr;
    else
        selectedBlock_ = moduleBlock(module);

    repaint();

    blockSelectionChanged(selectedBlock_);
}

// Return the currently-selected molecule
Module *ModuleListChart::currentModule() const
{
    if (!selectedBlock_)
        return nullptr;

    // Cast selectedBlock_ up to a ModuleBlock
    auto *moduleBlock = dynamic_cast<ModuleBlock *>(selectedBlock_);
    if (!moduleBlock)
        return nullptr;

    return moduleBlock->module();
}

/*
 * Block Interaction
 */

// Return whether to accept the dragged object (described by its mime info)
bool ModuleListChart::acceptDraggedObject(const MimeStrings *strings)
{
    // Check the content of the strings
    if (strings->hasData(MimeString::ModuleType))
    {
        // We accept the drop of an object specifying a Module type - we'll create a new instance of this type in the
        // list
        return true;
    }
    else if (strings->hasData(MimeString::LocalType))
    {
        // We accept the drop of a local block (a Module, referenced by its unique name) - we'll move it into the list
        return true;
    }

    return false;
}

// Handle hover over specified hotspot, returning whether layout update is required
bool ModuleListChart::handleHotSpotHover(const ChartHotSpot *hotSpot)
{
    // Need to recalculate widgets in order to display hotspot drop area
    return true;
}

// Handle the drop of an object (described by its mime info)
void ModuleListChart::handleDroppedObject(const MimeStrings *strings)
{
    // Check - if there is no current hotspot, then we have nothing to do
    if (!currentHotSpot_)
        return;

    if (strings->hasData(MimeString::LocalType))
    {
        // Local data - i.e. the dragged block which originated from this chart
        if (!draggedBlock_)
        {
            Messenger::error("Local data dropped, but no dragged block is set.\n");
            return;
        }

        // Cast the dragged block up to a ModuleBlock
        auto *draggedModuleBlock = dynamic_cast<ModuleBlock *>(draggedBlock_);
        if (!draggedModuleBlock)
            return;

        // Get the Module associated to the dragged block
        Module *draggedModule = draggedModuleBlock->module();

        // Cast the blocks either side of the current hotspot up to ModuleBlocks, and get their Modules
        auto *moduleBlockBefore = dynamic_cast<ModuleBlock *>(currentHotSpot_->blockBefore());
        Module *moduleBeforeHotSpot = (moduleBlockBefore ? moduleBlockBefore->module() : nullptr);
        auto *moduleBlockAfter = dynamic_cast<ModuleBlock *>(currentHotSpot_->blockAfter());
        Module *moduleAfterHotSpot = (moduleBlockAfter ? moduleBlockAfter->module() : nullptr);

        // Check the blocks either side of the hotspot to see where our Module needs to be (or has been returned to)
        if ((draggedModule->prev() == moduleBeforeHotSpot) && (draggedModule->next() == moduleAfterHotSpot))
        {
            // Dragged block has not moved. Nothing to do.
            return;
        }
        else
        {
            // Block has been dragged to a new location...
            if (moduleBeforeHotSpot)
                moduleList_->modules().moveAfter(draggedModule, moduleBeforeHotSpot);
            else
                moduleList_->modules().moveBefore(draggedModule, moduleAfterHotSpot);

            // Flag that the current data has changed
            emit(dataModified());
        }
    }
    else if (strings->hasData(MimeString::ModuleType))
    {
        // Create a new instance of the specified module type
        Module *newModule = dissolve_.createModuleInstance(strings->data(MimeString::ModuleType));

        // Cast necessary blocks around the current hotspot up to ModuleBlocks, and get their Modules
        auto *moduleBlockAfter = dynamic_cast<ModuleBlock *>(currentHotSpot_->blockAfter());
        Module *moduleAfterHotSpot = (moduleBlockAfter ? moduleBlockAfter->module() : nullptr);

        // Add the new modele
        if (moduleAfterHotSpot)
            moduleList_->modules().ownBefore(newModule, moduleAfterHotSpot);
        else
            moduleList_->modules().own(newModule);

        newModule->setConfigurationLocal(localConfiguration_ != nullptr);

        // Set Configuration targets as appropriate
        if (newModule->nRequiredTargets() != Module::ZeroTargets)
        {
            if (localConfiguration_)
                newModule->addTargetConfiguration(localConfiguration_);
            else
                newModule->addTargetConfigurations(dissolve_.configurations());
        }

        // Flag that the current data has changed
        emit(dataModified());
    }
}

// Return mime info for specified block (owned by this chart)
MimeStrings ModuleListChart::mimeInfo(ChartBlock *block)
{
    // Try to cast the block into a ModuleBlock
    auto *moduleBlock = dynamic_cast<ModuleBlock *>(block);
    if (!moduleBlock)
        return MimeStrings();

    MimeStrings mimeStrings;
    mimeStrings.add(MimeString::LocalType, moduleBlock->module()->uniqueName());

    return mimeStrings;
}

// Specified block has been double clicked
void ModuleListChart::blockDoubleClicked(ChartBlock *block)
{
    // Cast block to a ModuleBlock
    auto *moduleBlock = dynamic_cast<ModuleBlock *>(block);
    if (!moduleBlock)
        return;

    // Emit the relevant signal
    emit(ChartBase::blockDoubleClicked(QString::fromStdString(std::string(moduleBlock->module()->uniqueName()))));
}

// The chart has requested removal of one of its blocks
void ModuleListChart::blockRemovalRequested(const QString &blockIdentifier)
{
    // Get the reference to the Module list
    List<Module> &modules = moduleList_->modules();

    // Find the named Module in our list
    Module *module = moduleList_->find(qPrintable(blockIdentifier));
    if (!module)
    {
        Messenger::error("Can't find module to remove ({}) in our target list!\n", qPrintable(blockIdentifier));
        return;
    }

    // Are we sure that we want to delete the Module?
    QMessageBox queryBox;
    queryBox.setText(QString("This will delete the Module '%1'.").arg(blockIdentifier));
    queryBox.setInformativeText("This cannot be undone. Proceed?");
    queryBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    queryBox.setDefaultButton(QMessageBox::No);
    auto ret = queryBox.exec();

    if (ret == QMessageBox::Yes)
    {
        modules.cut(module);

        // Notify that we are removing this Module/block
        emit(blockRemoved(blockIdentifier));

        // If the module to delete is the currently-displayed one, unset it now
        if (module == currentModule())
            setCurrentModule(nullptr);

        // Remove the Module instance
        dissolve_.deleteModuleInstance(module);

        emit(dataModified());
        emit(fullUpdate());

        updateControls();
    }
}

// Block selection has changed
void ModuleListChart::blockSelectionChanged(ChartBlock *block)
{
    // If a NULL block pointer was provided there is no current selection
    if (!block)
    {
        emit(ChartBase::blockSelectionChanged(QString()));
        return;
    }

    // Cast block to a ModuleBlock
    auto *moduleBlock = dynamic_cast<ModuleBlock *>(block);
    if (!moduleBlock)
        return;

    // Emit the relevant signal
    emit(ChartBase::blockSelectionChanged(QString::fromStdString(std::string(moduleBlock->module()->uniqueName()))));
}

/*
 * Widget Layout
 */

// Calculate new widget geometry according to the layout requirements
QSize ModuleListChart::calculateNewWidgetGeometry(QSize currentSize)
{
    /*
     * ModuleList layout is a single vertical column.
     */

    // Create a metrics object
    ModuleListChartMetrics metrics;

    // Left edge of next widget, and maximum height
    auto top = metrics.chartMargin() - metrics.verticalModuleSpacing();
    auto hotSpotTop = 0;
    auto maxWidth = 0;

    // Get the first hot spot in the list (the list should have been made the correct size in updateContentBlocks()).
    ChartHotSpot *hotSpot = hotSpots_.first();

    // Loop over widgets
    ModuleBlock *lastVisibleBlock = nullptr;
    for (ModuleBlock *block : moduleBlockWidgets_)
    {
        // Set default visibility of the block
        block->setVisible(true);

        // If this block is currently being dragged, hide it and continue with the next one
        if (draggedBlock_ == block)
        {
            block->setVisible(false);
            continue;
        }

        // Add vertical spacing
        top += metrics.verticalModuleSpacing();

        // If our hotspot is the current one, increase the size.
        if (hotSpot == currentHotSpot_)
            top += metrics.verticalInsertionSpacing();

        // Set top edge of this widget
        block->setNewPosition(metrics.chartMargin(), top);

        // Try to give our block its preferred (minimum) size
        QSize minSize = block->widget()->minimumSizeHint();
        block->setNewSize(minSize.width(), minSize.height());

        // Set the hotspot to end at the left edge of the current block
        hotSpot->setGeometry(QRect(0, hotSpotTop, width(), top - hotSpotTop));

        // Set surrounding blocks for the hotspot
        hotSpot->setSurroundingBlocks(lastVisibleBlock, block);

        // Set new hotspot top edge
        hotSpotTop = top + minSize.height();

        // Add on height of widget and spacing to top edge
        top += minSize.height();

        // Check maximal width
        if (minSize.width() > maxWidth)
            maxWidth = minSize.width();

        // Set the last visible block
        lastVisibleBlock = block;

        // Move to the next hotspot
        hotSpot = hotSpot->next();
    }
    // Handle final block
    top -= metrics.verticalModuleSpacing();

    // Finalise required size
    QSize requiredSize = QSize(maxWidth + 2 * metrics.chartMargin(), top + metrics.chartMargin());

    // Set final hotspot geometry
    hotSpot->setGeometry(QRect(0, hotSpotTop, width(), height() - hotSpotTop));
    hotSpot->setSurroundingBlocks(lastVisibleBlock, nullptr);
    hotSpot = hotSpot->next();

    // Set the correct heights for all hotspots up to the current one - any after that are not required and will have zero
    // height
    for (auto *spot = hotSpot; spot != nullptr; spot = spot->next())
        spot->setHeight(0);

    // If there is a current hotspot, set the insertion widget to be visible and set its geometry
    if (currentHotSpot_)
    {
        insertionBlock_->setVisible(true);

        insertionBlock_->setGeometry(currentHotSpot_->geometry());
    }
    else
        insertionBlock_->setVisible(false);

    // Return required size
    return requiredSize;
}
