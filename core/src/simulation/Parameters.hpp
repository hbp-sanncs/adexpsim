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
#include <string>
#include <vector>

#include <common/Types.hpp>
#include <common/Vector.hpp>

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
constexpr Val tauI = 5e-3;      // Time constant for exponential decay of gI [s]
constexpr Val tauE = 5e-3;      // Time constant for exponential decay of gE [s]
constexpr Val tauW = 144e-3;    // Time constant for exponential decay of w [s]
constexpr Val tauRef = 0e-3;    // Refractory period [s]
constexpr Val a = 4e-9;         // Subthreshold adaptation [S]
constexpr Val b = 0.0805e-9;    // Spike triggered adaptation [A]
constexpr Val w = 0.03e-6;      // Default synapse weight [S]
}

/**
 * Structure holding the parameters of a single AdExp neuron.
 */
class Parameters : public Vector<Parameters, 16> {
public:
	using Vector<Parameters, 16>::Vector;

	/**
	 * Names of the corresponding vector component (human readable name)
	 */
	static const std::vector<std::string> names;

	/**
	 * Internal name ids.
	 */
	static const std::vector<std::string> nameIds;

	/**
	 * Default constructor, uses the values defined in the DefaultParameters
	 * namespace as default parameters.
	 */
	Parameters()
	    : Vector<Parameters, 16>(
	          {DefaultParameters::gL, DefaultParameters::tauE,
	           DefaultParameters::tauI, DefaultParameters::tauW,
	           DefaultParameters::tauRef, DefaultParameters::eE,
	           DefaultParameters::eI, DefaultParameters::eTh,
	           DefaultParameters::eSpike, DefaultParameters::eReset,
	           DefaultParameters::deltaTh, DefaultParameters::a,
	           DefaultParameters::b, DefaultParameters::w,
	           DefaultParameters::eL, DefaultParameters::cM})
	{
	}

	NAMED_VECTOR_ELEMENT(gL, 0);        // Membrane leak conductance [S]
	NAMED_VECTOR_ELEMENT(tauE, 1);      // Time constant for decay of gE [s]
	NAMED_VECTOR_ELEMENT(tauI, 2);      // Time constant for decay of gI [s]
	NAMED_VECTOR_ELEMENT(tauW, 3);      // Time constant for decay of w [s]
	NAMED_VECTOR_ELEMENT(tauRef, 4);    // Refractory period [s]
	NAMED_VECTOR_ELEMENT(eE, 5);        // Excitatory reversal potential [V]
	NAMED_VECTOR_ELEMENT(eI, 6);        // Inhibitory reversal potential [V]
	NAMED_VECTOR_ELEMENT(eTh, 7);       // Threshold potential [V]
	NAMED_VECTOR_ELEMENT(eSpike, 8);    // Spike potential [V]
	NAMED_VECTOR_ELEMENT(eReset, 9);    // Reset potential [V]
	NAMED_VECTOR_ELEMENT(deltaTh, 10);  // Slope factor [V]
	NAMED_VECTOR_ELEMENT(a, 11);        // Subthreshold adaptation [S]
	NAMED_VECTOR_ELEMENT(b, 12);        // Spike triggered adaptation [A]
	NAMED_VECTOR_ELEMENT(w, 13);        // Synapse weight  [S]
	NAMED_VECTOR_ELEMENT(eL, 14);       // Leak channel reversal potential [V]
	NAMED_VECTOR_ELEMENT(cM, 15);       // Membrane capacitance [F]

	Val tauM() const { return cM() / gL(); }
};

/**
 * Structure holding the reduced parameter set which is actually used for the
 * calculations as well as some pre-calculated values
 */
class WorkingParameters : public Vector<WorkingParameters, 14> {
private:
	/**
	 * Inverse spike slope factor [1/V].
	 */
	mutable Val mInvDeltaTh;

	/**
	 * Value to which the exponent is clamped.
	 */
	mutable Val mMaxIThExponent;

	/**
	 * Effective spike potential. [V]
	 */
	mutable Val mESpikeEff;

	/**
	 * Reduced effective spike potential (used for clamping) [V]
	 */
	mutable Val mESpikeEffRed;

	/**
	 * Proposed ODE integrator tDelta. Set to one tenth of the smallest time
	 * constant.
	 */
	mutable Val mTDelta;

	/**
	 * Maximum and minimal potentials. Used to encode voltage ranges.
	 */
	mutable Val mVMax, mVMin;

public:
	using Vector<WorkingParameters, 14>::Vector;

	static constexpr Val MIN_DELTA_T =
	    0.1e-6;  // 0.1 µS -- used for the calculation of maxIThExponent

	/**
	 * Creates working parameters for the given Parameters instance.
	 *
	 * @param p is the Parameters instance for which the WorkingParameters set
	 * should be created.
	 */
	WorkingParameters(const Parameters &p = Parameters())
	    : Vector<WorkingParameters, 14>(
	          {fromParameter(0, p), fromParameter(1, p), fromParameter(2, p),
	           fromParameter(3, p), fromParameter(4, p), fromParameter(5, p),
	           fromParameter(6, p), fromParameter(7, p), fromParameter(8, p),
	           fromParameter(9, p), fromParameter(10, p), fromParameter(11, p),
	           fromParameter(12, p), fromParameter(13, p)})
	{
		update();
	}

	NAMED_VECTOR_ELEMENT(lL, 0);        // Membrane leak rate [Hz]
	NAMED_VECTOR_ELEMENT(lE, 1);        // Excitatory decay rate [Hz]
	NAMED_VECTOR_ELEMENT(lI, 2);        // Inhibitory decay rate [Hz]
	NAMED_VECTOR_ELEMENT(lW, 3);        // Adaptation cur. decay rate [Hz]
	NAMED_VECTOR_ELEMENT(tauRef, 4);    // Refractory period [s]
	NAMED_VECTOR_ELEMENT(eE, 5);        // Excitatory reversal potential [V]
	NAMED_VECTOR_ELEMENT(eI, 6);        // Inhibitory reversal potential [V]
	NAMED_VECTOR_ELEMENT(eTh, 7);       // Spike threshold potential [V]
	NAMED_VECTOR_ELEMENT(eSpike, 8);    // Spike generation potential [V]
	NAMED_VECTOR_ELEMENT(eReset, 9);    // Membrane reset potential [V]
	NAMED_VECTOR_ELEMENT(deltaTh, 10);  // Spike slope factor [V]
	NAMED_VECTOR_ELEMENT(lA, 11);       // Subthreshold adaptation [Hz]
	NAMED_VECTOR_ELEMENT(lB, 12);       // Spike trig. adaptation cur. [V/s]
	NAMED_VECTOR_ELEMENT(w, 13);        // Mult. for spikes weights [Hz]

	/**
	 * Vector containing the name of each component.
	 */
	static const std::vector<std::string> names;

	/**
	 * Internal name ids.
	 */
	static const std::vector<std::string> nameIds;

	/**
	 * Vector containing a description of each component.
	 */
	static const std::vector<std::string> descriptions;

	/**
	 * Vector containing the unit of each component.
	 */
	static const std::vector<std::string> units;

	/**
	 * Vector specifying whether there is a linear/affine transformation from
	 * the original parameter set to the component in the working parameter set.
	 */
	static const std::vector<bool> linear;

	/**
	 * Vector specifying whether the given parameter is also present in the
	 * IF_COND_EXP model.
	 */
	static const std::vector<bool> inIfCondExp;

	/**
	 * Names of the corresponding component in the original parameter set.
	 */
	static const std::vector<std::string> originalNames;

	/**
	 * Descriptions of the component in the original parameter set.
	 */
	static const std::vector<std::string> originalDescriptions;

	/**
	 * Units of the component in the original parameter set.
	 */
	static const std::vector<std::string> originalUnits;

	/**
	 * Creates a Parameters instance from the this WorkingParameters instance.
	 * Requires a reference to a Parameters instance form which the superfluous
	 * parameters (cM and eL) can be read.
	 */
	Parameters toParameters(const Parameters &params) const
	{
		return toParameters(params.cM(), params.eL());
	}

	/**
	 * Creates a Parameters instance from the this WorkingParameters instance.
	 * Requires a reference to a Parameters instance form which the superfluous
	 * parameters (cM and eL) can be read.
	 */
	Parameters toParameters(Val cM, Val eL) const;

	/**
	 * Transforms the parameter with the given index from the
	 * WorkingParameters to the Parameters range.
	 */
	Val toParameter(size_t idx, const Parameters &params) const
	{
		return toParameter(arr[idx], idx, params);
	}

	/**
	 * Transforms the parameter with the given index from the
	 * WorkingParameters to the Parameters range.
	 */
	Val toParameter(size_t idx, Val cM, Val eL) const
	{
		return toParameter(arr[idx], idx, cM, eL);
	}

	/**
	 * Transforms the parameter with the given index from the
	 * WorkingParameters to the Parameters range.
	 */
	static Val toParameter(Val v, size_t idx, const Parameters &params)
	{
		return toParameter(v, idx, params.cM(), params.eL());
	}

	/**
	 * Transforms the parameter with the given index from the
	 * WorkingParameters to the Parameters range.
	 */
	static Val toParameter(Val v, size_t idx, Val cM, Val eL);

	/**
	 * Transforms the parameter with the given index from the
	 * Parameters to the WorkingParameters range.
	 */
	Val fromParameter(size_t idx, Val cM, Val eL) const
	{
		return fromParameter(arr[idx], idx, cM, eL);
	}

	/**
	 * Transforms the parameter with the given index from the
	 * Parameters to the WorkingParameters range.
	 */
	static Val fromParameter(size_t idx, const Parameters &params)
	{
		return fromParameter(params[idx], idx, params.cM(), params.eL());
	}

	/**
	 * Transforms the parameter with the given index from the
	 * Parameters to the WorkingParameters range.
	 */
	static Val fromParameter(Val v, size_t idx, const Parameters &params)
	{
		return fromParameter(v, idx, params.cM(), params.eL());
	}

	/**
	 * Transforms the parameter with the given index from the
	 * Parameters to the WorkingParameters range.
	 */
	static Val fromParameter(Val v, size_t idx, Val cM, Val eL);

	/**
	 * Transforms the given working parameter to a value to be used when
	 * plotting the parameter.
	 */
	static Val workingToPlot(Val v, size_t idx, Val cM, Val eL)
	{
		if (linear[idx]) {
			return toParameter(v, idx, cM, eL);
		}
		return v;
	}

	/**
	 * Transforms the given working parameter to a value to be used when
	 * plotting the parameter.
	 */
	static Val workingToPlot(Val v, size_t idx, const Parameters &params)
	{
		return workingToPlot(v, idx, params.cM(), params.eL());
	}

	/**
	 * Transforms the given working parameter to a value to be used when
	 * plotting the parameter.
	 */
	Val workingToPlot(size_t idx, Val cM, Val eL) const
	{
		return workingToPlot(arr[idx], idx, cM, eL);
	}

	/**
	 * Transforms the given working parameter to a value to be used when
	 * plotting the parameter.
	 */
	Val workingToPlot(size_t idx, const Parameters &params) const
	{
		return workingToPlot(arr[idx], idx, params);
	}

	/**
	 * Transforms the given parameter to a value to be used when plotting the
	 * parameter.
	 */
	static Val parameterToPlot(Val v, size_t idx, Val cM, Val eL)
	{
		if (!linear[idx]) {
			return fromParameter(v, idx, cM, eL);
		}
		return v;
	}

	/**
	 * Transforms the given parameter to a value to be used when plotting the
	 * parameter.
	 */
	static Val parameterToPlot(Val v, size_t idx, const Parameters &params)
	{
		return parameterToPlot(v, idx, params.cM(), params.eL());
	}

	/**
	 * Transforms the given parameter to a value to be used when plotting the
	 * parameter.
	 */
	Val parameterToPlot(size_t idx, Val cM, Val eL) const
	{
		return parameterToPlot(arr[idx], idx, cM, eL);
	}

	/**
	 * Transforms the given parameter to a value to be used when plotting the
	 * parameter.
	 */
	Val parameterToPlot(size_t idx, const Parameters &params) const
	{
		return parameterToPlot(arr[idx], idx, params);
	}

	/**
	 * Transforms the given plot coordinate to a working parameter value.
	 */
	static Val plotToWorking(Val v, size_t idx, Val cM, Val eL)
	{
		if (linear[idx]) {
			return fromParameter(v, idx, cM, eL);
		}
		return v;
	}

	/**
	 * Transforms the given plot coordinate to a working parameter value.
	 */
	static Val plotToWorking(Val v, size_t idx, const Parameters &params)
	{
		return plotToWorking(v, idx, params.cM(), params.eL());
	}

	/**
	 * Transforms the given plot coordinate to a working parameter value.
	 */
	Val plotToWorking(size_t idx, Val cM, Val eL) const
	{
		return plotToWorking(arr[idx], idx, cM, eL);
	}

	/**
	 * Transforms the given plot coordinate to a working parameter value.
	 */
	Val plotToWorking(size_t idx, const Parameters &params) const
	{
		return plotToWorking(arr[idx], idx, params);
	}

	/**
	 * Transforms the given plot coordinate to a parameter value.
	 */
	static Val plotToParameter(Val v, size_t idx, Val cM, Val eL)
	{
		if (!linear[idx]) {
			return toParameter(v, idx, cM, eL);
		}
		return v;
	}

	/**
	 * Transforms the given plot coordinate to a parameter value.
	 */
	static Val plotToParameter(Val v, size_t idx, const Parameters &params)
	{
		return plotToParameter(v, idx, params.cM(), params.eL());
	}

	/**
	 * Transforms the given plot coordinate to a parameter value.
	 */
	Val plotToParameter(size_t idx, Val cM, Val eL) const
	{
		return plotToParameter(arr[idx], idx, cM, eL);
	}

	/**
	 * Transforms the given plot coordinate to a parameter value.
	 */
	Val plotToParameter(size_t idx, const Parameters &params) const
	{
		return plotToParameter(arr[idx], idx, params);
	}

	/**
	 * Function used to calculate the effective spike potential.
	 */
	static Val calculateESpikeEff(double eTh, double deltaTh);

	/**
	 * Updates some derived values. This method must be called whenever the
	 * parameters have been changed from the outside.
	 */
	void update() const
	{
		mInvDeltaTh = Val(1.0) / deltaTh();
		mMaxIThExponent =
		    log((eSpike() - eReset()) / (MIN_DELTA_T * deltaTh() * lL()));
		mESpikeEff = calculateESpikeEff(eTh(), deltaTh());
		mESpikeEffRed = mESpikeEff - Val(1e-4);
		mTDelta = std::max(Val(1e-7),
		                   Val(0.1) / std::max({lL(), lE(), lI(), lW(), lA()}));
		mVMax = std::max({0.0f, eE(), eI(), eSpike(), eTh(), eReset()});
		mVMin = std::min({0.0f, eE(), eI(), eSpike(), eTh(), eReset()});
	}

	/**
	 * Returns true if the parameters are in a range which is generally valid.
	 */
	bool valid() const
	{
		return lL() > 0 && lE() > 0 && lI() > 0 && lW() > 0 && tauRef() >= 0 &&
		       deltaTh() > 0 && lA() >= 0 && lB() >= 0 && eE() > eI() &&
		       eE() > eTh() && eE() > 0 && eSpike() > 0 && eSpike() > eTh() &&
		       eReset() <= 0;
	}

	/**
	 * Estimates a reasonable staring value for the weight w for a certain
	 * number of input spikes. w is chosen in such a way, that the maximum
	 * membrane potential that could theoretically be reached equals the
	 * effective spike potential. In practive the returned weight is too small,
	 * but the order of magnitude is correct.
	 *
	 * @param xi is the number of input spikes for which the neuron should
	 * spike.
	 */
	Val estimateW(Val xi) const
	{
		return -log(1 - mESpikeEff / eE()) * lE() / xi;
	}

	/**
	 * Calculates the absolute maximum value that could theoretically be
	 * reached for the given parameter set and the given excitatory input rate.
	 *
	 * @param lE0 is the excitatory input rate at time t = 0.
	 */
	Val calculateEExtr(double lE0);

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
	Val eSpikeEff(bool useIfCondExp = false) const
	{
		return useIfCondExp ? eTh() : mESpikeEff;
	}

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

	/**
	 * Returns the maximum potential stored in the parameters.
	 */
	Val vMax() const { return mVMax; }

	/**
	 * Returns the minimum potential stored in the parameters.
	 */
	Val vMin() const { return mVMin; }
};
}

#endif /* _ADEXPSIM_PARAMETERS_HPP_ */

