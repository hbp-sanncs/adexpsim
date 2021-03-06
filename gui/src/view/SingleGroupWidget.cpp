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

#include <QAction>
#include <QHBoxLayout>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>

#include <utils/ParameterCollection.hpp>

#include "ParameterWidget.hpp"
#include "SingleGroupWidget.hpp"

namespace AdExpSim {

SingleGroupWidget::SingleGroupWidget(
    std::shared_ptr<ParameterCollection> params, QWidget *parent)
    : QWidget(parent), params(params)
{
	// Create the update timer
	updateTimer = new QTimer(this);
	updateTimer->setSingleShot(true);
	connect(updateTimer, SIGNAL(timeout()), this,
	        SLOT(triggerUpdateParameters()));

	// Create the main layout
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setMargin(0);
	layout->setSpacing(0);

	// Create the toolbar and the corresponding actions
	actCopyToSpikeTrain =
	    new QAction(QIcon::fromTheme("go-up"), "To SpikeTrain", this);
	actCopyToSpikeTrain->setToolTip("Copy parameters to Spike Train settings");
	actLockNM1 = new QAction(QIcon("data/lock.png"), "Lock n", this);
	actLockNM1->setToolTip("Locks the n and n-1 sliders");
	actLockNM1->setCheckable(true);
	actLockNM1->setChecked(true);

	toolbar = new QToolBar();
	toolbar->addAction(actCopyToSpikeTrain);
	toolbar->addSeparator();
	toolbar->addAction(actLockNM1);

	connect(actCopyToSpikeTrain, SIGNAL(triggered()), this,
	        SLOT(copyToSpikeTrain()));

	// Create the other parameter widgets
	paramN = new ParameterWidget(this, "n", 3, 1, 10, "", "n");
	paramN->setIntOnly(true);
	paramN->setMinMaxEnabled(false);
	paramNM1 = new ParameterWidget(this, "n-1", 2, 0, 9, "", "nM1");
	paramNM1->setIntOnly(true);
	paramNM1->setMinMaxEnabled(false);
	paramNOut = new ParameterWidget(this, "nOut", 1, 0, 100, "", "nOut");
	paramNOut->setIntOnly(true);
	paramNOut->setMinMaxEnabled(false);

	connect(paramN, SIGNAL(update(Val, const QVariant &)),
	        SLOT(handleParameterUpdate(Val, const QVariant &)));
	connect(paramNM1, SIGNAL(update(Val, const QVariant &)),
	        SLOT(handleParameterUpdate(Val, const QVariant &)));
	connect(paramNOut, SIGNAL(update(Val, const QVariant &)),
	        SLOT(handleParameterUpdate(Val, const QVariant &)));

	// Add all widgets to the main layout
	layout->addWidget(toolbar);
	layout->addWidget(paramN);
	layout->addWidget(paramNM1);
	layout->addWidget(paramNOut);
	setLayout(layout);

	// Set the widgets to the correct values
	refresh();
}

SingleGroupWidget::~SingleGroupWidget()
{
	// Do nothing here, only needed for the shared_ptr
}

void SingleGroupWidget::handleParameterUpdate(Val value, const QVariant &data)
{
	if (signalsBlocked()) {
		return;
	}

	QString s = data.toString();
	if (s == "n") {
		params->singleGroup.n = value;
		if (actLockNM1->isChecked()) {
			params->singleGroup.nM1 = value - 1;
			paramNM1->setValue(value - 1);
		}
	} else if (s == "nM1") {
		params->singleGroup.nM1 = value;
		if (actLockNM1->isChecked()) {
			params->singleGroup.n = value + 1;
			paramN->setValue(value + 1);
		}
	} else if (s == "nOut") {
		params->singleGroup.nOut = value;
	}
	// Wait 100ms with triggering the update
	updateTimer->start(100);
}

void SingleGroupWidget::refresh()
{
	blockSignals(true);
	paramN->setValue(params->singleGroup.n);
	paramNM1->setValue(params->singleGroup.nM1);
	paramNOut->setValue(params->singleGroup.nOut);
	blockSignals(false);
}

void SingleGroupWidget::triggerUpdateParameters()
{
	emit updateParameters(std::set<size_t>{});
}

void SingleGroupWidget::copyToSpikeTrain()
{
	params->train.fromSingleGroupSpikeData(params->singleGroup);
	emit updateParameters(std::set<size_t>{});
}
}
