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
    Database database("supcrt98");

    ChemicalEditor editor(database);
    editor.addAqueousPhase("H O Ca C Cl");
    editor.addMineralPhase("Calcite");

    ChemicalSystem system(editor);

    EquilibriumProblem problem1(system);
    problem1.setTemperature(30.0, "celsius");
    problem1.setPressure(1.0, "bar");
    problem1.add("H2O", 1, "kg");
    problem1.add("CaCO3", 100, "g");

    EquilibriumProblem problem2(system);
    problem2.setTemperature(30.0, "celsius");
    problem2.setPressure(1.0, "bar");
    problem2.add("H2O", 1, "kg");
    problem2.add("CaCO3", 100, "g");
    problem2.add("HCl", 1, "mmol");

    ChemicalState state1 = equilibrate(problem1);
    ChemicalState state2 = equilibrate(problem2);

    EquilibriumPath path(system);

    ChemicalPlot plot0 = path.plot();
    plot0.xdata("pH");
    plot0.ydata("molality element=Ca units=molal");
    plot0.xlabel("pH");
    plot0.ylabel("Concentration [molal]");
    plot0.legend("Ca");

    ChemicalPlot plot1 = path.plot();
    plot1.xdata("amount element=Cl units=mmol");
    plot1.ydata("pH");
    plot1.xlabel("HCl [mmol]");
    plot1.ylabel("pH");
    plot1.nolegend();

    ChemicalOutput output = path.output();
    output.header("HCl [mmol]; Ca [molal]; pH");
    output.data("amount element=Cl units=mmol; molality element=Ca; pH");
    output.file("result.txt");

    path.solve(state1, state2);
}




