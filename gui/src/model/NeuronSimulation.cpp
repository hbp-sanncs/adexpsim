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

#include <simulation/DormandPrinceIntegrator.hpp>

#include "NeuronSimulation.hpp"

namespace AdExpSim {

void NeuronSimulation::prepare(const Parameters &params,
                               const SpikeTrain &train)
{
	this->params = params;
	this->evaluation = SpikeTrainEvaluation(train);
}

void NeuronSimulation::run()
{
	// Reset all output
	mValid = false;
	recorder.reset();
	outputSpikes.clear();
	outputGroups.clear();

	// Abort if the current parameters are not valid
	WorkingParameters wp(params);
	if (!wp.valid()) {
		return;
	}

	// Create a new controller -- make sure to abort after a certain count of
	// output spikes is superceeded.
	auto controller = createMaxOutputSpikeCountController(
	    [this]() -> size_t { return recorder.getData().outputSpikeTimes.size(); },
	    getTrain().getExpectedOutputSpikeCount() * 5);

	// Use a DormandPrinceIntegrator with default parameters
	DormandPrinceIntegrator integrator;

	// Run the actual simulation until the end of the time
	Model::simulate<Model::IF_COND_EXP>(getTrain().getSpikes(), recorder, controller, integrator,
	                wp, Time(-1), getTrain().getMaxT());

	// Run the evaluation to fetch the output spikes and the output groups
	evaluation.evaluate(wp, outputSpikes, outputGroups);

	// The result is not valid, if the controller has triggered
	mValid = !controller.tripped();
}
}

