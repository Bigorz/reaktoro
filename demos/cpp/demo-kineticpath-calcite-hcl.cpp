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

    editor.addAqueousPhase("H2O HCl CaCO3");
    editor.addMineralPhase("Calcite");

    editor.addMineralReaction("Calcite")
        .setEquation("Calcite = Ca++ + CO3--")
        .addMechanism("logk = -5.81 mol/(m2*s); Ea = 23.5 kJ/mol")
        .addMechanism("logk = -0.30 mol/(m2*s); Ea = 14.4 kJ/mol; a[H+] = 1.0")
        .setSpecificSurfaceArea(10, "cm2/g");

    ChemicalSystem system(editor);
    ReactionSystem reactions(editor);

    EquilibriumProblem problem(system);
    problem.setPartition("inert = Calcite");
    problem.add("H2O", 1, "kg");
    problem.add("HCl", 1, "mmol");

    ChemicalState state0 = equilibrate(problem);

    state0.setSpeciesAmount("Calcite", 100, "g");

    KineticPath path(reactions);
    path.setPartition("kinetic = Calcite");

    ChemicalPlot plot1 = path.plot();
    plot1.xdata("t units=minute");
    plot1.ydata("molality element=Ca");
    plot1.xlabel("t [minute]");
    plot1.ylabel("Concentration [molal]");
    plot1.legend("Ca");
    plot1.key("right center");

    ChemicalPlot plot2 = path.plot();
    plot2.xdata("t units=minute");
    plot2.ydata("amount species=Calcite units=g");
    plot2.xlabel("t [minute]");
    plot2.ylabel("Amount [g]");
    plot2.legend("Calcite");

    path.solve(state0, 0, 5, "minute");
}
