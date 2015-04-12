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

#include "PyOptimumState.hpp"

// Boost includes
#include <boost/python.hpp>
namespace py = boost::python;

// Reaktoro includes
#include <Reaktoro/Optimization/OptimumState.hpp>

namespace Reaktoro {

auto export_OptimumState() -> void
{
    py::class_<OptimumState>("OptimumState")
        .def_readwrite("x", &OptimumState::x)
        .def_readwrite("y", &OptimumState::y)
        .def_readwrite("z", &OptimumState::z)
        .def_readwrite("f", &OptimumState::f)
        .def_readwrite("g", &OptimumState::g)
        ;
}

} // namespace Reaktoro
