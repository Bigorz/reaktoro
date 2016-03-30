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
#include <Reaktoro/Common/Matrix.hpp>
#include <Reaktoro/Core/Element.hpp>
#include <Reaktoro/Core/Species.hpp>
#include <Reaktoro/Core/Phase.hpp>

namespace Reaktoro {

// Forward declarations
class ChemicalProperties;
class ThermoProperties;

/// The result of the thermodynamic model function that calculates the standard thermodynamic properties of all phases in a system.
using ThermoModelResult = std::vector<PhaseThermoModelResult>;

/// The result of the chemical model function that calculates the chemical properties of all phases in a system.
using ChemicalModelResult = std::vector<PhaseChemicalModelResult>;

/// The signature of the chemical model function that calculates the thermodynamic properties of a system.
using ThermoModel = std::function<ThermoModelResult(double, double)>;

/// The signature of the chemical model function that calculates the chemical properties of a system.
using ChemicalModel = std::function<ChemicalModelResult(double, double, const Vector&)>;

/// A class to represent a system and its attributes and properties.
/// @see Species, Phase
/// @ingroup Core
class ChemicalSystem
{
public:
    /// Construct a default ChemicalSystem instance
    ChemicalSystem();

    /// Construct a ChemicalSystem instance with given phases.
    explicit ChemicalSystem(const std::vector<Phase>& phases);

    /// Destroy this ChemicalSystem instance
    virtual ~ChemicalSystem();

    /// Set the function that calculates the thermodynamic properties of the species in the system
    auto setThermoModel(const ThermoModel& model) -> void;

    /// Set the function that calculates the chemical properties of the species and phases in the system
    auto setChemicalModel(const ChemicalModel& model) -> void;

    /// Return the thermodynamic model function of the chemical system.
    /// @see ThermoModel
    auto thermoModel() const -> const ThermoModel&;

    /// Return the chemical model function of the chemical system.
    /// @see ChemicalModel
    auto chemicalModel() const -> const ChemicalModel&;

    /// Return the number of elements in the system
    auto numElements() const -> unsigned;

    /// Return the number of species in the system
    auto numSpecies() const -> unsigned;

    /// Return the number of species in a phase of the system
    /// @param iphase The index of the phase
    auto numSpeciesInPhase(Index iphase) const -> unsigned;

    /// Return the number of phases in the system
    auto numPhases() const -> unsigned;

    /// Return the list of elements in the system
    auto elements() const -> const std::vector<Element>&;

    /// Return the list of species in the system
    auto species() const -> const std::vector<Species>&;

    /// Return the list of phases in the system
    auto phases() const -> const std::vector<Phase>&;

    /// Return the formula matrix of the system
    /// The formula matrix is defined as the matrix whose entry `(j, i)`
    /// is given by the number of atoms of its `j`-th element in its `i`-th species.
    auto formulaMatrix() const -> const Matrix&;

    /// Return an element of the system
    /// @param index The index of the element
    auto element(Index index) const -> const Element&;

    /// Return an element of the system
    /// @param name The name of the element
    auto element(std::string name) const -> const Element&;

    /// Return a species of the system
    /// @param index The index of the species
    auto species(Index index) const -> const Species&;

    /// Return a species of the system
    /// @param name The name of the species
    auto species(std::string name) const -> const Species&;

    /// Return a phase of the system
    /// @param index The index of the phase
    auto phase(Index index) const -> const Phase&;

    /// Return a phase of the system
    /// @param name The name of the phase
    auto phase(std::string name) const -> const Phase&;

    /// Return the index of an element in the system
    /// @param name The name of the element
    auto indexElement(std::string name) const -> Index;

    /// Return the index of an element in the system. system
    /// It throws an exception if the element does not exist
    /// @param name The name of the element
    auto indexElementWithError(std::string name) const -> Index;

    /// Return the index of a species in the system
    /// @param name The name of the species
    auto indexSpecies(std::string name) const -> Index;

    /// Return the index of a species in the system.
    /// @param name The name of the species
    /// @return The index of the species if found, or a runtime exception otherwise.
    auto indexSpeciesWithError(std::string name) const -> Index;

    /// Return the index of the first species in the system with any of the given names.
    /// @param names The tentative names of the species in the system.
    /// @return The index of the species if found, or the number of species otherwise.
    auto indexSpeciesAny(const std::vector<std::string>& names) const -> Index;

    /// Return the index of the first species in the system with any of the given names.
    /// @param names The tentative names of the species in the system.
    /// @return The index of the species if found, or a runtime exception otherwise.
    auto indexSpeciesAnyWithError(const std::vector<std::string>& names) const -> Index;

    /// Return the index of a phase in the system
    /// @param name The name of the phase
    auto indexPhase(std::string name) const -> Index;

    /// Return the index of a phase in the system. system
    /// It throws an exception if the phase does not exist
    /// @param name The name of the phase
    auto indexPhaseWithError(std::string name) const -> Index;

    /// Return the index of the phase that contains a given species
    /// @param index The index of the species
    auto indexPhaseWithSpecies(Index index) const -> Index;

    /// Return the index of the first species in a phase
    /// @param The index of the phase
    auto indexFirstSpeciesInPhase(Index iphase) const -> unsigned;

    /// Return the indices of a set of elements in the system
    /// @param name The names of the elements
    auto indicesElements(const std::vector<std::string>& names) const -> Indices;

    /// Return the indices of the elements that compose a species
    /// @param index The index of the species
    auto indicesElementsInSpecies(Index index) const -> Indices;

    /// Return the indices of the elements that compose a set of species
    /// @param indices The indices of the species
    auto indicesElementsInSpecies(const Indices& indices) const -> Indices;

    /// Return the indices of a set of species in the system
    /// @param names The names of the species
    auto indicesSpecies(const std::vector<std::string>& names) const -> Indices;

    /// Return the indices of a set of phases in the system
    /// @param names The names of the phases
    auto indicesPhases(const std::vector<std::string>& names) const -> Indices;

    /// Return the index of the phase that contains a given species
    /// @param indices The indices of the species
    auto indicesPhasesWithSpecies(const Indices& indices) const -> Indices;

    /// Return the indices of the fluid phases.
    auto indicesFluidPhases() const -> Indices;

    /// Return the indices of the solid phases.
    auto indicesSolidPhases() const -> Indices;

    /// Calculate the molar amounts of the elements (in units of mol)
    /// @param n The molar amounts of the species (in units of mol)
    auto elementAmounts(const Vector& n) const -> Vector;

    /// Calculate the molar amounts of the elements in a given phase (in units of mol)
    /// @param iphase The index of the phase
    /// @param n The molar amounts of the species (in units of mol)
    auto elementAmountsInPhase(Index iphase, const Vector& n) const -> Vector;

    /// Calculate the molar amounts of the elements in a given set of species (in units of mol)
    /// @param ispecies The indices of the species
    /// @param n The molar amounts of the species (in units of mol)
    auto elementAmountsInSpecies(const Indices& ispecies, const Vector& n) const -> Vector;

    /// Calculate the molar amount of an elements (in units of mol)
    /// @param ielement The index of the element
    /// @param n The molar amounts of the species (in units of mol)
    auto elementAmount(Index ielement, const Vector& n) const -> double;

    /// Calculate the molar amounts of the elements in a given phase (in units of mol)
    /// @param ielement The index of the element
    /// @param iphase The index of the phase
    /// @param n The molar amounts of the species (in units of mol)
    auto elementAmountInPhase(Index ielement, Index iphase, const Vector& n) const -> double;

    /// Calculate the molar amounts of the elements in a given set of species (in units of mol)
    /// @param ielement The index of the element
    /// @param ispecies The indices of the species in the set
    /// @param n The molar amounts of the species (in units of mol)
    auto elementAmountInSpecies(Index ielement, const Indices& ispecies, const Vector& n) const -> double;

    /// Calculate the standard thermodynamic properties of the species.
    /// @param T The temperature of the system (in units of K)
    /// @param P The pressure of the system (in units of Pa)
    auto properties(double T, double P) const -> ThermoProperties;

    /// Calculate the thermodynamic and chemical properties of the chemical system.
    /// @param T The temperature of the system (in units of K)
    /// @param P The pressure of the system (in units of Pa)
    /// @param n The molar amounts of the species (in units of mol)
    auto properties(double T, double P, const Vector& n) const -> ChemicalProperties;

private:
    struct Impl;

    std::shared_ptr<Impl> pimpl;
};

/// Output a ChemicalSystem instance
auto operator<<(std::ostream& out, const ChemicalSystem& system) -> std::ostream&;

} // namespace Reaktoro
