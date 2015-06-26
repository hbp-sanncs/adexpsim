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

#include "ParameterCollection.hpp"

namespace AdExpSim {

std::vector<std::string> ParameterCollection::modelNames = {"IfCondExp",
                                                            "AdIfCondExp"};

std::vector<std::string> ParameterCollection::evaluationNames = {"Train",
                                                                 "Single"};

ParameterCollection::ParameterCollection()
    : model(ModelType::IF_COND_EXP),
      evaluation(EvaluationType::SPIKE_TRAIN),
      train({
             {3, 1, 1e-3}, {2, 0, 1e-3}, {0, 0, 1e-3},
            },
            3, true, 0.033_s),
      singleGroup(train.toSingleGroupSpikeData())
{
	// Initialize the optimize/explore flags
	optimize.fill(true);
	explore.fill(false);

	// Initialize the minimum/maximum values
	// TODO
}
}
