// Reaktoro is a C++ library for computational reaction modelling.
//
// Copyright (C) 2014 Allan Leal
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

// Boost includes
#include <boost/python.hpp>
namespace py = boost::python;

// PyReaktoro includes
#include <PyReaktoro/PyCommon.hpp>
#include <PyReaktoro/PyCore.hpp>
#include <PyReaktoro/PyEquilibrium.hpp>
#include <PyReaktoro/PyInterfaces.hpp>
#include <PyReaktoro/PyKinetics.hpp>
#include <PyReaktoro/PyMath.hpp>
#include <PyReaktoro/PyOptimization.hpp>
#include <PyReaktoro/PyReactions.hpp>
#include <PyReaktoro/PyThermodynamics.hpp>

BOOST_PYTHON_MODULE(reaktoro)
{
    // Set numpy as the numeric::array engine
    py::numeric::array::set_module_and_type("numpy", "ndarray");

    // The following export order matters (e.g., Optimization module needs to be exported before Equilibrium module)
    Reaktoro::export_Common();
    Reaktoro::export_Core();
    Reaktoro::export_Interfaces();
    Reaktoro::export_Optimization();
    Reaktoro::export_Equilibrium();
    Reaktoro::export_Kinetics();
    Reaktoro::export_Math();
    Reaktoro::export_Reactions();
    Reaktoro::export_Thermodynamics();
}
