// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2021 Team Dissolve and contributors

#include "gui/viewer.hui"
#include <QMenu>
#include <QMouseEvent>

/*
 * Private Functions
 */

// Mouse press event
void BaseViewer::mousePressEvent(QMouseEvent *event)
{
    // Store the current button state and mouse position (with inverted y coordinate)
    buttonState_ = event->buttons();
    rMouseDown_.set(event->pos().x(), contextHeight_ - event->pos().y(), 0.0);
    rMouseLast_ = rMouseDown_;
    mouseDownModifiers_ = event->modifiers();
    mouseReleaseTimer_.start();

    // If a 2D view, store the clicked local coordinate
    if (view().isFlatView())
        clicked2DAxesCoordinates_ = screenTo2DAxes(event->pos().x(), contextHeight_ - event->pos().y());

    interacting_ = true;

    // Handle the event
    startInteraction();
}

// Mouse release event
void BaseViewer::mouseReleaseEvent(QMouseEvent *event)
{
    // Detect right-click intended to call a context menu
    if (buttonState_.testFlag(Qt::RightButton) &&
        (((rMouseLast_ - rMouseDown_).magnitude() <= 1) || mouseReleaseTimer_.isActive()))
    {
        contextMenuRequested(event->pos());
    }
    else
    {
        // Handle the event normally
        endInteraction();
    }

    postRedisplay();

    // Clear button state and interaction flag
    buttonState_ = Qt::NoButton;
    interacting_ = false;
}

// Mouse move event
void BaseViewer::mouseMoveEvent(QMouseEvent *event)
{
    const auto dx = event->pos().x() - rMouseLast_.x;
    const auto dy = (contextHeight_ - event->pos().y()) - rMouseLast_.y;

    // Store the new mouse coordinate with inverted y coordinate
    rMouseLast_.set(event->pos().x(), contextHeight_ - event->pos().y(), 0.0);

    // If a 2D view, store the current local Axes coordinate
    if (view().isFlatView())
        current2DAxesCoordinates_ = screenTo2DAxes(rMouseLast_.x, rMouseLast_.y);

    // Handle the event, passing the delta position
    mouseMoved(dx, dy);

    setFocus();
}

// Mouse wheel event
void BaseViewer::wheelEvent(QWheelEvent *event)
{
    // Handle the event
    mouseWheeled(event->pixelDelta().y());

    postRedisplay();
}

// Mouse double click event
void BaseViewer::mouseDoubleClickEvent(QMouseEvent *event)
{
    // Handle the event
    mouseDoubleClicked();
}

/*
 * Private Slots
 */

// Key press event
void BaseViewer::keyPressEvent(QKeyEvent *event)
{
    // Handle the event
    if (keyPressed(event->key()))
        event->accept();
    else
        event->ignore();
}

// Key release event
void BaseViewer::keyReleaseEvent(QKeyEvent *event)
{
    // Handle the event
    if (keyReleased(event->key()))
        event->accept();
    else
        event->ignore();
}

/*
 * Protected Virtuals
 */

// Mouse moved
void BaseViewer::mouseMoved(int dx, int dy) {}

// Mouse 'wheeled'
void BaseViewer::mouseWheeled(int delta)
{
    auto scrollup = delta > 0;

    // Perform camera zoom
    double zrange = view_.axes().stretch(2) * view_.axes().realRange(2);
    if (zrange < 1.0)
        zrange = 1.0;
    view_.translateView(0.0, 0.0, 0.5 * zrange * (scrollup ? -1.0 : 1.0));

    // Never let camera z go above z = 1.0...
    auto trans = view_.viewTranslation();
    if (trans.z > 1.0)
        trans.z = 1.0;
    view_.setViewTranslation(trans.x, trans.y, trans.z);

    postRedisplay();
}

// Mouse double clicked
void BaseViewer::mouseDoubleClicked() {}

// Context menu requested
void BaseViewer::contextMenuRequested(QPoint pos) {}

// Key pressed
bool BaseViewer::keyPressed(int key) { return false; }

// Key released
bool BaseViewer::keyReleased(int key) { return false; }
