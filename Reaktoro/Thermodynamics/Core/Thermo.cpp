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

#include "Thermo.hpp"

// C++ includes
#include <functional>
using namespace std::placeholders;

// Reaktoro includes
#include <Reaktoro/Common/Constants.hpp>
#include <Reaktoro/Common/NamingUtils.hpp>
#include <Reaktoro/Common/OptimizationUtils.hpp>
#include <Reaktoro/Common/ReactionEquation.hpp>
#include <Reaktoro/Common/ThermoScalar.hpp>
#include <Reaktoro/Common/Units.hpp>
#include <Reaktoro/Thermodynamics/Core/Database.hpp>
#include <Reaktoro/Thermodynamics/Models/SpeciesElectroState.hpp>
#include <Reaktoro/Thermodynamics/Models/SpeciesElectroStateHKF.hpp>
#include <Reaktoro/Thermodynamics/Models/SpeciesThermoState.hpp>
#include <Reaktoro/Thermodynamics/Models/SpeciesThermoStateHKF.hpp>
#include <Reaktoro/Thermodynamics/Species/AqueousSpecies.hpp>
#include <Reaktoro/Thermodynamics/Species/GaseousSpecies.hpp>
#include <Reaktoro/Thermodynamics/Species/MineralSpecies.hpp>
#include <Reaktoro/Thermodynamics/Water/WaterElectroState.hpp>
#include <Reaktoro/Thermodynamics/Water/WaterElectroStateJohnsonNorton.hpp>
#include <Reaktoro/Thermodynamics/Water/WaterThermoState.hpp>
#include <Reaktoro/Thermodynamics/Water/WaterThermoStateUtils.hpp>

namespace Reaktoro {
namespace {

/// The signature of a function that calculates the thermodynamic state of water
using WaterThermoStateFunction =
    std::function<WaterThermoState(double, double)>;

/// The signature of a function that calculates the thermodynamic state of a species
using SpeciesThermoStateFunction =
    std::function<SpeciesThermoState(double, double, std::string)>;

/// The signature of a function that calculates the electrostatic state of water
using WaterElectroStateFunction =
    std::function<WaterElectroState(double, double)>;

auto errorNonExistentSpecies(const std::string& name) -> void
{
    Exception exception;
    exception.error << "Cannot get an instance of the species `" << name << "` in the database.";
    exception.reason << "There is no such species in the database.";
    RaiseError(exception);
}

} // namespace

struct Thermo::Impl
{
    /// The database instance
    Database database;

    /// The Haar-Gallagher-Kell (1984) equation of state for water
    WaterThermoStateFunction water_thermo_state_hgk_fn;

    /// The Wagner and Pruss (1995) equation of state for water
    WaterThermoStateFunction water_thermo_state_wagner_pruss_fn;

    /// The Johnson and Norton equation of state for the electrostatic state of water
    WaterElectroStateFunction water_eletro_state_fn;

    /// The HKF equation of state for the thermodynamic state of aqueous, gaseous and mineral species
    SpeciesThermoStateFunction species_thermo_state_hkf_fn;

    /// The temperature units for the calculations
    std::string temperature_units = "kelvin";

    /// The pressure units for the calculations
    std::string pressure_units = "pascal";

    Impl()
    {}

    Impl(const Database& database)
    : database(database)
    {
        // Initialize the Haar-Gallagher-Kell (1984) equation of state for water
        water_thermo_state_hgk_fn = [](double T, double P)
        {
            return Reaktoro::waterThermoStateHGK(T, P);
        };

        water_thermo_state_hgk_fn = memoize(water_thermo_state_hgk_fn);

        // Initialize the Wagner and Pruss (1995) equation of state for water
        water_thermo_state_wagner_pruss_fn = [](double T, double P)
        {
            return Reaktoro::waterThermoStateWagnerPruss(T, P);
        };

        water_thermo_state_wagner_pruss_fn = memoize(water_thermo_state_wagner_pruss_fn);

        // Initialize the Johnson and Norton equation of state for the electrostatic state of water
        water_eletro_state_fn = [=](double T, double P)
        {
            const WaterThermoState wts = water_thermo_state_wagner_pruss_fn(T, P);
            return waterElectroStateJohnsonNorton(T, P, wts);
        };

        water_eletro_state_fn = memoize(water_eletro_state_fn);

        // Initialize the HKF equation of state for the thermodynamic state of aqueous, gaseous and mineral species
        species_thermo_state_hkf_fn = [=](double T, double P, std::string species)
        {
            return speciesThermoStateHKF(T, P, species);
        };

        species_thermo_state_hkf_fn = memoize(species_thermo_state_hkf_fn);
    }

    auto setTemperatureUnits(std::string units) -> void
    {
        temperature_units = units;
    }

    auto setPressureUnits(std::string units) -> void
    {
        pressure_units = units;
    }

    auto convertUnits(double& T, double& P) -> void
    {
        // These units conversion functions are expensive, so use it only when needed (i.e., not standard T, P units).
        if(temperature_units != "kelvin")
            T = units::convert(T, temperature_units, "kelvin");
        if(pressure_units != "pascal")
            P = units::convert(P, pressure_units, "pascal");
    }

    auto speciesThermoStateHKF(double T, double P, std::string species) -> SpeciesThermoState
    {
        if(database.containsAqueousSpecies(species))
            return aqueousSpeciesThermoStateHKF(T, P, database.aqueousSpecies(species));
        if(database.containsGaseousSpecies(species))
            return Reaktoro::speciesThermoStateHKF(T, P, database.gaseousSpecies(species));
        if(database.containsMineralSpecies(species))
            return Reaktoro::speciesThermoStateHKF(T, P, database.mineralSpecies(species));
        errorNonExistentSpecies(species);
        return {};
    }

    auto aqueousSpeciesThermoStateHKF(double T, double P, const AqueousSpecies& species) -> SpeciesThermoState
    {
        const WaterThermoState wts = water_thermo_state_wagner_pruss_fn(T, P);

        if(isAlternativeWaterName(species.name()))
            return speciesThermoStateSolventHKF(T, P, wts);

        const WaterElectroState wes = water_eletro_state_fn(T, P);

        FunctionG g = functionG(T, P, wts);

        SpeciesElectroState aes = speciesElectroStateHKF(g, species);

        return speciesThermoStateSoluteHKF(T, P, species, aes, wes);
    }

    auto standardPartialMolarGibbsEnergy(double T, double P, std::string species) -> ThermoScalar
    {
        const auto species_thermo_properties = getSpeciesInterpolatedThermoProperties(species);
        if(!species_thermo_properties.empty())
            if(!species_thermo_properties().gibbs_energy.empty())
                return ThermoScalar(species_thermo_properties().gibbs_energy(T, P), 0.0, 0.0);

        const auto reaction_thermo_properties = getReactionInterpolatedThermoProperties(species);
        if(!reaction_thermo_properties.empty())
            if(!reaction_thermo_properties.get().gibbs_energy.empty())
                return standardGibbsEnergyFromReaction(T, P, species, reaction_thermo_properties.get());

        if(hasThermoParamsHKF(species))
            return species_thermo_state_hkf_fn(T, P, species).gibbs_energy;

        Exception exception;
        exception.error << "Cannot calculate the standard Gibbs energy of species `" << species << "`.";
        exception.reason << "The species instance has no thermodynamic data for such calculation.";
        RaiseError(exception);
        return {};
    }

    auto standardPartialMolarHelmholtzEnergy(double T, double P, std::string species) -> ThermoScalar
    {
        const auto species_thermo_properties = getSpeciesInterpolatedThermoProperties(species);
        if(!species_thermo_properties.empty())
            if(!species_thermo_properties().helmholtz_energy.empty())
                return ThermoScalar(species_thermo_properties().helmholtz_energy(T, P), 0.0, 0.0);

        const auto reaction_thermo_properties = getReactionInterpolatedThermoProperties(species);
        if(!reaction_thermo_properties.empty())
            if(!reaction_thermo_properties.get().helmholtz_energy.empty())
                return standardHelmholtzEnergyFromReaction(T, P, species, reaction_thermo_properties.get());

        if(hasThermoParamsHKF(species))
            return species_thermo_state_hkf_fn(T, P, species).helmholtz_energy;

        Exception exception;
        exception.error << "Cannot calculate the standard Helmholtz energy of species `" << species << "`.";
        exception.reason << "The species instance has no thermodynamic data for such calculation.";
        RaiseError(exception);
        return {};
    }

    auto standardPartialMolarInternalEnergy(double T, double P, std::string species) -> ThermoScalar
    {
        const auto species_thermo_properties = getSpeciesInterpolatedThermoProperties(species);
        if(!species_thermo_properties.empty())
            if(!species_thermo_properties().internal_energy.empty())
                return ThermoScalar(species_thermo_properties().internal_energy(T, P), 0.0, 0.0);

        const auto reaction_thermo_properties = getReactionInterpolatedThermoProperties(species);
        if(!reaction_thermo_properties.empty())
            if(!reaction_thermo_properties.get().internal_energy.empty())
                return standardInternalEnergyFromReaction(T, P, species, reaction_thermo_properties.get());

        if(hasThermoParamsHKF(species))
            return species_thermo_state_hkf_fn(T, P, species).internal_energy;

        Exception exception;
        exception.error << "Cannot calculate the standard internal energy of species `" << species << "`.";
        exception.reason << "The species instance has no thermodynamic data for such calculation.";
        RaiseError(exception);
        return {};
    }

    auto standardPartialMolarEnthalpy(double T, double P, std::string species) -> ThermoScalar
    {
        const auto species_thermo_properties = getSpeciesInterpolatedThermoProperties(species);
        if(!species_thermo_properties.empty())
            if(!species_thermo_properties().enthalpy.empty())
                return ThermoScalar(species_thermo_properties().enthalpy(T, P), 0.0, 0.0);

        const auto reaction_thermo_properties = getReactionInterpolatedThermoProperties(species);
        if(!reaction_thermo_properties.empty())
            if(!reaction_thermo_properties.get().enthalpy.empty())
                return standardEnthalpyFromReaction(T, P, species, reaction_thermo_properties.get());

        if(hasThermoParamsHKF(species))
            return species_thermo_state_hkf_fn(T, P, species).enthalpy;

        Exception exception;
        exception.error << "Cannot calculate the standard enthalpy of species `" << species << "`.";
        exception.reason << "The species instance has no thermodynamic data for such calculation.";
        RaiseError(exception);
        return {};
    }

    auto standardPartialMolarEntropy(double T, double P, std::string species) -> ThermoScalar
    {
        const auto species_thermo_properties = getSpeciesInterpolatedThermoProperties(species);
        if(!species_thermo_properties.empty())
            if(!species_thermo_properties().entropy.empty())
                return ThermoScalar(species_thermo_properties().entropy(T, P), 0.0, 0.0);

        const auto reaction_thermo_properties = getReactionInterpolatedThermoProperties(species);
        if(!reaction_thermo_properties.empty())
            if(!reaction_thermo_properties.get().entropy.empty())
                return standardEntropyFromReaction(T, P, species, reaction_thermo_properties.get());

        if(hasThermoParamsHKF(species))
            return species_thermo_state_hkf_fn(T, P, species).entropy;

        Exception exception;
        exception.error << "Cannot calculate the standard entropy of species `" << species << "`.";
        exception.reason << "The species instance has no thermodynamic data for such calculation.";
        RaiseError(exception);
        return {};
    }

    auto standardPartialMolarVolume(double T, double P, std::string species) -> ThermoScalar
    {
        const auto species_thermo_properties = getSpeciesInterpolatedThermoProperties(species);
        if(!species_thermo_properties.empty())
            if(!species_thermo_properties().volume.empty())
                return ThermoScalar(species_thermo_properties().volume(T, P), 0.0, 0.0);

        const auto reaction_thermo_properties = getReactionInterpolatedThermoProperties(species);
        if(!reaction_thermo_properties.empty())
            if(!reaction_thermo_properties.get().volume.empty())
                return standardVolumeFromReaction(T, P, species, reaction_thermo_properties.get());

        if(hasThermoParamsHKF(species))
            return species_thermo_state_hkf_fn(T, P, species).volume;

        Exception exception;
        exception.error << "Cannot calculate the standard volume of species `" << species << "`.";
        exception.reason << "The species instance has no thermodynamic data for such calculation.";
        RaiseError(exception);
        return {};
    }

    auto standardPartialMolarHeatCapacityConstP(double T, double P, std::string species) -> ThermoScalar
    {
        const auto species_thermo_properties = getSpeciesInterpolatedThermoProperties(species);
        if(!species_thermo_properties.empty())
            if(!species_thermo_properties().heat_capacity_cp.empty())
                return ThermoScalar(species_thermo_properties().heat_capacity_cp(T, P), 0.0, 0.0);

        const auto reaction_thermo_properties = getReactionInterpolatedThermoProperties(species);
        if(!reaction_thermo_properties.empty())
            if(!reaction_thermo_properties.get().heat_capacity_cp.empty())
                return standardHeatCapacityConstPFromReaction(T, P, species, reaction_thermo_properties.get());

        if(hasThermoParamsHKF(species))
            return species_thermo_state_hkf_fn(T, P, species).heat_capacity_cp;

        Exception exception;
        exception.error << "Cannot calculate the standard isobaric heat capacity of species `" << species << "`.";
        exception.reason << "The species instance has no thermodynamic data for such calculation.";
        RaiseError(exception);
        return {};
    }

    auto standardPartialMolarHeatCapacityConstV(double T, double P, std::string species) -> ThermoScalar
    {
        const auto species_thermo_properties = getSpeciesInterpolatedThermoProperties(species);
        if(!species_thermo_properties.empty())
            if(!species_thermo_properties().heat_capacity_cv.empty())
                return ThermoScalar(species_thermo_properties().heat_capacity_cv(T, P), 0.0, 0.0);

        const auto reaction_thermo_properties = getReactionInterpolatedThermoProperties(species);
        if(!reaction_thermo_properties.empty())
            if(!reaction_thermo_properties.get().heat_capacity_cv.empty())
                return standardHeatCapacityConstVFromReaction(T, P, species, reaction_thermo_properties.get());

        if(hasThermoParamsHKF(species))
            return species_thermo_state_hkf_fn(T, P, species).heat_capacity_cv;

        Exception exception;
        exception.error << "Cannot calculate the standard isochoric heat capacity of species `" << species << "`.";
        exception.reason << "The species instance has no thermodynamic data for such calculation.";
        RaiseError(exception);
        return {};
    }

    auto getSpeciesInterpolatedThermoProperties(std::string species) -> Optional<SpeciesThermoInterpolatedProperties>
    {
        if(database.containsAqueousSpecies(species))
            return database.aqueousSpecies(species).thermoData().properties;
        if(database.containsGaseousSpecies(species))
            return database.gaseousSpecies(species).thermoData().properties;
        if(database.containsMineralSpecies(species))
            return database.mineralSpecies(species).thermoData().properties;
        errorNonExistentSpecies(species);
        return {};
    }

    auto getReactionInterpolatedThermoProperties(std::string species) -> Optional<ReactionThermoInterpolatedProperties>
    {
        if(database.containsAqueousSpecies(species))
            return database.aqueousSpecies(species).thermoData().reaction;
        if(database.containsGaseousSpecies(species))
            return database.gaseousSpecies(species).thermoData().reaction;
        if(database.containsMineralSpecies(species))
            return database.mineralSpecies(species).thermoData().reaction;
        errorNonExistentSpecies(species);
        return {};
    }

    auto hasThermoParamsHKF(std::string species) -> bool
    {
        if(isAlternativeWaterName(species)) return true;
        if(database.containsAqueousSpecies(species))
            return !database.aqueousSpecies(species).thermoData().hkf.empty();
        if(database.containsGaseousSpecies(species))
            return !database.gaseousSpecies(species).thermoData().hkf.empty();
        if(database.containsMineralSpecies(species))
            return !database.mineralSpecies(species).thermoData().hkf.empty();
        errorNonExistentSpecies(species);
        return {};
    }

    template<typename PropertyFunction, typename EvalFunction>
    auto standardPropertyFromReaction(double T, double P, std::string species,
        const ReactionThermoInterpolatedProperties& reaction, PropertyFunction property,
        EvalFunction eval) -> ThermoScalar
    {
        const double stoichiometry = reaction.equation.stoichiometry(species);

        Assert(stoichiometry, "Cannot calculate the thermodynamic property of "
            "species `" + species + "` using its reaction data.", "This species "
            "is not present in the reaction equation `" +
            std::string(reaction.equation) + "` or has zero stoichiometry.");

        double sum = 0.0;
        for(auto pair : reaction.equation.equation())
        {
            const auto reactant = pair.first;
            const auto stoichiometry = pair.second;
            if(reactant != species)
                sum -= stoichiometry * property(T, P, reactant).val;
        }
        sum += eval();
        sum /= stoichiometry;

        return {sum, 0.0, 0.0};
    }

    auto standardGibbsEnergyFromReaction(double T, double P, std::string species, const ReactionThermoInterpolatedProperties& reaction) -> ThermoScalar
    {
        auto eval = [&]() { return reaction.gibbs_energy(T, P); };
        auto property = std::bind(&Impl::standardPartialMolarGibbsEnergy, *this, _1, _2, _3);
        return standardPropertyFromReaction(T, P, species, reaction, property, eval);
    }

    auto standardHelmholtzEnergyFromReaction(double T, double P, std::string species, const ReactionThermoInterpolatedProperties& reaction) -> ThermoScalar
    {
        auto eval = [&]() { return reaction.helmholtz_energy(T, P); };
        auto property = std::bind(&Impl::standardPartialMolarHelmholtzEnergy, *this, _1, _2, _3);
        return standardPropertyFromReaction(T, P, species, reaction, property, eval);
    }

    auto standardInternalEnergyFromReaction(double T, double P, std::string species, const ReactionThermoInterpolatedProperties& reaction) -> ThermoScalar
    {
        auto eval = [&]() { return reaction.internal_energy(T, P); };
        auto property = std::bind(&Impl::standardPartialMolarInternalEnergy, *this, _1, _2, _3);
        return standardPropertyFromReaction(T, P, species, reaction, property, eval);
    }

    auto standardEnthalpyFromReaction(double T, double P, std::string species, const ReactionThermoInterpolatedProperties& reaction) -> ThermoScalar
    {
        auto eval = [&]() { return reaction.enthalpy(T, P); };
        auto property = std::bind(&Impl::standardPartialMolarEnthalpy, *this, _1, _2, _3);
        return standardPropertyFromReaction(T, P, species, reaction, property, eval);
    }

    auto standardEntropyFromReaction(double T, double P, std::string species, const ReactionThermoInterpolatedProperties& reaction) -> ThermoScalar
    {
        auto eval = [&]() { return reaction.entropy(T, P); };
        auto property = std::bind(&Impl::standardPartialMolarEntropy, *this, _1, _2, _3);
        return standardPropertyFromReaction(T, P, species, reaction, property, eval);
    }

    auto standardVolumeFromReaction(double T, double P, std::string species, const ReactionThermoInterpolatedProperties& reaction) -> ThermoScalar
    {
        auto eval = [&]() { return reaction.volume(T, P); };
        auto property = std::bind(&Impl::standardPartialMolarVolume, *this, _1, _2, _3);
        return standardPropertyFromReaction(T, P, species, reaction, property, eval);
    }

    auto standardHeatCapacityConstPFromReaction(double T, double P, std::string species, const ReactionThermoInterpolatedProperties& reaction) -> ThermoScalar
    {
        auto eval = [&]() { return reaction.heat_capacity_cp(T, P); };
        auto property = std::bind(&Impl::standardPartialMolarHeatCapacityConstP, *this, _1, _2, _3);
        return standardPropertyFromReaction(T, P, species, reaction, property, eval);
    }

    auto standardHeatCapacityConstVFromReaction(double T, double P, std::string species, const ReactionThermoInterpolatedProperties& reaction) -> ThermoScalar
    {
        auto eval = [&]() { return reaction.heat_capacity_cv(T, P); };
        auto property = std::bind(&Impl::standardPartialMolarHeatCapacityConstV, *this, _1, _2, _3);
        return standardPropertyFromReaction(T, P, species, reaction, property, eval);
    }

    auto lnEquilibriumConstant(double T, double P, std::string reaction) -> ThermoScalar
    {
        ReactionEquation equation(reaction);
        const double R = universalGasConstant;
        ThermoScalar lnK;
        for(auto pair : equation.equation())
            lnK += pair.second * standardPartialMolarGibbsEnergy(T, P, pair.first);
        lnK /= -R*T;
        return lnK;
    }

    auto logEquilibriumConstant(double T, double P, std::string reaction) -> ThermoScalar
    {
        const double ln10 = 2.302585092994046;
        const ThermoScalar lnK = lnEquilibriumConstant(T, P, reaction);
        return lnK/ln10;
    }
};

Thermo::Thermo(const Database& database)
: pimpl(new Impl(database))
{}

auto Thermo::setTemperatureUnits(std::string units) -> void
{
    pimpl->setTemperatureUnits(units);
}

auto Thermo::setPressureUnits(std::string units) -> void
{
    pimpl->setPressureUnits(units);
}

auto Thermo::standardPartialMolarGibbsEnergy(double T, double P, std::string species) const -> ThermoScalar
{
    pimpl->convertUnits(T, P);
    return pimpl->standardPartialMolarGibbsEnergy(T, P, species);
}

auto Thermo::standardPartialMolarHelmholtzEnergy(double T, double P, std::string species) const -> ThermoScalar
{
    pimpl->convertUnits(T, P);
    return pimpl->standardPartialMolarHelmholtzEnergy(T, P, species);
}

auto Thermo::standardPartialMolarInternalEnergy(double T, double P, std::string species) const -> ThermoScalar
{
    pimpl->convertUnits(T, P);
    return pimpl->standardPartialMolarInternalEnergy(T, P, species);
}

auto Thermo::standardPartialMolarEnthalpy(double T, double P, std::string species) const -> ThermoScalar
{
    pimpl->convertUnits(T, P);
    return pimpl->standardPartialMolarEnthalpy(T, P, species);
}

auto Thermo::standardPartialMolarEntropy(double T, double P, std::string species) const -> ThermoScalar
{
    pimpl->convertUnits(T, P);
    return pimpl->standardPartialMolarEntropy(T, P, species);
}

auto Thermo::standardPartialMolarVolume(double T, double P, std::string species) const -> ThermoScalar
{
    pimpl->convertUnits(T, P);
    return pimpl->standardPartialMolarVolume(T, P, species);
}

auto Thermo::standardPartialMolarHeatCapacityConstP(double T, double P, std::string species) const -> ThermoScalar
{
    pimpl->convertUnits(T, P);
    return pimpl->standardPartialMolarHeatCapacityConstP(T, P, species);
}

auto Thermo::standardPartialMolarHeatCapacityConstV(double T, double P, std::string species) const -> ThermoScalar
{
    pimpl->convertUnits(T, P);
    return pimpl->standardPartialMolarHeatCapacityConstV(T, P, species);
}

auto Thermo::lnEquilibriumConstant(double T, double P, std::string reaction) -> ThermoScalar
{
    pimpl->convertUnits(T, P);
    return pimpl->lnEquilibriumConstant(T, P, reaction);
}

auto Thermo::logEquilibriumConstant(double T, double P, std::string reaction) -> ThermoScalar
{
    pimpl->convertUnits(T, P);
    return pimpl->logEquilibriumConstant(T, P, reaction);
}

auto Thermo::hasStandardPartialMolarGibbsEnergy(std::string species) const -> bool
{
    if(pimpl->hasThermoParamsHKF(species))
        return true;
    auto properties = pimpl->getSpeciesInterpolatedThermoProperties(species);
    if(!properties.empty() && !properties().gibbs_energy.empty())
        return true;
    auto reaction = pimpl->getReactionInterpolatedThermoProperties(species);
    if(!reaction.empty() && !reaction().gibbs_energy.empty())
        return true;
    return false;
}

auto Thermo::hasStandardPartialMolarHelmholtzEnergy(std::string species) const -> bool
{
    if(pimpl->hasThermoParamsHKF(species))
        return true;
    auto properties = pimpl->getSpeciesInterpolatedThermoProperties(species);
    if(!properties.empty() && !properties().helmholtz_energy.empty())
        return true;
    auto reaction = pimpl->getReactionInterpolatedThermoProperties(species);
    if(!reaction.empty() && !reaction().helmholtz_energy.empty())
        return true;
    return false;
}

auto Thermo::hasStandardPartialMolarInternalEnergy(std::string species) const -> bool
{
    if(pimpl->hasThermoParamsHKF(species))
        return true;
    auto properties = pimpl->getSpeciesInterpolatedThermoProperties(species);
    if(!properties.empty() && !properties().internal_energy.empty())
        return true;
    auto reaction = pimpl->getReactionInterpolatedThermoProperties(species);
    if(!reaction.empty() && !reaction().internal_energy.empty())
        return true;
    return false;
}

auto Thermo::hasStandardPartialMolarEnthalpy(std::string species) const -> bool
{
    if(pimpl->hasThermoParamsHKF(species))
        return true;
    auto properties = pimpl->getSpeciesInterpolatedThermoProperties(species);
    if(!properties.empty() && !properties().enthalpy.empty())
        return true;
    auto reaction = pimpl->getReactionInterpolatedThermoProperties(species);
    if(!reaction.empty() && !reaction().enthalpy.empty())
        return true;
    return false;
}

auto Thermo::hasStandardPartialMolarEntropy(std::string species) const -> bool
{
    if(pimpl->hasThermoParamsHKF(species))
        return true;
    auto properties = pimpl->getSpeciesInterpolatedThermoProperties(species);
    if(!properties.empty() && !properties().entropy.empty())
        return true;
    auto reaction = pimpl->getReactionInterpolatedThermoProperties(species);
    if(!reaction.empty() && !reaction().entropy.empty())
        return true;
    return false;
}

auto Thermo::hasStandardPartialMolarVolume(std::string species) const -> bool
{
    if(pimpl->hasThermoParamsHKF(species))
        return true;
    auto properties = pimpl->getSpeciesInterpolatedThermoProperties(species);
    if(!properties.empty() && !properties().volume.empty())
        return true;
    auto reaction = pimpl->getReactionInterpolatedThermoProperties(species);
    if(!reaction.empty() && !reaction().volume.empty())
        return true;
    return false;
}

auto Thermo::hasStandardPartialMolarHeatCapacityConstP(std::string species) const -> bool
{
    if(pimpl->hasThermoParamsHKF(species))
        return true;
    auto properties = pimpl->getSpeciesInterpolatedThermoProperties(species);
    if(!properties.empty() && !properties().heat_capacity_cp.empty())
        return true;
    auto reaction = pimpl->getReactionInterpolatedThermoProperties(species);
    if(!reaction.empty() && !reaction().heat_capacity_cp.empty())
        return true;
    return false;
}

auto Thermo::hasStandardPartialMolarHeatCapacityConstV(std::string species) const -> bool
{
    if(pimpl->hasThermoParamsHKF(species))
        return true;
    auto properties = pimpl->getSpeciesInterpolatedThermoProperties(species);
    if(!properties.empty() && !properties().heat_capacity_cv.empty())
        return true;
    auto reaction = pimpl->getReactionInterpolatedThermoProperties(species);
    if(!reaction.empty() && !reaction().heat_capacity_cv.empty())
        return true;
    return false;
}

auto Thermo::speciesThermoStateHKF(double T, double P, std::string species) -> SpeciesThermoState
{
    pimpl->convertUnits(T, P);
    return pimpl->species_thermo_state_hkf_fn(T, P, species);
}

auto Thermo::waterThermoStateHGK(double T, double P) -> WaterThermoState
{
    pimpl->convertUnits(T, P);
    return pimpl->water_thermo_state_hgk_fn(T, P);
}

auto Thermo::waterThermoStateWagnerPruss(double T, double P) -> WaterThermoState
{
    pimpl->convertUnits(T, P);
    return pimpl->water_thermo_state_wagner_pruss_fn(T, P);
}

} // namespace Reaktoro
