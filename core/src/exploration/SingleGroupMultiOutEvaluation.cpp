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

#include <simulation/Controller.hpp>
#include <simulation/DormandPrinceIntegrator.hpp>
#include <simulation/Model.hpp>
#include <simulation/Recorder.hpp>

#include "FractionalSpikeCount.hpp"
#include "SingleGroupMultiOutEvaluation.hpp"

namespace AdExpSim {
/**
 * Student-t like long-tail distribution, no normalization. In contrast to a
 * gauss-like distribution has a long tail which makes it more suitable for
 * optimization.
 */
static Val dist(Val x, Val mu, Val nu)
{
	Val d = x - mu;
	return std::pow(1.0 + d * d / nu, -(nu + 1.0) * 0.5);
}

static Val correct(Val x)
{
	return x < 1.0 ? std::pow(x, 5.0f) : x;
}

EvaluationResult SingleGroupMultiOutEvaluation::evaluate(
    const WorkingParameters &params) const
{
	// Calculate the fractional spike count
	const Val nOut = spikeData.nOut * env.burstSize;
	FractionalSpikeCount eval(useIfCondExp, eTar, nOut * 10);
	auto resN = eval.calculate(sN, params);
	auto resNM1 = eval.calculate(sNM1, params);

	// Run a short simulation to get the state the neuron is in at time T
	NullController controller;
	DormandPrinceIntegrator integrator(eTar);
	LastStateRecorder recorder;
	Model::simulate<Model::FAST_EXP | Model::DISABLE_SPIKING |
	                Model::CLAMP_ITH>(useIfCondExp, sN, recorder, controller,
	                                  integrator, params, Time(-1),
	                                  env.T);

	// Calculate the
	const State sRescale = State(100.0, 0.1, 0.1, 0.1);
	const Val delta = ((State() - recorder.state()) * sRescale).sqrL2Norm();
	const Val pReset = exp(-delta);

	// Convert the fractional spike counts to a value between 0 and 1
	static constexpr Val nu = 1;
	const Val pN = dist(correct(resN.fracSpikeCount()), nOut + 0.3, nu);
	const Val pNM1 = dist(correct(resNM1.fracSpikeCount()), 0.0, nu);
	const Val pBin = ((resN.spikeCount == nOut) && (resNM1.spikeCount == 0));
	return EvaluationResult({pN * pNM1 * pReset, pBin, pN, pNM1, pReset,
	                         resN.fracSpikeCount(), resNM1.fracSpikeCount()});
}

const EvaluationResultDescriptor SingleGroupMultiOutEvaluation::descr =
    EvaluationResultDescriptor(EvaluationType::SINGLE_GROUP_MULTI_OUT)
        .add("Soft", "pSoft", "", 0.0, Range(0.0, 1.0), true)
        .add("Binary", "pBin", "", 0.0, Range(0.0, 1.0))
        .add("True Pos.", "pTPos", "", 0.0, Range(0.0, 1.0))
        .add("True Neg.", "pTNeg", "", 0.0, Range(0.0, 1.0))
        .add("Reset", "pReset", "", 0.0, Range(0.0, 1.0))
        .add("#Spike(N)", "cN", "", 0.0, Range::lowerBound(0.0))
        .add("#Spike(N-1)", "cNM1", "", 0.0, Range::lowerBound(0.0));
}
