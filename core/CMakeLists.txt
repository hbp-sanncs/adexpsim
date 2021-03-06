#  AdExpSim -- Simulator for the AdExp model
#  Copyright (C) 2015  Andreas Stöckel
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#
# This CMakeLists file builds the AdExpSimCore library used by both the CLI
# applications and the GUI
#

CMAKE_MINIMUM_REQUIRED (VERSION 3.3)
PROJECT (AdExpSimCore)

# Add the src/ and lib/ directory as include directory
INCLUDE_DIRECTORIES(
	include/
	lib/
	src/
)

CONFIGURE_FILE(
	"${PROJECT_SOURCE_DIR}/include/config.h.in"
	"${PROJECT_SOURCE_DIR}/include/config.h"
)

# AdExpSimCore library
ADD_LIBRARY(AdExpSimCore
	src/common/Matrix
	src/common/ProbabilityUtils
	src/common/Terminal
	src/common/Timer
	src/common/Types
	src/common/Vector
	src/exploration/EvaluationResult
	src/exploration/Exploration
	src/exploration/FractionalSpikeCount
	src/exploration/Optimization
	src/exploration/Simplex
	src/exploration/SimplexPool
	src/exploration/SingleGroupEvaluationBase
	src/exploration/SingleGroupSingleOutEvaluation
	src/exploration/SingleGroupMultiOutEvaluation
	src/exploration/SpikeTrainEvaluation
	src/simulation/Controller
	src/simulation/DormandPrinceIntegrator
	src/simulation/HardwareParameters
	src/simulation/Integrator
	src/simulation/Model
	src/simulation/Parameters
	src/simulation/Recorder
	src/simulation/Spike
	src/simulation/SpikeTrain
	src/simulation/State
	src/utils/ParameterCollection
)

# Link library
TARGET_LINK_LIBRARIES(AdExpSimCore
	rt
	pthread
)

