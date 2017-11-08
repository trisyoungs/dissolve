/*
	*** Main Window
	*** src/gui/mainwindow.h
	Copyright T. Youngs 2007-2017

	This file is part of DUQ.

	DUQ is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	DUQ is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with DUQ.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DUQ_MAINWINDOW_H
#define DUQ_MAINWINDOW_H

#include "gui/ui_mainwindow.h"
#include "templates/reflist.h"

// Forward Declarations
class DUQ;
class QMdiSubWindow;

class MonitorWindow : public QMainWindow
{
	// All Qt declarations must include this macro
	Q_OBJECT

	public:
	// Constructor / Destructor
	MonitorWindow(DUQ& duq);
	~MonitorWindow();
	// Main form declaration
	Ui::MonitorWindow ui;

	protected:
	void closeEvent(QCloseEvent* event);
	void resizeEvent(QResizeEvent* event);


	/*
	 * DUQ Reference
	 */
	private:
	// DUQ reference
	DUQ& duq_;

	public:
	// Return reference to DUQ
	DUQ& duq();


	/*
	 * Update Functions
	 */
	public:
	// Update Targets 
	enum UpdateTarget { DefaultTarget = 0, AllTargets = 65535 };

	private:
	// Whether window is currently refreshing
	bool refreshing_;
	// Whether window has been shown
	bool shown_;
	// List of current data windows and their display target (as a pointer)
	RefList<QMdiSubWindow,void*> currentWindows_;

	public:
	// Set up window after load
	void setUp();
	// Return window for specified data (as pointer), if it exists
	QMdiSubWindow* currentWindow(void* windowContents);
	// Add window for widget containing specified data (as pointer)
	QMdiSubWindow* addWindow(QWidget* widget, void* windowContents, const char* windowTitle);
	// Remove window for specified data (as pointer), removing it from the list
	bool removeWindow(void* windowContents);

	public slots:
	// Refresh specified aspects of the window
	void updateWidgets(int targets = MonitorWindow::DefaultTarget);
};

#endif
