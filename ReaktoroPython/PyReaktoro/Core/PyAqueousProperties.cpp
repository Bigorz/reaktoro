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

#include "PyChemicalProperties.hpp"

// Boost includes
#include <boost/python.hpp>
namespace py = boost::python;

// Reaktoro includes
#include <Reaktoro/Core/AqueousProperties.hpp>
#include <Reaktoro/Core/ChemicalProperties.hpp>
#include <Reaktoro/Core/ChemicalSystem.hpp>

namespace Reaktoro {

auto export_AqueousProperties() -> void
{
    auto update1 = static_cast<void (AqueousProperties::*)(double, double, const Vector&)>(&AqueousProperties::update);
    auto update2 = static_cast<void (AqueousProperties::*)(const ChemicalProperties&)>(&AqueousProperties::update);

    auto pE1 = static_cast<ChemicalScalar (AqueousProperties::*)() const>(&AqueousProperties::pE);
    auto pE2 = static_cast<ChemicalScalar (AqueousProperties::*)(std::string) const>(&AqueousProperties::pE);

    auto Eh1 = static_cast<ChemicalScalar (AqueousProperties::*)() const>(&AqueousProperties::Eh);
    auto Eh2 = static_cast<ChemicalScalar (AqueousProperties::*)(std::string) const>(&AqueousProperties::Eh);

    py::class_<AqueousProperties>("AqueousProperties")
        .def(py::init<>())
        .def(py::init<const ChemicalSystem&>())
        .def("update", update1)
        .def("update", update2)
        .def("temperature", &AqueousProperties::temperature)
        .def("pressure", &AqueousProperties::pressure)
        .def("composition", &AqueousProperties::composition, py::return_internal_reference<>())
        .def("pH", &AqueousProperties::pH)
        .def("pe", pE1)
        .def("pe", pE2)
        .def("Eh", Eh1)
        .def("Eh", Eh2)
        ;
}

} // namespace Reaktoro
