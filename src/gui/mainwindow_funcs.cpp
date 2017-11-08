/*
	*** Main Window - Functions
	*** src/gui/mainwindow_funcs.cpp
	Copyright T. Youngs 2007-2017

	This file is part of dUQ.

	dUQ is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	dUQ is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with dUQ.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "main/duq.h"
#include "gui/mainwindow.h"
#include "gui/browser.h"
#include <QCloseEvent>
#include <QMdiSubWindow>

// Constructor
MonitorWindow::MonitorWindow(DUQ& duq) : QMainWindow(NULL), duq_(duq)
{
	// Initialise Qt's icon resource
	Q_INIT_RESOURCE(icons);

	// Set up user interface
	ui.setupUi(this);

	// Add default subwindows
	ui.MainArea->addSubWindow(new BrowserWindow(*this, duq_));

	refreshing_ = false;
}

// Destructor
MonitorWindow::~MonitorWindow()
{
}

// Return reference to dUQ
DUQ& MonitorWindow::duq()
{
	return duq_;
}

// Catch window close event
void MonitorWindow::closeEvent(QCloseEvent* event)
{
	/* if (saveBeforeClose())
	{
		saveSettings();
		event->accept();
	}
	else event->ignore();
	*/
	event->accept();
}

void MonitorWindow::resizeEvent(QResizeEvent* event)
{
}

/*
 * Update Functions
 */

// Set up window after load
void MonitorWindow::setUp()
{
// 	// Add controls for all processing modules
// 	RefListIterator<Module,bool> moduleIterator(duq_.processingModules().modules());
// 	while (Module* module = moduleIterator.iterate())
// 	{
// 		// Create a new ModuleWidget to use as the page for the tab
// 		ModuleControlWidget* moduleControlWidget = new ModuleControlWidget(NULL, module);
// 		ui.ProcessingModulesTab->addTab(moduleControlWidget, CharString("%s (%s)", module->name(), module->uniqueName()).get());
// 		
// 	}
}

// Return window for specified data (as pointer), if it exists
QMdiSubWindow* MonitorWindow::currentWindow(void* windowContents)
{
	RefListIterator<QMdiSubWindow,void*> windowIterator(currentWindows_);
	while (QMdiSubWindow* window = windowIterator.iterate()) if (windowIterator.currentData() == windowContents) return window;

	return NULL;
}

// Add window for specified data (as pointer)
QMdiSubWindow* MonitorWindow::addWindow(QWidget* widget, void* windowContents, const char* windowTitle)
{
	// Check that the windowContents aren't currently in the list
	QMdiSubWindow* window = currentWindow(windowContents);
	if (window)
	{
		Messenger::print("Refused to add window contents %p to our list, as it is already present elsewhere.\n");
		return window;
	}

	// Create a new QMdiSubWindow
	window = ui.MainArea->addSubWindow(widget);
	window->setWindowTitle(windowTitle);
	window->show();

	currentWindows_.add(window, windowContents);

	return window;
}

// Remove window for specified data (as pointer), removing it from the list
bool MonitorWindow::removeWindow(void* windowContents)
{
	// Find the windowContents the list
	QMdiSubWindow* window = currentWindow(windowContents);
	if (!window)
	{
		Messenger::print("Couldn't remove window contents %p from list, since it is not present.\n");
		return false;
	}

	currentWindows_.remove(window);

	return true;
}

// Refresh specified aspects of the window
void MonitorWindow::updateWidgets(int targets)
{

}
