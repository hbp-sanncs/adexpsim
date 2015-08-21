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

#include <limits>
#include <vector>

#include <common/ProbabilityUtils.hpp>
#include <simulation/DormandPrinceIntegrator.hpp>
#include <simulation/Model.hpp>

#include "SingleGroupMultiOutEvaluation.hpp"

namespace AdExpSim {

namespace {
/**
 * Class used internally to record the number of output spikes and the state
 * after the spikes occured.
 */
class SpikeRecorder : public NullRecorder {
public:
	std::vector<std::pair<Time, State>> spikes;

	void outputSpike(Time t, const State &s) { spikes.emplace_back(t, s); }
};
}

// static Time tDelta(-1);
static Time tDelta = 0.1e-3_s;

/* Class SingleGroupMultiOutEvaluation */

Val SingleGroupMultiOutEvaluation::maximumPotential(
    const SpikeVec &spikes, const WorkingParameters &params, const State &state,
    const Time t0, const Time lastSpikeTime) const
{
	NullRecorder recorder;
	//	DormandPrinceIntegrator integrator(eTar);
	RungeKuttaIntegrator integrator;
	MaxValueController controller;

	// Extract a sub-list of spikes starting at the given t0
	SpikeVec subSpikes = extractSpikesFrom(spikes, t0);
	if (useIfCondExp) {
		Model::simulate<Model::IF_COND_EXP | Model::DISABLE_SPIKING>(
		    subSpikes, recorder, controller, integrator, params, tDelta,
		    MAX_TIME, state, lastSpikeTime);
	} else {
		Model::simulate<Model::CLAMP_ITH | Model::DISABLE_SPIKING |
		                Model::FAST_EXP>(subSpikes, recorder, controller,
		                                 integrator, params, tDelta, MAX_TIME,
		                                 state, lastSpikeTime);
	}

	return controller.vMax;
}

SingleGroupMultiOutEvaluation::SpikeCountResult
SingleGroupMultiOutEvaluation::spikeCount(const SpikeVec &spikes,
                                          const WorkingParameters &params) const
{
	// First pass: Record all output spikes
	SpikeRecorder recorder;
	//	DormandPrinceIntegrator integrator(eTar);
	RungeKuttaIntegrator integrator;
	DefaultController controller;
	if (useIfCondExp) {
		Model::simulate<Model::IF_COND_EXP>(spikes, recorder, controller,
		                                    integrator, params, tDelta);
	} else {
		Model::simulate<Model::FAST_EXP>(spikes, recorder, controller,
		                                 integrator, params, tDelta);
	}

	// Second pass: Record the potentially reachable potential before and after
	// the last spike.
	SpikeCountResult res(recorder.spikes.size());
	if (res.spikeCount <= 1) {
		const Val vMax = maximumPotential(spikes, params);
		res.vMax0 = vMax;
		res.vMax1 = vMax;
	}
	if (res.spikeCount >= 1) {
		const State &s0 = recorder.spikes[res.spikeCount - 1].second;
		const Time t0 = recorder.spikes[res.spikeCount - 1].first;
		res.vMax1 = maximumPotential(spikes, params, s0, t0, Time(0));
	}
	if (res.spikeCount >= 2) {
		const State &s0 = recorder.spikes[res.spikeCount - 2].second;
		const Time t0 = recorder.spikes[res.spikeCount - 2].first;
		res.vMax0 = maximumPotential(spikes, params, s0, t0, Time(0));
	}
	return res;
}

EvaluationResult SingleGroupMultiOutEvaluation::evaluate(
    const WorkingParameters &params) const
{
	// TODO
	return EvaluationResult();
}
}
