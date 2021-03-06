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

#include <cmath>

#include "Parameters.hpp"

namespace AdExpSim {

/* Class Parameters */

const std::vector<std::string> Parameters::names = {
    "gL",     "τE",     "τI",      "τW", "τRef", "eE", "eI", "eTh",
    "eSpike", "eReset", "deltaTh", "a",  "b",    "w",  "eL", "cM"};

const std::vector<std::string> Parameters::nameIds = {
    "gL",     "tauE",   "tauI",    "tauW", "tauRef", "eE", "eI", "eTh",
    "eSpike", "eReset", "deltaTh", "a",    "b",      "w",  "eL", "cM"};

/* Class WorkingParameters */

const std::vector<std::string> WorkingParameters::names = {
    "λL",  "λE",     "λI",     "λW",  "τRef", "eE", "eI",
    "eTh", "eSpike", "eReset", "ΔTh", "λA",   "λB", "w"};

const std::vector<std::string> WorkingParameters::nameIds = {
    "lL",  "lE",     "lI",     "lW",      "tauRef", "eE", "eI",
    "eTh", "eSpike", "eReset", "deltaTh", "lA",     "lB", "w"};

const std::vector<std::string> WorkingParameters::descriptions = {
    "Membrane leak rate",            "Excitatory channel decay rate",
    "Inhibitory channel decay rate", "Adaptation current decay rate",
    "Refractory period",             "Excitatory reversal potential",
    "Inhibitory reveral potential",  "Spike threshold potential",
    "Spike generation potential",    "Reset potential",
    "Spike slope factor",            "Subthreshold adaptation rate",
    "Spike adaptation current",      "Synapse weight multiplicator"};

const std::vector<std::string> WorkingParameters::units = {
    "Hz", "Hz", "Hz", "Hz", "V", "V", "V", "V", "V", "V", "Hz", "V/s", "V/As"};

const std::vector<bool> WorkingParameters::linear = {
    true, false, false, false, true, true, true, true,
    true, true,  true,  true,  true, true};

const std::vector<bool> WorkingParameters::inIfCondExp = {
    true, true, true,  false, true, true,  true, true,
    true, true, false, false, false, true};

const std::vector<std::string> WorkingParameters::originalNames = {
    "gL",     "τE",     "τI",  "τW", "τRef", "eE", "eI", "eTh",
    "eSpike", "eReset", "ΔTh", "gA", "iB", "w"};

const std::vector<std::string> WorkingParameters::originalUnits = {
    "S", "s", "s", "s", "s", "V", "V", "V", "V", "V", "V", "S", "A", "S"};

const std::vector<std::string> WorkingParameters::originalDescriptions = {
    "Membrane leak conductance",
    "Excitatory channel decay time const.",
    "Inhibitory channel decay time const.",
    "Adaptation current decay time const.",
    "Refractory period",
    "Excitatory reversal potential",
    "Inhibitory reveral potential",
    "Spike threshold potential",
    "Spike generation potential",
    "Reset potential",
    "Spike slope factor",
    "Subthreshold adaptation time const.",
    "Spike adaptation current",
    "Synapse weight multiplicator"};

Parameters WorkingParameters::toParameters(Val cM, Val eL) const
{
	Parameters res;
	res.cM() = cM;
	res.eL() = eL;
	for (size_t i = 0; i < WorkingParameters::Size; i++) {
		res[i] = toParameter(i, cM, eL);
	}
	return res;
}

Val WorkingParameters::toParameter(Val v, size_t idx, Val cM, Val eL)
{
	switch (idx) {
		case 0:
		case 11:
		case 12:
		case 13:
			return v * cM;
		case 1:
		case 2:
		case 3:
			return 1.0 / v;
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
			return v + eL;
		case 4:
		case 10:
			return v;
	}
	return v;
}

Val WorkingParameters::fromParameter(Val v, size_t idx, Val cM, Val eL)
{
	switch (idx) {
		case 0:
		case 11:
		case 12:
		case 13:
			return v / cM;
		case 1:
		case 2:
		case 3:
			return 1.0 / v;
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
			return v - eL;
		case 4:
		case 10:
			return v;
	}
	return v;
}

Val WorkingParameters::calculateESpikeEff(double eTh, double deltaTh)
{
	constexpr double EPS = 1e-9;

	// Abort if deltaTh is near zero. Additionally,  as shown (TM), threshold
	// potentials smaller or equal to deltaTh will result in a instable neuron
	// state and cause spikes no matter the current voltage is.
	if (deltaTh < EPS || eTh <= deltaTh) {
		return std::numeric_limits<Val>::lowest();
	}

	// Copy some variables for convenient access
	const double invDeltaTh = 1.0 / deltaTh;
	const double logDeltaTh = log(deltaTh);

	// Start a newton iteration
	double x = eTh + EPS;
	while (true) {
		const double dx = (logDeltaTh + (x - eTh) * invDeltaTh - log(x)) *
		                  (x * deltaTh) / (x - deltaTh);
		x -= dx;
		if (fabs(dx) < EPS) {
			break;
		}
	}
	return x;
}

Val WorkingParameters::calculateEExtr(double lE0)
{
	return eE() * (1.0 - exp(-lE0 / lE()));
}
}

