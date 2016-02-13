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

#include "Keywords.hpp"

// Reaktoro includes
#include <Reaktoro/Common/Exception.hpp>
#include <Reaktoro/Common/StringUtils.hpp>

namespace Reaktoro {
namespace kwd {

ValueUnits::ValueUnits()
{}

ValueUnits::ValueUnits(double value, std::string units)
: value(value), units(units)
{}

ValueUnits::ValueUnits(std::string str)
{
    auto words = split(str);
    Assert(words.size() == 2, "Could not create a ValueUnits instance from `" + str + "`.",
        "Expecting two words in the format `value units`, e.g., `300 kelvin`, `50 moles`");
    value = tofloat(words[0]);
    units = words[1];
}

EntityValueUnits::EntityValueUnits()
{}

EntityValueUnits::EntityValueUnits(std::string entity, double value, std::string units)
: ValueUnits(value, units), entity(entity)
{}

EntityValueUnits::EntityValueUnits(std::string str)
{
    auto words = split(str);
    Assert(words.size() == 3, "Could not create an EntityValueUnits instance from `" + str + "`.",
        "Expecting three words in the format `entity value units`, e.g., `Calcite 100 g`");
    entity = words[0];
    value = tofloat(words[1]);
    units = words[2];
}

ValueUnitsEntity::ValueUnitsEntity()
{}

ValueUnitsEntity::ValueUnitsEntity(double value, std::string units, std::string entity)
: EntityValueUnits(entity, value, units)
{}

ValueUnitsEntity::ValueUnitsEntity(std::string str)
{
    auto words = split(str);
    Assert(words.size() == 3, "Could not parse `" + str + "`.",
        "Expecting three words in the format `value units entity`, e.g., `1 kg H2O`");
    value = tofloat(words[0]);
    units = words[1];
    entity = words[2];
}

} // namespace kwd
} // namespace Reaktoro
