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
#include <simulation/Model.hpp>
#include <simulation/Recorder.hpp>
#include <simulation/SpikeTrain.hpp>

#include <iostream>

using namespace AdExpSim;

int main()
{
	// Use the default parameters
	Parameters params;
	NullController controller;
	DormandPrinceIntegrator integrator;

	// Create the recorder
	CsvRecorder<> recorder(params, 0.1e-3_s, std::cout);

	// Create a vector containing all input spikes
	/*	SpikeTrain train({{4, 1, 1e-3}, {1, 0, 1e-3}}, 10,
	                     true, 0.1_s, 0.01);
	SpikeVec spikes = train.getSpikes();*/
	SpikeVec spikes = buildInputSpikes(3, 0.5e-3_s);

	WorkingParameters wParams(params);
	/*	std::cerr << "Max. iTh exponent: " << wParams.maxIThExponent() <<
	   std::endl;
	    std::cerr << "Effective spike potential: "
	              << wParams.eSpikeEff() + params.eL() << std::endl;*/

	Model::simulate<Model::IF_COND_EXP | Model::DISABLE_SPIKING>(
	    spikes, recorder, controller, integrator, wParams, Time(-1), 0.1_s, State(wParams.eReset()));
	/*	std::cerr << "Max. membrane potential: " << controller.vMax +
	   params.eL()
	              << std::endl;*/
	return 0;
}

