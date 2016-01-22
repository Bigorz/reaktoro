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

#include "PyOptimumOptions.hpp"

// Boost includes
#include <boost/python.hpp>
namespace py = boost::python;

// Reaktoro includes
#include <Reaktoro/Optimization/OptimumOptions.hpp>

namespace Reaktoro {

auto export_OptimumOptions() -> void
{
    py::class_<OptimumParamsIpNewton>("OptimumParamsIpNewton")
        .def_readwrite("mu", &OptimumParamsIpNewton::mu)
        .def_readwrite("tau", &OptimumParamsIpNewton::tau)
        ;

    py::class_<OptimumParamsIpAction>("OptimumParamsIpAction")
        .def_readwrite("mu", &OptimumParamsIpAction::mu)
        .def_readwrite("tau", &OptimumParamsIpAction::tau)
        ;

    py::class_<OptimumParamsKarpov>("OptimumParamsKarpov")
        .def_readwrite("line_search_max_iterations", &OptimumParamsKarpov::line_search_max_iterations)
        .def_readwrite("line_search_wolfe", &OptimumParamsKarpov::line_search_wolfe)
        .def_readwrite("tau_feasible", &OptimumParamsKarpov::tau_feasible)
        .def_readwrite("tau_descent", &OptimumParamsKarpov::tau_descent)
        .def_readwrite("feasibility_tolerance", &OptimumParamsKarpov::feasibility_tolerance)
        .def_readwrite("negative_dual_tolerance", &OptimumParamsKarpov::negative_dual_tolerance)
        .def_readwrite("active_to_inactive", &OptimumParamsKarpov::active_to_inactive)
        .def_readwrite("use_kkt_solver", &OptimumParamsKarpov::use_kkt_solver)
        ;

    py::class_<OptimumParamsIpOpt>("OptimumParamsIpOpt")
        .def_readwrite("mu", &OptimumParamsIpOpt::mu)
        .def_readwrite("delta", &OptimumParamsIpOpt::delta)
        .def_readwrite("eta_phi", &OptimumParamsIpOpt::eta_phi)
        .def_readwrite("gamma_alpha", &OptimumParamsIpOpt::gamma_alpha)
        .def_readwrite("gamma_phi", &OptimumParamsIpOpt::gamma_phi)
        .def_readwrite("gamma_theta", &OptimumParamsIpOpt::gamma_theta)
        .def_readwrite("kappa_epsilon", &OptimumParamsIpOpt::kappa_epsilon)
        .def_readwrite("kappa_mu", &OptimumParamsIpOpt::kappa_mu)
        .def_readwrite("kappa_sigma", &OptimumParamsIpOpt::kappa_sigma)
        .def_readwrite("kappa_soc", &OptimumParamsIpOpt::kappa_soc)
        .def_readwrite("s_phi", &OptimumParamsIpOpt::s_phi)
        .def_readwrite("s_theta", &OptimumParamsIpOpt::s_theta)
        .def_readwrite("tau_min", &OptimumParamsIpOpt::tau_min)
        .def_readwrite("theta_mu", &OptimumParamsIpOpt::theta_mu)
        .def_readwrite("max_iters_soc", &OptimumParamsIpOpt::max_iters_soc)
        .def_readwrite("soc", &OptimumParamsIpOpt::soc)
        .def_readwrite("mux", &OptimumParamsIpOpt::mux)
        .def_readwrite("scaling", &OptimumParamsIpOpt::scaling)
        ;

    py::class_<OptimumOutput, py::bases<OutputterOptions>>("OptimumOutput")
        .def_readwrite("xprefix", &OptimumOutput::xprefix)
        .def_readwrite("yprefix", &OptimumOutput::yprefix)
        .def_readwrite("zprefix", &OptimumOutput::zprefix)
        .def_readwrite("xnames", &OptimumOutput::xnames)
        .def_readwrite("ynames", &OptimumOutput::ynames)
        .def_readwrite("znames", &OptimumOutput::znames)
        ;

    py::class_<OptimumOptions>("OptimumOptions")
        .def_readwrite("tolerance", &OptimumOptions::tolerance)
        .def_readwrite("max_iterations", &OptimumOptions::max_iterations)
        .def_readwrite("output", &OptimumOptions::output)
        .def_readwrite("ipopt", &OptimumOptions::ipopt)
        .def_readwrite("ipnewton", &OptimumOptions::ipnewton)
        ;
}

} // namespace Reaktoro
