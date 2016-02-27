// Reaktoro is a unified framework for modeling chemically reactive systems.
//
// Copyright (C) 2014-2015 Allan Leal
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

#pragma once

// Reaktoro includes
#include <Reaktoro/Common/Outputter.hpp>
#include <Reaktoro/Math/LU.hpp>
#include <Reaktoro/Optimization/KktSolver.hpp>

namespace Reaktoro {

struct OptimumParamsActNewton
{
    /// The threshold for which primal variables lower than it is updated explicitly
    double threshold = 1.0e-14;
};

struct OptimumParamsIpAction
{
    /// The perturbation parameter (or barrier parameter) for the interior-point method
    double mu = 1.0e-20;

    /// The fraction-to-the boundary parameter to relax the line-search backtracking step.
    double tau = 0.9999;
};

struct OptimumParamsIpNewton
{
    /// The perturbation parameter (or barrier parameter) for the interior-point method
    double mu = 1.0e-20;

    /// The fraction-to-the boundary parameter to relax the line-search backtracking step.
    double tau = 0.9999;
};

struct OptimumParamsIpActive
{
    /// The parameter ε for the numerical representation of a zero.
    /// The value of the `i`-th primal variable is considered zero if `x[i] < ε`.
    double epsilon = 1e-20;

    /// The factor τ for the barrier parameter μ defined here as μ = ετ.
    /// The parameter ε is the numerical zero for a primal variable.
    /// @see epsilon
    double tau = 1e-5;
};

struct OptimumParamsIpOpt
{
    /// The interior-point perturbation parameters (default: {1e-8, 1e-16})
    std::vector<double> mu = std::vector<double>({1e-8, 1e-16}); // need explicit initialization list to compile in MSVC 2013

    double delta = 1.0;
    double eta_phi = 1.0e-4;
    double gamma_alpha = 0.05;
    double gamma_phi = 1.0e-5;
    double gamma_theta = 1.0e-5;
    double kappa_epsilon = 10.0;
    double kappa_mu = 0.2;
    double kappa_sigma = 1.0e+10;
    double kappa_soc = 0.99;
    double s_phi = 2.3;
    double s_theta = 1.1;
    double tau_min = 0.9999;
    double theta_mu = 2.0;
    unsigned max_iters_soc = 4;
    bool soc = true;

    /// The factor used to correct the primal initial guess that are too small or on the boundary.
    double mux = 1.0e-5;

    /// The flag that indicates if the KKT problems should be scaled with the matrix sqrt(diag(x))
    bool scaling = true;
};

struct OptimumParamsKarpov
{
    /// The maximum number of iterations for the line search minimization problem.
    double line_search_max_iterations = 3;

    /// The constant for the Wolfe condition of sufficient decrease in the backtracking line search step
    double line_search_wolfe = 1.0e-4;

    // The fraction-to-the-boundary factor used in the feasible step
    double tau_feasible = 0.99;

    // The fraction-to-the-boundary factor used in the descent step
    double tau_descent = 0.9999;

    /// The tolerance for the feasibility problem.
    double feasibility_tolerance = 1.0e-13;

    /// The tolerance for the negative dual variables `z`.
    double negative_dual_tolerance = -1.0e-2;

    /// The value used to remove a primal variable from an active state (i.e., a variable on the bound)
    /// to an inactive state (i.e., a variable in the interior domain).
    double active_to_inactive = 1.0e-6;

    /// The flag that indicates if KktSolver should be used to solve the linear systems
    bool use_kkt_solver = false;
};

struct OptimumParamsRefiner
{
    bool use_lma_setup = true;
};

/// A type that describes the options for the output of a optimisation calculation
struct OptimumOutput : OutputterOptions
{
    /// The prefix for the primal variables `x`.
    std::string xprefix = "x";

    /// The prefix for the dual variables `y`.
    std::string yprefix = "y";

    /// The prefix for the dual variables `z`.
    std::string zprefix = "z";

    /// The names of the primal variables `x`.
    /// Numbers will be used if not properly set (e.g., `x[0]`, `x[1]`)
    std::vector<std::string> xnames;

    /// The names of the dual variables `y`.
    /// Numbers will be used if not properly set (e.g., `y[0]`, `y[1]`)
    std::vector<std::string> ynames;

    /// The names of the dual variables `z`.
    /// Numbers will be used if not properly set (e.g., `z[0]`, `z[1]`)
    std::vector<std::string> znames;
};

/// A type that describes the regularization options for the optimisation calculation
struct OptimumParamsRegularization
{
    /// The regularization parameter @f$\delta@f$ of the linear equality constraints.
    double delta = 0.0;

    /// The regularization parameter @f$\gamma@f$ for bounded solutions.
    double gamma = 0.0;

    /// The boolean flag that indicates if echelonization should be performed.
	/// The echelonization of the linear constraints can help on robustness and
	/// accuracy by minimizing round-off errors.
	bool echelonize = true;

    /// The maximum denominator that can exist in the coefficient matrix `A`.
    /// Set this option to zero if the coefficients in `A` are not represented
    /// by rational numbers. Otherwise, set it to the maximum denominator that can
    /// represent the coefficients in rational form. This is a useful information to
    /// eliminate round-off errors when assembling the regularized coefficient matrix.
    unsigned max_denominator = 0;
};

/// A type that describes the options of a optimisation calculation
struct OptimumOptions
{
    /// The tolerance for the residual of the optimality conditions.
    double tolerance = 1.0e-6;

    /// The tolerance for the variation in primal variables x.
    /// Set this to a value greater than zero to stop the calculation
    /// whenever `max(abs(dx)) < tolerancex`, where `dx` is the current step
    /// of the primal variables.
    double tolerancex = 0.0;

    /// The tolerance for the variation in objective value.
    /// Set this to a value greater than zero to stop the calculation
    /// whenever `abs(f - fprev) < tolerancef`, where `f` and `fprev` are the
    /// current and previous value of the objective function.
    double tolerancef = 0.0;

    /// The maximum number of iterations in the optimisation calculations.
    unsigned max_iterations = 2000;

    /// The options for the output of the optimisation calculations
    OptimumOutput output;

    /// The parameters for the ActNewton algorithm
    OptimumParamsActNewton actnewton;

    /// The parameters for the IpAction algorithm
    OptimumParamsIpAction ipaction;

    /// The parameters for the IpOpt algorithm
    OptimumParamsIpOpt ipopt;

    /// The parameters for the IpNewton algorithm
    OptimumParamsIpNewton ipnewton;

    /// The parameters for the IpActive algorithm
    OptimumParamsIpActive ipactive;

    /// The parameters for the Karpov algorithm
    OptimumParamsKarpov karpov;

    /// The parameters for the Refiner algorithm
    OptimumParamsRefiner refiner;

    /// The regularization options for the optimisation calculation
    OptimumParamsRegularization regularization;

    /// The options for the KKT calculations
    KktOptions kkt;
};

} // namespace Reaktoro
