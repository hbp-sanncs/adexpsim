/*
 *  AdExpSim -- Simulator for the AdExp model
 *  Copyright (C) 2015  Andreas Stöckel
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>

#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QToolBox>
#include <QToolBar>
#include <QScrollArea>
#include <QDockWidget>
#include <QTimer>
#include <QLabel>
#include <QVBoxLayout>

#include <simulation/Parameters.hpp>
#include <simulation/Spike.hpp>
#include <view/ParametersWidget.hpp>
#include <view/SpikeTrainWidget.hpp>

#include "MainWindow.hpp"
#include "ExplorationWindow.hpp"
#include "SimulationWindow.hpp"

namespace AdExpSim {
MainWindow::MainWindow()
    : params(std::make_shared<Parameters>()),
      train(std::make_shared<SpikeTrain>(
          SpikeTrain({
                      {3, 1, 1e-3}, {2, 0, 1e-3},
                     },
                     2, true, 0.033_s)))
{
	// Create all actions and menus
	createActions();
	createMenus();
	createWidgets();

	// Resize the window
	resize(400, 600);

	// Open a new exploration and simulation window
	newExploration();
	newSimulation();

	// Set the window title and the icon
	setWindowIcon(QIcon("data/icon_main.svg"));
	setWindowTitle("Control Panel ‒ AdExpSim");
}

MainWindow::~MainWindow() {}

void MainWindow::createActions()
{
	actNewExplorationWnd = new QAction(tr("New Exploration Window..."), this);
	connect(actNewExplorationWnd, SIGNAL(triggered()), this,
	        SLOT(newExploration()));

	actNewSimulationWnd = new QAction(tr("New Simulation Window..."), this);
	connect(actNewSimulationWnd, SIGNAL(triggered()), this,
	        SLOT(newSimulation()));

	actOpenExploration = new QAction(QIcon::fromTheme("document-open"),
	                                 tr("Open Exploration..."), this);

	actSaveExploration = new QAction(QIcon::fromTheme("document-save"),
	                                 tr("Save Current Exploration..."), this);

	actExit = new QAction(tr("Exit"), this);
	connect(actExit, SIGNAL(triggered()), this, SLOT(close()));
}

void MainWindow::createMenus()
{
	QMenu *fileMenu = new QMenu(tr("&File"), this);
	fileMenu->addAction(actNewExplorationWnd);
	fileMenu->addAction(actNewSimulationWnd);
	fileMenu->addSeparator();
	fileMenu->addAction(actOpenExploration);
	fileMenu->addAction(actSaveExploration);
	fileMenu->addSeparator();
	fileMenu->addAction(actExit);

	menuBar()->addMenu(fileMenu);
}

void MainWindow::createWidgets()
{
	// Create the tool box
	QToolBox *tools = new QToolBox(this);

	// Create the spike train panel and add it to the tool box
	spikeTrainWidget = new SpikeTrainWidget(train, this);
	connect(spikeTrainWidget, SIGNAL(updateParameters(std::set<size_t>)), this,
	        SLOT(handleUpdateParameters(std::set<size_t>)));
	tools->addItem(spikeTrainWidget, "Spike Train");

	// Create the parameters panel and add it to the tool box
	parametersWidget = new ParametersWidget(params, this);
	connect(parametersWidget, SIGNAL(updateParameters(std::set<size_t>)), this,
	        SLOT(handleUpdateParameters(std::set<size_t>)));
	tools->addItem(parametersWidget, "Parameters");

	// Set the tool box as central widget
	setCentralWidget(tools);
}

void MainWindow::newExploration()
{
	ExplorationWindow *wnd = new ExplorationWindow(params, train);
	connect(wnd, SIGNAL(updateParameters(std::set<size_t>)), this,
	        SLOT(handleUpdateParameters(std::set<size_t>)));
	windows.push_back(wnd);
	wnd->show();
}

void MainWindow::newSimulation()
{
	SimulationWindow *wnd = new SimulationWindow(params, train);
	connect(wnd, SIGNAL(updateParameters(std::set<size_t>)), this,
	        SLOT(handleUpdateParameters(std::set<size_t>)));
	windows.push_back(wnd);
	wnd->show();
}

void MainWindow::handleUpdateParameters(std::set<size_t> dims)
{
	// Forward the event to the ParametersWidget instance
	parametersWidget->handleUpdateParameters(dims);

	// Forward the event to all exploration and simulation windows
	for (auto window : windows) {
		if (window != nullptr) {
			window->handleUpdateParameters(dims);
		}
	}
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	for (auto window : windows) {
		if (window != nullptr) {
			window->close();
		}
	}
}
}

