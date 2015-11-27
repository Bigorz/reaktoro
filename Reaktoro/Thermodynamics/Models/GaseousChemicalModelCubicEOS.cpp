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

#include "GaseousChemicalModelCubicEOS.hpp"

// C++ includes
#include <map>

// Reaktoro includes
#include <Reaktoro/Common/Exception.hpp>
#include <Reaktoro/Thermodynamics/EOS/CubicEOS.hpp>
#include <Reaktoro/Thermodynamics/Mixtures/GaseousMixture.hpp>

namespace Reaktoro {
namespace {

auto gaseousChemicalModelCubicEOS(const GaseousMixture& mixture, CubicEOS::Model modeltype) -> PhaseChemicalModel
{
    // The number of gases in the mixture
    const unsigned nspecies = mixture.numSpecies();

    // Get the the critical temperatures, pressures and acentric factors of the gases
    std::vector<double> Tc, Pc, omega;
    for(GaseousSpecies species : mixture.species())
    {
        Tc.push_back(species.criticalTemperature());
        Pc.push_back(species.criticalPressure());
        omega.push_back(species.acentricFactor());
    }

    // Initialize the CubicEOS instance
    CubicEOS eos(nspecies);
    eos.setPhaseAsVapor();
    eos.setCriticalTemperatures(Tc);
    eos.setCriticalPressures(Pc);
    eos.setAcentricFactors(omega);
    eos.setModel(modeltype);

    // Define the chemical model function of the gaseous phase
    auto model = [=](const GaseousMixtureState state) mutable
    {
        // Auxiliary references
        const auto& T = state.T;
        const auto& P = state.P;
        const auto& x = state.x;

        // Evaluate the CubicEOS object function
        const CubicEOS::Result eosres = eos(T, P, x);

        // The ln of molar fractions
        const ChemicalVector ln_x = log(x);

        // The ln of pressure in bar units
        const ThermoScalar ln_Pbar = log(1e-5 * P);

        // Create an alias to the ln fugacity coefficients
        const auto& ln_phi = eosres.ln_fugacity_coefficients;

        PhaseChemicalModelResult res(nspecies);
        res.ln_activity_coefficients = ln_phi;
        res.ln_activities = ln_phi + ln_x + ln_Pbar;
        res.molar_volume = eosres.molar_volume;
        res.residual_molar_gibbs_energy = eosres.residual_molar_gibbs_energy;
        res.residual_molar_enthalpy = eosres.residual_molar_enthalpy;
        res.residual_molar_heat_capacity_cp = eosres.residual_molar_heat_capacity_cp;
        res.residual_molar_heat_capacity_cv = eosres.residual_molar_heat_capacity_cv;

        return res;
    };

    // Define the chemical model function of the gaseous phase
    PhaseChemicalModel f = [=](Temperature T, Pressure P, const Vector& n) mutable
    {
        // Calculate state of the mixture
        const GaseousMixtureState state = mixture.state(T, P, n);

        return model(state);
    };

    return f;
}

} // namespace

auto gaseousChemicalModelVanDerWaals(const GaseousMixture& mixture) -> PhaseChemicalModel
{
    return gaseousChemicalModelCubicEOS(mixture, CubicEOS::VanDerWaals);
}

auto gaseousChemicalModelRedlichKwong(const GaseousMixture& mixture) -> PhaseChemicalModel
{
    return gaseousChemicalModelCubicEOS(mixture, CubicEOS::RedlichKwong);
}

auto gaseousChemicalModelSoaveRedlichKwong(const GaseousMixture& mixture) -> PhaseChemicalModel
{
    return gaseousChemicalModelCubicEOS(mixture, CubicEOS::SoaveRedlichKwong);
}

auto gaseousChemicalModelPengRobinson(const GaseousMixture& mixture) -> PhaseChemicalModel
{
    return gaseousChemicalModelCubicEOS(mixture, CubicEOS::PengRobinson);
}

} // namespace Reaktoro
