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

/**
 * @file Parameters.hpp
 *
 * Contains the "Parameters" structure holding all parameters of the AdExp
 * model.
 *
 * @author Andreas Stöckel
 */

#ifndef _ADEXPSIM_PARAMETERS_HPP_
#define _ADEXPSIM_PARAMETERS_HPP_

#include <algorithm>
#include <cmath>

#include <utils/Types.hpp>
#include <utils/Vector.hpp>

namespace AdExpSim {

namespace DefaultParameters {
constexpr Val cM = 1e-9;        // Membrane capacitance [F]
constexpr Val gL = 0.05e-6;     // Membrane leak conductance [S]
constexpr Val eL = -70e-3;      // Leak channel reversal potential [V]
constexpr Val eE = 0e-3;        // Excitatory channel reversal potential [V]
constexpr Val eI = -70e-3;      // Inhibitory channel reversal potential [V]
constexpr Val eTh = -54.0e-3;   // Threshold potential [V]
constexpr Val eSpike = 20e-3;   // Spike potential [V]
constexpr Val eReset = -80e-3;  // Reset potential [V]
constexpr Val deltaTh = 2e-3;   // Slope factor [V]
constexpr Val tauI = 5e-3;      // Time constant for exponential decay of gI
constexpr Val tauE = 5e-3;      // Time constant for exponential decay of gE
constexpr Val tauW = 144e-3;    // Time constant for exponential decay of w
constexpr Val a = 4e-9;         // Subthreshold adaptation [S]
constexpr Val b = 0.0805e-9;    // Spike triggered adaptation [A]
}

/**
 * Structure holding the parameters of a single AdExp neuron.
 */
struct Parameters {
	Val cM = DefaultParameters::cM;    // Membrane capacitance [F]
	Val gL = DefaultParameters::gL;    // Membrane leak conductance [S]
	Val eL = DefaultParameters::eL;    // Leak channel reversal potential [V]
	Val eE = DefaultParameters::eE;    // Excitatory reversal potential [V]
	Val eI = DefaultParameters::eI;    // Inhibitory reversal potential [V]
	Val eTh = DefaultParameters::eTh;  // Threshold potential [V]
	Val eSpike = DefaultParameters::eSpike;    // Spike potential [V]
	Val eReset = DefaultParameters::eReset;    // Reset potential [V]
	Val deltaTh = DefaultParameters::deltaTh;  // Slope factor [V]
	Val tauI = DefaultParameters::tauI;        // Time constant for decay of gI
	Val tauE = DefaultParameters::tauE;        // Time constant for decay of gE
	Val tauW = DefaultParameters::tauW;        // Time constant for decay of w
	Val a = DefaultParameters::a;              // Subthreshold adaptation [S]
	Val b = DefaultParameters::b;              // Spike triggered adaptation [A]
};

/**
 * Structure holding the reduced parameter set which is actually used for the
 * calculations as well as some pre-calculated values
 */
class WorkingParameters : public Vector<WorkingParameters, 13> {
private:
	/**
	 * Inverse spike slope factor [1/V].
	 */
	Val mInvDeltaTh;

	/**
	 * Value to which the exponent is clamped.
	 */
	Val mMaxIThExponent;

	/**
	 * Effective spike potential. [V]
	 */
	Val mESpikeEff;

	/**
	 * Reduced effective spike potential (used for clamping) [V]
	 */
	Val mESpikeEffRed;

	/**
	 * Proposed ODE integrator tDelta. Set to one tenth of the smallest time
	 * constant.
	 */
	Val mTDelta;

public:
	using Vector<WorkingParameters, 13>::Vector;

	static constexpr Val MIN_DELTA_T =
	    0.1e-6;  // 0.1 µS -- used for the calculation of maxIThExponent

	/**
	 * Creates working parameters for the given Parameters instance.
	 *
	 * @param p is the Parameters instance for which the WorkingParameters set
	 * should be created.
	 */
	WorkingParameters(const Parameters &p = Parameters())
	    : Vector<WorkingParameters, 13>({
	          p.gL / p.cM,        // lL (0)
	          Val(1.0) / p.tauE,  // lE (1)
	          Val(1.0) / p.tauI,  // lI (2)
	          Val(1.0) / p.tauW,  // lW (3)
	          p.eE - p.eL,        // eE (4)
	          p.eI - p.eL,        // eI (5)
	          p.eTh - p.eL,       // eTh (6)
	          p.eSpike - p.eL,    // eSpike (7)
	          p.eReset - p.eL,    // eReset (8)
	          p.deltaTh,          // deltaTh (9)
	          p.a / p.cM,         // lA (10)
	          p.b / p.cM,         // lB (11)
	          Val(1.0) / p.cM     // wSpike (12)
	      })
	{
		update();
	}

	NAMED_VECTOR_ELEMENT(lL, 0);       // Membrane leak rate [Hz]
	NAMED_VECTOR_ELEMENT(lE, 1);       // Excitatory decay rate [Hz]
	NAMED_VECTOR_ELEMENT(lI, 2);       // Inhibitory decay rate [Hz]
	NAMED_VECTOR_ELEMENT(lW, 3);       // Adaptation cur. decay rate [Hz]
	NAMED_VECTOR_ELEMENT(eE, 4);       // Excitatory reversal potential [V]
	NAMED_VECTOR_ELEMENT(eI, 5);       // Inhibitory reversal potential [V]
	NAMED_VECTOR_ELEMENT(eTh, 6);      // Spike threshold potential [V]
	NAMED_VECTOR_ELEMENT(eSpike, 7);   // Spike generation potential [V]
	NAMED_VECTOR_ELEMENT(eReset, 8);   // Membrane reset potential [V]
	NAMED_VECTOR_ELEMENT(deltaTh, 9);  // Spike slope factor [V]
	NAMED_VECTOR_ELEMENT(lA, 10);      // Subthreshold adaptation [Hz]
	NAMED_VECTOR_ELEMENT(lB, 11);      // Spike trig. adaptation cur. [V/s]
	NAMED_VECTOR_ELEMENT(wSpike, 12);  // Mult. for spikes weights [V/A*s]

	/**
	 * Function used to calculate the effective spike potential.
	 */
	static Val calculateESpikeEff(double eTh, double deltaTh);

	/**
	 * Updates some derived values. This method must be called whenever the
	 * parameters have been changed from the outside.
	 */
	void update()
	{
		mInvDeltaTh = Val(1.0) / deltaTh();
		mMaxIThExponent =
		    log((eSpike() - eReset()) / (MIN_DELTA_T * deltaTh() * lL()));
		mESpikeEff = calculateESpikeEff(eTh(), deltaTh());
		mESpikeEffRed = mESpikeEff - Val(1e-4);
		mTDelta = Val(0.1) / std::max({lL(), lE(), lI(), lW(), lA()});
	}

	/**
	 * Returns the inverse spike slope factor. This is a derived value, call
	 * update() after any of the other parameters were changed to recalculate
	 * this value.
	 */
	Val invDeltaTh() const { return mInvDeltaTh; }

	/**
	 * Returns the value to which the exponent of the dvTh calculation is
	 * clamped to prevent overshooting. This is a derived value, call update()
	 * after any of the other parameters were changed to recalculate this value.
	 */
	Val maxIThExponent() const { return mMaxIThExponent; };

	/**
	 * Returns the effective spike potential. The membrane potential must be
	 * larger than this value for a spike to be produced. A spike will be
	 * produced if there are no inhibitory currents. This is a derived
	 * value, call update() after any of the other parameters were changed to
	 * recalculate this value.
	 */
	Val eSpikeEff() const { return mESpikeEff; }

	/**
	 * Returns a smaller version (0.1mV) of the effective spike potential.
	 * This value is used when the CLAMP_ITH flag is set in order to prevent the
	 * membrane potential from being set exactly to the spike potential, which
	 * could cause the neuron to be staying in an unwanted equilibrium, which
	 * may prevent abort conditions from triggering.
	 */
	Val eSpikeEffRed() const { return mESpikeEffRed; }

	/**
	 * Returns the tDelta proposed for this parameter set.
	 */
	Val tDelta() const { return mTDelta; }
};
}

#endif /* _ADEXPSIM_PARAMETERS_HPP_ */

