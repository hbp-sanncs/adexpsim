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

#include <QTimer>
#include <QThreadPool>

#include <exploration/Exploration.hpp>
#include <exploration/SpikeTrainEvaluation.hpp>
#include <exploration/SingleGroupSingleOutEvaluation.hpp>
#include <exploration/SingleGroupMultiOutEvaluation.hpp>
#include <utils/ParameterCollection.hpp>

#include "IncrementalExploration.hpp"

namespace AdExpSim {

/*
 * Class IncrementalExplorationRunner
 */

IncrementalExplorationRunner::IncrementalExplorationRunner(
    Exploration &exploration, std::shared_ptr<ParameterCollection> params)
    : aborted(false), exploration(exploration), params(params)
{
	setAutoDelete(false);
}

IncrementalExplorationRunner::~IncrementalExplorationRunner()
{
	// Destructor needed here for shared pointers
}

void IncrementalExplorationRunner::run()
{
	bool ok = false;
	auto progressCallback = [&](float p) -> bool {
		emit progress(p);
		return !aborted.load();
	};

	switch (params->evaluation) {
		case EvaluationType::SPIKE_TRAIN:
			ok = exploration.run(
			    SpikeTrainEvaluation(params->train,
			                         params->model == ModelType::IF_COND_EXP),
			    progressCallback);
			break;
		case EvaluationType::SINGLE_GROUP_SINGLE_OUT:
			ok = exploration.run(SingleGroupSingleOutEvaluation(
			                         params->environment, params->singleGroup,
			                         params->model == ModelType::IF_COND_EXP),
			                     progressCallback);
			break;
		case EvaluationType::SINGLE_GROUP_MULTI_OUT:
			ok = exploration.run(SingleGroupMultiOutEvaluation(
			                         params->environment, params->singleGroup,
			                         params->model == ModelType::IF_COND_EXP),
			                     progressCallback);
			break;
	}

	// Emit the done event
	emit done(ok && !aborted.load());
}

void IncrementalExplorationRunner::abort() { aborted.store(true); }

/*
 * Class IncrementalExploration
 */

constexpr int IncrementalExploration::MIN_LEVEL;
constexpr int IncrementalExploration::MAX_LEVEL;
constexpr int IncrementalExploration::MAX_LEVEL_INITIAL;

IncrementalExploration::IncrementalExploration(
    std::shared_ptr<ParameterCollection> params, QObject *parent)
    : QObject(parent),
      pool(new QThreadPool(this)),
      maxLevel(MAX_LEVEL_INITIAL),
      dimX(0),
      dimY(1),
      minX(1),
      maxX(100),
      minY(1),
      maxY(100),
      params(params),
      level(MIN_LEVEL),
      restart(false),
      inEmitData(false),
      currentRunner(nullptr)
{
	updateTimer = new QTimer(this);
	connect(updateTimer, SIGNAL(timeout()), this, SLOT(updateTimeout()));
}

IncrementalExploration::~IncrementalExploration()
{
	if (currentRunner != nullptr) {
		currentRunner->abort();
	}
	pool->waitForDone();
}

bool IncrementalExploration::isActive() const
{
	return (currentRunner != nullptr) || (level <= maxLevel);
}

void IncrementalExploration::setMaxLevel(int maxLevel)
{
	// Make sure the given level is in the valid range between MIN_LEVEL and
	// MAX_LEVEL
	this->maxLevel = std::min(MAX_LEVEL, std::max(MIN_LEVEL, maxLevel));

	// If the given level is larger than the current level and we're idling,
	// get to work!
	if (level <= this->maxLevel && currentRunner == nullptr) {
		start();
	}

	// If the current level is larger than the new maximum level and we're
	// working => abort!
	if (level > maxLevel && currentRunner != nullptr) {
		currentRunner->abort();
	}
}

void IncrementalExploration::start()
{
	// Create a new Exploration instance
	const size_t res = 1 << level;
	exploration =
	    Exploration(params->params, dimX, dimY, DiscreteRange(minX, maxX, res),
	                DiscreteRange(minY, maxY, res));

	// Create a new IncrementExplorationRunner and connect all signals
	currentRunner = new IncrementalExplorationRunner(exploration, params);
	connect(currentRunner, SIGNAL(progress(float)), this,
	        SLOT(runnerProgress(float)));
	connect(currentRunner, SIGNAL(done(bool)), this, SLOT(runnerDone(bool)));

	// Reset the restart flag and increment the level counter
	restart = false;
	level++;

	// Start the runner
	pool->start(currentRunner);
}

void IncrementalExploration::update()
{
	// Delay the update action by 250msec, however, we can abort the current job
	updateTimer->start(250);
	if (currentRunner != nullptr) {
		currentRunner->abort();
	}
}

void IncrementalExploration::updateTimeout()
{
	// Kill the update timer
	updateTimer->stop();

	// Schedule a restart and reset the current level
	restart = true;
	level = MIN_LEVEL;

	// If there currently is no runner, start a new one, otherwise abort the
	// current runner
	if (currentRunner == nullptr) {
		start();
	} else {
		currentRunner->abort();
	}
}

void IncrementalExploration::runnerProgress(float p)
{
	// l is always set ot the next level, so we have to decrement it by one
	const int L = level - MIN_LEVEL - 1;
	if (L < 0 || restart) {
		emit progress(0.0, false);
		return;
	}

	// Calculate the normalization value (sum over 2^i for i=0..(MAX-MIN))
	const int norm = (1 << (maxLevel - MIN_LEVEL + 1)) - 1;

	// Calculate the previous progress (sum over 2^i for i = 0..L-1))
	const int previous = (1 << L) - 1;

	// Progress is (previous + 2^L * p) / norm
	const Val pTotal = (Val(previous) + Val(1 << L) * p) / Val(norm);
	emit progress(pTotal, true);
}

void IncrementalExploration::runnerDone(bool ok)
{
	// If the result is ok, emit the data
	if (ok) {
		inEmitData = true;
		emit data(exploration);
		inEmitData = false;
	} else {
		emit progress(0.0, false);
	}

	// Delete the runner object and the exploration object
	delete currentRunner;
	currentRunner = nullptr;

	// If the last level is reached, emit a last progress event
	if (level > maxLevel) {
		emit progress(1.0, false);
	}

	// Start the next iteration
	if ((level <= maxLevel && ok) || restart) {
		start();
	}
}

void IncrementalExploration::updateRange(size_t dimX, size_t dimY, Val minX,
                                         Val maxX, Val minY, Val maxY)
{
	if ((dimX != this->dimX || dimY != this->dimY || minX != this->minX ||
	     maxX != this->maxX || minY != this->minY || maxY != this->maxY) &&
	    !inEmitData) {
		// Copy the range and schedule an update
		this->dimX = dimX;
		this->dimY = dimY;
		this->minX = minX;
		this->maxX = maxX;
		this->minY = minY;
		this->maxY = maxY;
		update();
	}
}
}

