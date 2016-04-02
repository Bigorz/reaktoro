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

#pragma once

// Reaktoro includes
#include <Reaktoro/Common/ChemicalVector.hpp>
#include <Reaktoro/Common/ThermoVector.hpp>

namespace Reaktoro {

// Forward declarations
class ChemicalSystem;

/// Defines a class for querying thermodynamic and chemical properties of a set of phases.
class ChemicalProperties
{
public:
    /// Construct a default ChemicalProperties instance.
    ChemicalProperties();

    /// Construct a ChemicalProperties instance with given ChemicalSystem.
    ChemicalProperties(const ChemicalSystem& system);

    /// Construct a copy of a ChemicalProperties instance.
    ChemicalProperties(const ChemicalProperties& other);

    /// Destroy this instance.
    virtual ~ChemicalProperties();

    /// Construct a copy of a ChemicalProperties instance.
    auto operator=(ChemicalProperties other) -> ChemicalProperties&;

    /// Update the thermodynamic properties of the chemical system.
    /// @param T The new temperature (in units of K)
    /// @param P The new pressure (in units of Pa)
    auto update(double T, double P) -> void;

    /// Update the chemical properties of the chemical system.
    /// @param T The new temperature (in units of K)
    /// @param P The new pressure (in units of Pa)
    /// @param n The new species amounts (in units of mol)
    auto update(double T, double P, const Vector& n) -> void;

    /// Return the temperature of the system (in units of K).
    auto temperature() const -> double;

    /// Return the pressure of the system (in units of Pa).
    auto pressure() const -> double;

    /// Return the molar amounts of the species (in units of mol).
    auto composition() const -> const Vector&;

    /// Return the molar fractions of the species.
    auto molarFractions() const -> ChemicalVector;

    /// Return the ln activity coefficients of the species.
    auto lnActivityCoefficients() const -> ChemicalVector;

    /// Return the ln activity constants of the species.
    auto lnActivityConstants() const -> ThermoVector;

    /// Return the ln activities of the species.
    auto lnActivities() const -> ChemicalVector;

    /// Return the chemical potentials of the species (in units of J/mol).
    auto chemicalPotentials() const -> ChemicalVector;

    /// Return the standard partial molar Gibbs energies of the species (in units of J/mol).
    auto standardPartialMolarGibbsEnergies() const -> ThermoVector;

    /// Return the standard partial molar enthalpies of the species (in units of J/mol).
    auto standardPartialMolarEnthalpies() const -> ThermoVector;

    /// Return the standard partial molar volumes of the species (in units of m3/mol).
    auto standardPartialMolarVolumes() const -> ThermoVector;

    /// Return the standard partial molar entropies of the species (in units of J/(mol*K)).
    auto standardPartialMolarEntropies() const -> ThermoVector;

    /// Return the standard partial molar internal energies of the species (in units of J/mol).
    auto standardPartialMolarInternalEnergies() const -> ThermoVector;

    /// Return the standard partial molar Helmholtz energies of the species (in units of J/mol).
    auto standardPartialMolarHelmholtzEnergies() const -> ThermoVector;

    /// Return the standard partial molar isobaric heat capacities of the species (in units of J/(mol*K)).
    auto standardPartialMolarHeatCapacitiesConstP() const -> ThermoVector;

    /// Return the standard partial molar isochoric heat capacities of the species (in units of J/(mol*K)).
    auto standardPartialMolarHeatCapacitiesConstV() const -> ThermoVector;

    /// Return the molar Gibbs energies of the phases (in units of J/mol).
    auto phaseMolarGibbsEnergies() const -> ChemicalVector;

    /// Return the molar enthalpies of the phases (in units of J/mol).
    auto phaseMolarEnthalpies() const -> ChemicalVector;

    /// Return the molar volumes of the phases (in units of m3/mol).
    auto phaseMolarVolumes() const -> ChemicalVector;

    /// Return the molar entropies of the phases (in units of J/(mol*K)).
    auto phaseMolarEntropies() const -> ChemicalVector;

    /// Return the molar internal energies of the phases (in units of J/mol).
    auto phaseMolarInternalEnergies() const -> ChemicalVector;

    /// Return the molar Helmholtz energies of the phases (in units of J/mol).
    auto phaseMolarHelmholtzEnergies() const -> ChemicalVector;

    /// Return the molar isobaric heat capacities of the phases (in units of J/(mol*K)).
    auto phaseMolarHeatCapacitiesConstP() const -> ChemicalVector;

    /// Return the molar isochoric heat capacities of the phases (in units of J/(mol*K)).
    auto phaseMolarHeatCapacitiesConstV() const -> ChemicalVector;

    /// Return the specific Gibbs energies of the phases (in units of J/kg).
    auto phaseSpecificGibbsEnergies() const -> ChemicalVector;

    /// Return the specific enthalpies of the phases (in units of J/kg).
    auto phaseSpecificEnthalpies() const -> ChemicalVector;

    /// Return the specific volumes of the phases (in units of m3/kg).
    auto phaseSpecificVolumes() const -> ChemicalVector;

    /// Return the specific entropies of the phases (in units of J/(kg*K)).
    auto phaseSpecificEntropies() const -> ChemicalVector;

    /// Return the specific internal energies of the phases (in units of J/kg).
    auto phaseSpecificInternalEnergies() const -> ChemicalVector;

    /// Return the specific Helmholtz energies of the phases (in units of J/kg).
    auto phaseSpecificHelmholtzEnergies() const -> ChemicalVector;

    /// Return the specific isobaric heat capacities of the phases (in units of J/(kg*K)).
    auto phaseSpecificHeatCapacitiesConstP() const -> ChemicalVector;

    /// Return the specific isochoric heat capacities of the phases (in units of J/(kg*K)).
    auto phaseSpecificHeatCapacitiesConstV() const -> ChemicalVector;

    /// Return the densities of the phases (in units of kg/m3).
    auto phaseDensities() const -> ChemicalVector;

    /// Return the masses of the phases (in units of kg).
    auto phaseMasses() const -> ChemicalVector;

    /// Return the molar amounts of the phases (in units of mol).
    auto phaseAmounts() const -> ChemicalVector;

    /// Return the volumes of the phases (in units of m3).
    auto phaseVolumes() const -> ChemicalVector;

    /// Return the volume of the system (in units of m3).
    auto volume() const -> ChemicalScalar;

    /// Return the total fluid volume of the system (in units of m3).
    /// The fluid volume is defined as the sum of volumes of all fluid phases.
    auto fluidVolume() const -> ChemicalScalar;

    /// Return the total solid volume of the system (in units of m3).
    /// The solid volume is defined as the sum of volumes of all solid phases.
    auto solidVolume() const -> ChemicalScalar;

    /// Return the porosity of the system.
    /// The porosity of the system is defined as the fluid volume divided by
    /// the total volume of system.
    auto porosity() const -> ChemicalScalar;

    /// Return the pH of the system.
    /// If the chemical system has no aqueous phase, then zero is returned.
    /// The aqueous phase must have a hydron species named either H+, H+(aq), or H[+].
    auto pH() const -> ChemicalScalar;

    /// Return the pe of the system.
    /// This methods calculates pe using the dual chemical potential of charge element.
    /// This is an alternative approach to using a half reaction (Kulik, 2006).
    /// If the chemical system has no aqueous phase, then zero is returned.
    auto pe() const -> ChemicalScalar;

    /// Return the pe of the system calculated using a given half reaction.
    /// Use this method to specify a half reaction for the calculation of pe.
    /// For example:
    /// ~~~
    /// ChemicalProperties properties(system);
    /// properties.update(T, P, n);
    /// properties.pe("Fe++ = Fe+++ + e-");
    /// ~~~
    /// Note that the electro species `e-` must be present in the half reaction.
    /// If the chemical system has no aqueous phase, then zero is returned.
    auto pe(std::string reaction) const -> ChemicalScalar;

    /// Return the redox potential of the system (in units of V).
    /// This methods calculates Eh using the dual chemical potential of charge element.
    /// This is an alternative approach to using a half reaction (Kulik, 2006).
    /// If the chemical system has no aqueous phase, then zero is returned.
    auto Eh() const -> ChemicalScalar;

    /// Return the redox potential of the system calculated using a given half reaction (in units of V).
    /// Use this method to specify a half reaction for the calculation of Eh.
    /// For example:
    /// ~~~
    /// ChemicalProperties properties(system);
    /// properties.update(T, P, n);
    /// properties.Eh("Fe++ = Fe+++ + e-");
    /// ~~~
    /// Note that the electro species `e-` must be present in the half reaction.
    /// If the chemical system has no aqueous phase, then zero is returned.
    auto Eh(std::string reaction) const -> ChemicalScalar;

private:
    struct Impl;

    std::unique_ptr<Impl> pimpl;
};

} // namespace Reaktoro
