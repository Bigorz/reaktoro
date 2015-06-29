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

#include "KineticPath.hpp"

// Reaktoro includes
#include <Reaktoro/Common/Exception.hpp>
#include <Reaktoro/Common/Units.hpp>
#include <Reaktoro/Core/ChemicalOutput.hpp>
#include <Reaktoro/Core/ChemicalPlot.hpp>
#include <Reaktoro/Core/ChemicalState.hpp>
#include <Reaktoro/Core/ChemicalSystem.hpp>
#include <Reaktoro/Core/Partition.hpp>
#include <Reaktoro/Core/ReactionSystem.hpp>
#include <Reaktoro/Kinetics/KineticOptions.hpp>
#include <Reaktoro/Kinetics/KineticSolver.hpp>

namespace Reaktoro {

struct KineticPath::Impl
{
    /// The kinetically-controlled chemical reactions
    ReactionSystem reactions;

    /// The chemical system instance
    ChemicalSystem system;

    /// The partition of the species in the chemical system
    Partition partition;

    /// The kinetic solver instance
    KineticSolver solver;

    /// The options of the kinetic path
    KineticOptions options;

    /// The output instance of the kinetic path calculation
    ChemicalOutput output;

    /// The plots of the kinetic path calculation
    std::vector<ChemicalPlot> plots;

    Impl(const ReactionSystem& reactions)
    : reactions(reactions), system(reactions.system()), solver(reactions)
    {
        solver.setPartition(Partition(system));
    }

    auto setOptions(const KineticOptions& options_) -> void
    {
        // Initialise the options of the kinetic path
        options = options_;

        // Set the options of the kinetic solver
        solver.setOptions(options);
    }

    auto setPartition(const Partition& partition) -> void
    {
        solver.setPartition(partition);
    }

    auto setPartition(std::string partition) -> void
    {
        solver.setPartition(partition);
    }

    auto solve(ChemicalState& state, double t0, double t1, std::string units) -> void
    {
        t0 = units::convert(t0, units, "s");
        t1 = units::convert(t1, units, "s");
        solver.initialize(state, t0);

        double t = t0;

        // Initialize the output of the equilibrium path calculation
        if(output) output.open();

        // Initialize the plots of the equilibrium path calculation
        for(auto& plot : plots) plot.open();

        while(t < t1)
        {
            // Update the output with current state
            if(output) output.update(state, t);

            // Update the plots with current state
            for(auto& plot : plots) plot.update(state, t);

            // Integrate one time step only
            solver.step(state, t, t1);
        }

        // Update the output with the final state
        if(output) output.update(state, t1);

        // Update the plots with the final state
        for(auto& plot : plots) plot.update(state, t1);
    }
};

KineticPath::KineticPath(const ReactionSystem& reactions)
: pimpl(new Impl(reactions))
{}

KineticPath::KineticPath(const KineticPath& other)
: pimpl(new Impl(*other.pimpl))
{}

KineticPath::~KineticPath()
{}

auto KineticPath::operator=(KineticPath other) -> KineticPath&
{
    pimpl = std::move(other.pimpl);
    return *this;
}

auto KineticPath::setOptions(const KineticOptions& options) -> void
{
    pimpl->setOptions(options);
}

auto KineticPath::setPartition(const Partition& partition) -> void
{
    pimpl->setPartition(partition);
}

auto KineticPath::setPartition(std::string partition) -> void
{
    pimpl->setPartition(partition);
}

auto KineticPath::solve(ChemicalState& state, double t0, double t1, std::string units) -> void
{
    pimpl->solve(state, t0, t1, units);
}

auto KineticPath::output() -> ChemicalOutput
{
    pimpl->output = ChemicalOutput(pimpl->reactions);
    return pimpl->output;
}

auto KineticPath::plot() -> ChemicalPlot
{
    pimpl->plots.push_back(ChemicalPlot(pimpl->reactions));
    return pimpl->plots.back();
}

auto KineticPath::plots(unsigned num) -> std::vector<ChemicalPlot>
{
    for(unsigned i = 0; i < num; ++i) plot();
    return pimpl->plots;
}

} // namespace Reaktoro
