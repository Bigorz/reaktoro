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
// but WITHOUT ANY WARRANTY without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

#include "PyODE.hpp"

// Boost includes
#include <boost/python.hpp>
namespace py = boost::python;

// Reaktoro includes
#include <Reaktoro/Math/ODE.hpp>

namespace Reaktoro {

auto export_ODE() -> void
{
    py::enum_<ODEStepMode>("ODEStepMode")
        .value("Adams", ODEStepMode::Adams)
        .value("BDF", ODEStepMode::BDF)
        ;

    py::enum_<ODEIterationMode>("ODEIterationMode")
        .value("Functional", ODEIterationMode::Functional)
        .value("Newton", ODEIterationMode::Newton)
        ;

    py::class_<ODEOptions>("ODEOptions")
        .def(py::init<>())
        .def_readwrite("step", &ODEOptions::step)
        .def_readwrite("iteration", &ODEOptions::iteration)
        .def_readwrite("initial_step", &ODEOptions::initial_step)
        .def_readwrite("min_step", &ODEOptions::min_step)
        .def_readwrite("max_step", &ODEOptions::max_step)
        .def_readwrite("reltol", &ODEOptions::reltol)
        .def_readwrite("abstol", &ODEOptions::abstol)
        .def_readwrite("max_error_test_fails", &ODEOptions::max_error_test_fails)
        .def_readwrite("abstols", &ODEOptions::abstols)
        ;

//
//    auto function1 = static_cast<const ODEFunction&(ODEProblem::*)() const>(&ODEProblem::function);
//    auto function2 = static_cast<int(ODEProblem::*)(double, const Vector&, Vector&f) const>(&ODEProblem::function);
//
//    auto jacobian1 = static_cast<const ODEJacobian&(ODEProblem::*)() const>(&ODEProblem::jacobian);
//    auto jacobian2 = static_cast<int(ODEProblem::*)(double, const Vector&, Matrix&f) const>(&ODEProblem::jacobian);
//
//    py::class_<ODEProblem>("ODEProblem")
//        .def(py::init<>())
//        .def("setNumEquations", &ODEProblem::setNumEquations)
//        .def("setFunction", &ODEProblem::setFunction)
//        .def("setJacobian", &ODEProblem::setJacobian)
//        .def("initialized", &ODEProblem::initialized)
//        .def("numEquations", &ODEProblem::numEquations)
//        .def("function", function1, py::return_internal_reference<>())
//        .def("jacobian", jacobian1, py::return_internal_reference<>())
//        .def("function", function2)
//        .def("jacobian", jacobian2)
//        ;
//
//    auto integrate1 = static_cast<void(ODESolver::*)(double&, Vector&)>(&ODESolver::integrate);
//    auto integrate2 = static_cast<void(ODESolver::*)(double&, Vector&, double)>(&ODESolver::integrate);
//
//    py::class_<ODESolver>("ODESolver")
//        .def(py::init<>())
//        .def("setOptions", &ODESolver::setOptions)
//        .def("setProblem", &ODESolver::setProblem)
//        .def("initialize", &ODESolver::initialize)
//        .def("integrate", integrate1)
//        .def("integrate", integrate2)
//        .def("solve", &ODESolver::solve)
//        ;
}

} // namespace Reaktoro
