// Reaktor is a C++ library for computational reaction modelling.
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

#include "AqueousMixture.hpp"

// C++ includes
#include <algorithm>

// Reaktor includes
#include <Reaktor/Common/SetUtils.hpp>

namespace Reaktor {
namespace {

auto speciesIndex(const std::vector<AqueousSpecies>& solution, const std::string& name) -> Index
{
    for(Index i = 0; i < solution.size(); ++i)
        if(solution[i].name == name) return i;
    return solution.size();
}

auto waterIndex(const std::vector<AqueousSpecies>& solution) -> Index
{
    auto comparer = [](const AqueousSpecies& species)
    {
        return species.name == "H2O(l)";
    };

    return std::find_if(solution.begin(), solution.end(), comparer) - solution.begin();
}

auto chargedSpeciesIndices(const std::vector<AqueousSpecies>& solution) -> Indices
{
    Indices indices;
    for(unsigned i = 0; i < solution.size(); ++i)
        if(solution[i].charge != 0)
            indices.push_back(i);
    return indices;
}

auto neutralSpeciesIndices(const std::vector<AqueousSpecies>& solution) -> Indices
{
    Indices indices;
    for(unsigned i = 0; i < solution.size(); ++i)
        if(solution[i].charge == 0)
            indices.push_back(i);
    return indices;
}

auto cationIndices(const std::vector<AqueousSpecies>& solution) -> Indices
{
    Indices indices_cations;
    for(unsigned i = 0; i < solution.size(); ++i)
        if(solution[i].charge > 0)
            indices_cations.push_back(i);
    return indices_cations;
}

auto anionIndices(const std::vector<AqueousSpecies>& solution) -> Indices
{
    Indices indices_anions;
    for(unsigned i = 0; i < solution.size(); ++i)
        if(solution[i].charge < 0)
            indices_anions.push_back(i);
    return indices_anions;
}

auto chargedSpeciesLocalIndex(const std::vector<AqueousSpecies>& solution, const std::string& name) -> Index
{
    const Index idx = speciesIndex(solution, name);
    return index(idx, chargedSpeciesIndices(solution));
}

auto neutralSpeciesLocalIndex(const std::vector<AqueousSpecies>& solution, const std::string& name) -> Index
{
    const Index idx = speciesIndex(solution, name);
    return index(idx, neutralSpeciesIndices(solution));
}

auto cationLocalIndex(const std::vector<AqueousSpecies>& solution, const std::string& name) -> Index
{
    const Index idx = speciesIndex(solution, name);
    return index(idx, cationIndices(solution));
}

auto anionLocalIndex(const std::vector<AqueousSpecies>& solution, const std::string& name) -> Index
{
    const Index idx = speciesIndex(solution, name);
    return index(idx, anionIndices(solution));
}

auto speciesNames(const std::vector<AqueousSpecies>& solution) -> std::vector<std::string>
{
    const unsigned nspecies = solution.size();
    std::vector<std::string> names(nspecies);
    for(unsigned i = 0; i < nspecies; ++i)
        names[i] = solution[i].name;
    return names;
}

auto chargedSpeciesNames(const std::vector<AqueousSpecies>& solution) -> std::vector<std::string>
{
    return extract(speciesNames(solution), chargedSpeciesIndices(solution));
}

auto neutralSpeciesNames(const std::vector<AqueousSpecies>& solution) -> std::vector<std::string>
{
    return extract(speciesNames(solution), neutralSpeciesIndices(solution));
}

auto cationNames(const std::vector<AqueousSpecies>& solution) -> std::vector<std::string>
{
    return extract(speciesNames(solution), cationIndices(solution));
}

auto anionNames(const std::vector<AqueousSpecies>& solution) -> std::vector<std::string>
{
    return extract(speciesNames(solution), anionIndices(solution));
}

auto chargedSpeciesCharges(const std::vector<AqueousSpecies>& solution) -> Vector
{
    return rows(speciesCharges(solution), chargedSpeciesIndices(solution));
}

auto cationCharges(const std::vector<AqueousSpecies>& solution) -> Vector
{
    return rows(speciesCharges(solution), cationIndices(solution));
}

auto anionCharges(const std::vector<AqueousSpecies>& solution) -> Vector
{
    return rows(speciesCharges(solution), anionIndices(solution));
}

auto dissociationMatrix(const std::vector<AqueousSpecies>& solution) -> Matrix
{
    // The indices of the neutral and charged species
    const Indices indices_neutral = neutralSpeciesIndices(solution);
    const Indices indices_charged = chargedSpeciesIndices(solution);

    // Gets the stoichiometry of the i-th charged species in the j-th neutral species
    auto stoichiometry = [&](unsigned i, unsigned j) -> double
    {
        const Index ineutral = indices_neutral[i];
        const Index icharged = indices_charged[j];
        const AqueousSpecies& neutral = solution[ineutral];
        const AqueousSpecies& charged = solution[icharged];
        const auto iter = neutral.dissociation.find(charged.name);
        return iter != neutral.dissociation.end() ? iter->second : 0.0;
    };

    // Assemble the dissociation matrix of the neutral species with respect to the charged species
    const unsigned num_charged_species = indices_charged.size();
    const unsigned num_neutral_species = indices_neutral.size();
    Matrix dissociation_matrix = zeros(num_neutral_species, num_charged_species);
    for(unsigned i = 0; i < num_neutral_species; ++i)
        for(unsigned j = 0; j < num_charged_species; ++j)
            dissociation_matrix(i, j) = stoichiometry(i, j);
    return dissociation_matrix;
}

} // namespace

AqueousMixture::AqueousMixture()
{}

AqueousMixture::AqueousMixture(const std::vector<AqueousSpecies>& species)
: GeneralMixture<AqueousSpecies>(species)
{
    // Initialize the index of the water species
    idx_water = idxSpecies("H2O(l)");

    // Initialize the indices of the charged and neutral aqueous species
    for(unsigned i = 0; i < species.size(); ++i)
        if(species[i].charge() != 0) idx_charged.push_back(i);
        else idx_neutral_species.push_back(i);

    // Initialize the set of aqueous species that are regarded as ions in the aqueous mixture
    ions =
    {
        "Ag+"  , "Fe++"  , "Ca+++" , "Ru+++"  , "BrO3-" , "ReO4-"  ,
        "Au+"  , "Hg++"  , "Ce+++" , "Sm+++"  , "CN-"   , "SCN-"   ,
        "Cs+"  , "Mg++"  , "Cr+++" , "Tb+++"  , "Cl-"   , "CO3--"  ,
        "Cu+"  , "Mn++"  , "Dy+++" , "Tm+++"  , "ClO-"  , "CrO4--" ,
        "H+"   , "Ni++"  , "Er+++" , "V+++"   , "ClO2-" , "HPO4--" ,
        "K+"   , "Pb++"  , "Eu+++" , "Y+++"   , "ClO3-" , "SO3--"  ,
        "Li+"  , "Pd++"  , "Fe+++" , "Yb+++"  , "ClO4-" , "SO4--"  ,
        "NH4+" , "Ru++"  , "Gd+++" , "Ce++++" , "F-"    , "Se--"   ,
        "Na+"  , "Sn++"  , "Ho+++" , "Hf++++" , "HCO3-" , "SeO3--" ,
        "Rb+"  , "Sr++"  , "In+++" , "Np++++" , "HS-"   , "SeO4--" ,
        "VO2+" , "TcO++" , "La+++" , "Pu++++" , "HSO4-" , "TcO4--" ,
        "Ba++" , "UO2++" , "Lu+++" , "Sn++++" , "I-"    , "VO4---" ,
        "Ca++" , "VO++"  , "Nd+++" , "Th++++" , "IO3-"  ,
        "Cd++" , "Zn++"  , "Np+++" , "U++++"  , "N3-"   ,
        "Co++" , "Al+++" , "Pm+++" , "Zr++++" , "NO2-"  ,
        "Cu++" , "Am+++" , "Pr+++" , "Br-"    , "NO3-"  ,
        "Eu++" , "Au+++" , "Pu+++" , "BrO-"   , "OH-"
    };

    // Initialize the indices of the ionic species
    for(const AqueousSpecies& iter : species)
        if(ions.count(iter.name()))
            idx_ions.push_back(idxSpecies(iter.name()));

    // Initialize the indices of the aqueous complexes
    for(const AqueousSpecies& iter : species)
        if(not iter.dissociation().empty())
            idx_complexes.push_back(idxSpecies(iter.name()));

    // Initialize the electrical charges of the aqueous species
    z.resize(species.size());
    for(unsigned i = 0; i < species.size(); ++i)
        z[i] = species[i].charge();

    // Initialize the electrical charges of the ions
    zi = rows(idx_ions, z);

    // Initialize the matrix that represents the dissociation
    // of the aqueous complexes into ions
    const unsigned num_ions = idx_ions.size();
    const unsigned num_complexes = idx_complexes.size();

    nu = zeros(num_complexes, num_ions);

    auto stoichiometry = [&](unsigned i, unsigned j) -> double
    {
        for(auto pair : species[idx_complexes[i]].dissociation())
            if(pair.first == species[idx_ions[j]].name())
                return pair.second;
        return 0.0;
    };

    for(unsigned i = 0; i < num_complexes; ++i)
        for(unsigned j = 0; j < num_ions; ++j)
            nu(i, j) = stoichiometry(i, j);
}

AqueousMixture::~AqueousMixture()
{}

auto AqueousMixture::numChargedSpecies() const -> unsigned
{
    return idx_charged.size();
}

auto AqueousMixture::numIons() const -> unsigned
{
    return idx_ions.size();
}

auto AqueousMixture::numComplexes() const -> unsigned
{
    return idx_complexes.size();
}

auto AqueousMixture::charges() const -> Vector
{
    return z;
}

auto AqueousMixture::idxNeutralSpecies() const -> const Indices&
{
    return idx_neutral_species;
}

auto AqueousMixture::idxChargedSpecies() const -> const Indices&
{
    return idx_charged;
}

auto AqueousMixture::idxIons() const -> const Indices&
{
    return idx_ions;
}

auto AqueousMixture::idxCations() const -> Indices
{
    Indices idx_cations;

    for(const auto& idx : idx_charged)
        if(z[idx] > 0) idx_cations.push_back(idx);

    return idx_cations;
}

auto AqueousMixture::idxAnions() const -> Indices
{
    Indices idx_anions;

    for(const auto& idx : idx_charged)
        if(z[idx] < 0) idx_anions.push_back(idx);

    return idx_anions;
}

auto AqueousMixture::idxComplexes() const -> const Indices&
{
    return idx_complexes;
}

auto AqueousMixture::idxWater() const -> const Index&
{
    return idx_water;
}

auto AqueousMixture::dissociationMatrix() const -> const Matrix&
{
    return nu;
}

auto AqueousMixture::idxIon(const std::string& ion) const -> Index
{
    const Index idx = idxSpecies(ion);

    return std::find(idx_ions.begin(), idx_ions.end(), idx) - idx_ions.begin();
}

auto AqueousMixture::neutralSpecies() const -> std::vector<std::string>
{
    return extract(speciesNames(), idxNeutralSpecies());
}

auto AqueousMixture::chargedSpecies() const -> std::vector<std::string>
{
    return extract(speciesNames(), idxChargedSpecies());
}

auto AqueousMixture::cations() const -> std::vector<std::string>
{
    return extract(speciesNames(), idxCations());
}

auto AqueousMixture::anions() const -> std::vector<std::string>
{
    return extract(speciesNames(), idxAnions());
}

auto AqueousMixture::complexes() const -> std::vector<std::string>
{
    return extract(speciesNames(), idxComplexes());
}

auto AqueousMixture::molalities(const Vector& n) const -> ChemicalVector
{
    const unsigned size = numSpecies();

    const double nw = n[idx_water];

    const Vector m = 55.508 * n/nw;

    Matrix dmdn = zeros(size, size);

    for(unsigned i = 0; i < size; ++i)
        dmdn(i, i) = m[i]/n[i];

    for(unsigned i = 0; i < size; ++i)
        dmdn(i, idx_water) -= m[i]/nw;

    return partialVector(m, dmdn);
}

auto AqueousMixture::stoichiometricMolalities(const ChemicalVector& m) const -> ChemicalVector
{
    // The molalities of the ionic species
    ChemicalVector m_ions;
    func(m_ions) = rows(idx_ions, func(m));
    grad(m_ions) = rows(idx_ions, grad(m));

    // The molalities of the complex species
    ChemicalVector m_complexes;
    func(m_complexes) = rows(idx_complexes, func(m));
    grad(m_complexes) = rows(idx_complexes, grad(m));

    // The stoichiometric molalities of the ionic species
    ChemicalVector ms;
    func(ms) = func(m_ions) + nu.transpose() * func(m_complexes);
    grad(ms) = grad(m_ions) + nu.transpose() * grad(m_complexes);

    return ms;
}

auto AqueousMixture::effectiveIonicStrength(const ChemicalVector& m) const -> ChemicalScalar
{
    const unsigned num_species = numSpecies();

    ChemicalScalar Ie = partialScalar(0.0, Vector(num_species));

    func(Ie) = 0.5 * (z.array() * z.array() * func(m).array()).sum();

    for(unsigned j = 0; j < num_species; ++j)
        grad(Ie)[j] = 0.5 * (z.array() * z.array() * grad(m).col(j).array()).sum();

    return Ie;
}

auto AqueousMixture::stoichiometricIonicStrength(const ChemicalVector& ms) const -> ChemicalScalar
{
    const unsigned num_species = numSpecies();

    ChemicalScalar Is = partialScalar(0.0, Vector(num_species));

    func(Is) = 0.5 * (zi.array() * zi.array() * func(ms).array()).sum();

    for(unsigned j = 0; j < num_species; ++j)
        grad(Is)[j] = 0.5 * (zi.array() * zi.array() * grad(ms).col(j).array()).sum();

    return Is;
}

} // namespace Reaktor
