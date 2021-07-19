// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Team Dissolve and contributors

#include "base/messenger.h"
#include "gui/dataviewer.hui"
#include "gui/gizmo.h"
#include "gui/render/renderabledata1d.h"
#include "gui/render/renderabledata2d.h"
#include "gui/render/renderabledata3d.h"
#include "gui/selectgenericitemdialog.h"
#include "io/export/data1d.h"
#include "io/export/data2d.h"
#include "io/export/data3d.h"
#include "math/data2d.h"
#include <QFileDialog>

/*
 * Private Functions
 */

// Show general context menu
void DataViewer::showGeneralContextMenu(QPoint pos)
{
    QMenu menu;

    // Reset View
    QAction *resetViewAction = menu.addAction("&Reset View");

    // Copy to clipboard
    QAction *copyToClipboardAction = menu.addAction("&Copy to clipboard");

    // Execute the menu
    QAction *selectedAction = menu.exec(mapToGlobal(pos));

    // Act on the action!
    if (selectedAction == resetViewAction)
        view_.resetViewMatrix();
    else if (selectedAction == copyToClipboardAction)
        copyViewToClipboard(true);
}

// Show renderable context menu
void DataViewer::showRenderableContextMenu(QPoint pos, std::shared_ptr<Renderable> renderable)
{
    QMenu menu;
    QAction *action;
    RefDataList<QAction, Gizmo *> copyToActions;
    QFont italicFont(menu.font());
    italicFont.setItalic(true);

    // Add header
    action = menu.addAction(QString::fromStdString(std::string(queryObjectInfo())));
    action->setEnabled(false);
    action->setFont(italicFont);
    menu.addSeparator();

    // -- Hide Renderable
    QAction *hideAction = menu.addAction("&Hide");

    // -- Save As...
    QAction *saveAsAction = menu.addAction("&Save as...");
    saveAsAction->setEnabled(renderable->type() >= Renderable::Data1DRenderable &&
                             renderable->type() <= Renderable::Data3DRenderable);

    // -- Copy To...
    QMenu *copyToMenu = menu.addMenu("&Copy to...");
    copyToMenu->setFont(menu.font());
    // Get list of viable destinations that will accept our data
    RefList<Gizmo> destinations =
        Gizmo::allThatAccept(QString::fromStdString(std::string(Renderable::renderableTypes().keyword(renderable->type()))));
    if (destinations.nItems() == 0)
        copyToMenu->setEnabled(false);
    else
    {
        for (Gizmo *destination : destinations)
        {
            action = copyToMenu->addAction(destination->uniqueName());
            copyToActions.append(action, destination);
        }
    }

    // -- Remove Renderable
    QAction *removeAction = nullptr;
    if (hasFlag(DataViewer::UserCanRemoveDataFlag))
    {
        menu.addSeparator();
        removeAction = menu.addAction("&Remove");
    }

    // Execute the menu
    QAction *selectedAction = menu.exec(mapToGlobal(pos));

    // Act on the action!
    if (selectedAction)
    {
        if (selectedAction == hideAction)
        {
            renderable->setVisible(false);
            emit(renderableChanged());
        }
        else if (selectedAction == saveAsAction)
        {
            // Get save file name
            QString filename =
                QFileDialog::getSaveFileName(this, "Select Exported Data File", QDir::currentPath(), "All Files (*.*)");
            if (!filename.isEmpty())
            {
                if (renderable->type() == Renderable::Data1DRenderable)
                {
                    Data1DExportFileFormat exportFormat(qPrintable(filename));
                    auto *r1d = static_cast<RenderableData1D *>(renderable.get());
                    if (!r1d->source())
                        Messenger::error("Failed to locate 1D data to export.\n");
                    else
                        exportFormat.exportData(r1d->source()->get());
                }
                else if (renderable->type() == Renderable::Data2DRenderable)
                {
                    Data2DExportFileFormat exportFormat(qPrintable(filename));
                    auto *r2d = static_cast<RenderableData2D *>(renderable.get());
                    if (!r2d->source())
                        Messenger::error("Failed to locate 2D data to export.\n");
                    else
                        exportFormat.exportData(r2d->source()->get());
                }
                else if (renderable->type() == Renderable::Data3DRenderable)
                {
                    Data3DExportFileFormat exportFormat(qPrintable(filename));
                    auto *r3d = static_cast<RenderableData3D *>(renderable.get());
                    if (!r3d->source())
                        Messenger::error("Failed to locate 3D data to export\n");
                    else
                        exportFormat.exportData(r3d->source()->get());
                }
            }
        }
        else if (copyToActions.contains(selectedAction))
        {
            Gizmo *destination = copyToActions.dataForItem(selectedAction);
            if (!destination)
                return;
            destination->sendData(
                QString::fromStdString(std::string(Renderable::renderableTypes().keyword(renderable->type()))),
                renderable->tag(), renderable->name());
        }
        else if (selectedAction == removeAction)
            removeRenderable(renderable);
    }

    // Done
    postRedisplay();
}
