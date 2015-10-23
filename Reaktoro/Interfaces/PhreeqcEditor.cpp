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

#include "PhreeqcEditor.hpp"

// C++ includes
#include <set>

// Reaktoro includes
#include <Reaktoro/Common/StringUtils.hpp>
#include <Reaktoro/Core/ChemicalSystem.hpp>
#include <Reaktoro/Interfaces/Phreeqc.hpp>

namespace Reaktoro {

struct PhreeqcEditor::Impl
{
    // The name of the database file
    std::string database;

    // The names of the elements used to speciate the aqueous phase
    std::vector<std::string> elements;

    // The names of the gases used to set the gaseous phase
    std::vector<std::string> gases;

    // The names of the minerals used to set the pure mineral phases
    std::vector<std::string> minerals;
};

PhreeqcEditor::PhreeqcEditor()
: pimpl(new Impl())
{
}

PhreeqcEditor::PhreeqcEditor(std::string database)
: PhreeqcEditor()
{
    pimpl->database = database;
}

PhreeqcEditor::PhreeqcEditor(const PhreeqcEditor& other)
: pimpl(new Impl(*other.pimpl))
{}

PhreeqcEditor::~PhreeqcEditor()
{}

auto PhreeqcEditor::operator=(PhreeqcEditor other) -> PhreeqcEditor&
{
    pimpl = std::move(other.pimpl);
    return *this;
}

auto PhreeqcEditor::setAqueousPhase(const std::vector<std::string>& elements) -> void
{
    pimpl->elements = elements;
}

auto PhreeqcEditor::setAqueousPhase(std::string elements) -> void
{
    pimpl->elements = split(elements);
}

auto PhreeqcEditor::setGaseousPhase(const std::vector<std::string>& gases) -> void
{
    pimpl->gases = gases;
}

auto PhreeqcEditor::setGaseousPhase(std::string gases) -> void
{
    pimpl->gases = split(gases);
}

auto PhreeqcEditor::setMineralPhases(const std::vector<std::string>& minerals) -> void
{
    pimpl->minerals = minerals;
}

auto PhreeqcEditor::setMineralPhases(std::string minerals) -> void
{
    pimpl->minerals = split(minerals);
}

PhreeqcEditor::operator ChemicalSystem() const
{
    Phreeqc phreeqc = *this;
    ChemicalSystem system = phreeqc;
    return system;
}

PhreeqcEditor::operator Phreeqc() const
{
    // The indentation string
    const std::string indent = "    ";

    // The set of ignored elements
    const std::set<std::string> ignored_elements = {"H"};

    // Create the auxiliary input string
    std::string input;

    // Define the SOLUTION block
    input = "SOLUTION\n"
            "    units   ppm\n";
    for(auto element : pimpl->elements)
        if(!ignored_elements.count(element))
            input += indent + element + " 1.0\n";

    // Define the EQUILIBRIUM_PHASES block containing the minerals
    input += "EQUILIBRIUM_PHASES\n";
    for(auto mineral : pimpl->minerals)
        input += indent + mineral + " 0.0\n";

    // Define the GAS_PHASE block containing the gases
    input += "GAS_PHASE\n";
    for(auto gas : pimpl->gases)
        input += indent + gas + " 0.0\n";

    // End the input script
    input += "END\n";

    // Create the Phreeqc instance with available database and create input script
    Phreeqc phreeqc(pimpl->database);
    phreeqc.execute(input);

    return phreeqc;
}

} // namespace Reaktoro

