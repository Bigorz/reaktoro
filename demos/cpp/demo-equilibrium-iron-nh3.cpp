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

#include <Reaktoro/Reaktoro.hpp>
using namespace Reaktoro;

int main()
{
    Database database("supcrt98.xml");

    ChemicalEditor editor(database);
    editor.addAqueousPhase("H2O Fe(OH)2 Fe(OH)3 NH3");
    editor.addGaseousPhase("NH3(g)");
    editor.addMineralPhase("Magnetite");

    ChemicalSystem system(editor);

    std::cout << system << std::endl;

    return 0;

    EquilibriumProblem problem(system);
    problem.setTemperature(60, "celsius");
    problem.setPressure(300, "bar");
    problem.add("H2O", 1, "kg");
    problem.add("CO2", 100, "g");
    problem.add("NaCl", 0.1, "mol");

    ChemicalState state = equilibrate(problem);

    std::cout << state << std::endl;
}
