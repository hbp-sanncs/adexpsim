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
 *  GNU General Public License for more details.d.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file Controller.hpp
 *
 * Contains basic controller classes used for the simulation. The controller
 * class determines when the simulation is aborted and my be used to track
 * minima/maxima in the state variables.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_CONTROLLER_HPP_
#define _ADEXPSIM_CONTROLLER_HPP_

#include <common/Types.hpp>

#include "Parameters.hpp"
#include "State.hpp"

namespace AdExpSim {

enum class ControllerResult { CONTINUE, MAY_CONTINUE, ABORT };

/**
 * The NullController class runs the simulation until tEnd is reached, producing
 * no overhead.
 */
class NullController {
public:
	static ControllerResult control(Time, const State &, const AuxiliaryState &,
	                                const WorkingParameters &, bool)
	{
		return ControllerResult::CONTINUE;
	}
};

/**
 * The DefaultController class is used to abort the model calculation once the
 * model has settled: The leak current is set to zero and the channel
 * conductance is small.
 */
class DefaultController {
public:
	/**
	 * Minimum neuron voltage at which the simulation can be aborted.
	 */
	static constexpr Val MIN_VOLTAGE = 1e-4;

	/**
	 * Minimum excitatory plus inhibitory channel rate.
	 */
	static constexpr Val MIN_RATE = 1e-3;

	/**
	 * Minimum total current. A non-zero voltage is ignored if the current is
	 * near zero, as the AdExp model may go into an additional equilibrium
	 * stage.
	 */
	static constexpr Val MIN_DV = 1e-3;

	/**
	 * The control function is responsible for aborting the simulation. The
	 * default controller aborts the simulation once the neuron has settled,
	 * so there are no inhibitory or excitatory currents and the membrane
	 * potential is near its resting potential.
	 *
	 * @param s is the current neuron state.
	 * @return true if the neuron should continue, false otherwise.
	 */
	static ControllerResult control(Time, const State &s,
	                                const AuxiliaryState &as,
	                                const WorkingParameters &, bool inRefrac)
	{
		return ((fabs(s.v()) > MIN_VOLTAGE &&
		         fabs(as.dvL() + as.dvE() + as.dvI() + as.dvTh()) > MIN_DV) ||
		        (s.lE() + s.lI()) > MIN_RATE || inRefrac)
		           ? ControllerResult::CONTINUE
		           : ControllerResult::MAY_CONTINUE;
	}
};

/**
 * The MaxValueController class is used to abort the model calculation once the
 * maximum voltage is reached and to record this maximum voltage.
 *
 * TODO: Better split this into a "MaxValueRecorder" and a "MaxValueController",
 * where the latter only consists of the static version of the "control"
 * function.
 */
class MaxValueController {
public:
	/**
	 * Minimum excitatory plus inhibitory channel rate. This value is chosen
	 * rather high as we want to abort as early as possible and only if the
	 * high excitatory current could still increase the membrane potential.
	 */
	static constexpr Val MIN_RATE = 10;

	/**
	 * Minimum excitatory plus inhibitory channel rate. This value is chosen
	 * rather high as we want to abort as early as possible and only if the
	 * high excitatory current could still increase the membrane potential.
	 */
	static constexpr Val MAX_DV = -1e-4;

	/**
	 * Maximum voltage
	 */
	Val vMax;

	/**
	 * Time point at which the maximum voltage was recorded.
	 */
	Time tVMax;

	/**
	 * Time point at which the effective spiking potential was reached.
	 */
	Time tSpike;

	/**
	 * Default constructor, resets this instance to its initial state.
	 */
	MaxValueController() { reset(); }

	/**
	 * Resets the controller to its initial state.
	 */
	void reset()
	{
		vMax = std::numeric_limits<Val>::lowest();
		tVMax = MAX_TIME;
		tSpike = MAX_TIME;
	}

	/**
	 * Static version of the control rule which determines when no larger
	 * membrane potentials or even spikes can be produced. This rule can be
	 * incorporated into other controllers.
	 */
	static ControllerResult control(const State &s, const AuxiliaryState &aux,
	                                bool inRefrac)
	{
		// Calculate the total current (voltage change rate, without dvL)
		const Val dvSum = aux.dvTh() + aux.dvE() + aux.dvI() + s.dvW();

		return (s.lE() > MIN_RATE || inRefrac ||
		        (dvSum < MAX_DV && dvSum + aux.dvL() < MAX_DV))
		           ? ControllerResult::CONTINUE
		           : ControllerResult::MAY_CONTINUE;
	}

	/**
	 * The control function is responsible for aborting the simulation. The
	 * default controller aborts the simulation once the neuron has settled,
	 * so there are no inhibitory or excitatory currents and the membrane
	 * potential is near its resting potential.
	 */
	ControllerResult control(Time t, const State &s, const AuxiliaryState &aux,
	                         const WorkingParameters p, bool inRefrac)
	{
		// Track the maximum voltage
		if (s.v() > vMax) {
			vMax = s.v();
			tVMax = t;
		}

		// Track the time at which the spike potential was reached
		if (s.v() > p.eSpikeEff() && t < tSpike) {
			tSpike = t;
		}

		// Do not abort as long as lE is larger than the minimum rate and the
		// current is negative (charges the neuron)
		return control(s, aux, inRefrac);
	}
};

/**
 * Controller class used to limit the number of output spikes to a reasonable
 * count.
 */
template <typename CountFun, typename ParentController>
class MaxOutputSpikeCountController {
private:
	ParentController &parent;
	CountFun countFun;
	size_t maxCount;

public:
	MaxOutputSpikeCountController(CountFun countFun, size_t maxCount,
	                              ParentController &parent)
	    : parent(parent), countFun(countFun), maxCount(maxCount)
	{
	}

	ControllerResult control(Time t, const State &s, const AuxiliaryState &as,
	                         const WorkingParameters &p, bool inRefrac) const
	{
		return tripped() ? ControllerResult::ABORT
		                 : parent.control(t, s, as, p, inRefrac);
	}

	bool tripped() const { return countFun() > maxCount; }
};

/**
 * Constructor method for the MaxOutputSpikeCountController.
 */
template <typename CountFun, typename ParentController>
static MaxOutputSpikeCountController<CountFun, ParentController>
createMaxOutputSpikeCountController(CountFun countFun, size_t maxCount,
                                    ParentController &parent)
{
	return MaxOutputSpikeCountController<CountFun, ParentController>(
	    countFun, maxCount, parent);
}

template <typename CountFun>
static MaxOutputSpikeCountController<CountFun, DefaultController>
createMaxOutputSpikeCountControllerWithDefault(CountFun countFun,
                                               size_t maxCount)
{
	DefaultController controller;  // DefaultController.control is static
	return createMaxOutputSpikeCountController(countFun, maxCount, controller);
}

template <typename CountFun>
static MaxOutputSpikeCountController<CountFun, NullController>
createMaxOutputSpikeCountController(CountFun countFun, size_t maxCount)
{
	NullController controller;  // NullController.control is static
	return createMaxOutputSpikeCountController(countFun, maxCount, controller);
}
}

#endif /* _ADEXPSIM_CONTROLLER_HPP_ */

