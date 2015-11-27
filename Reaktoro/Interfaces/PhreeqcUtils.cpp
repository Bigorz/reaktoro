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

#include "PhreeqcUtils.hpp"

// Reaktoro includes
#include <Reaktoro/Common/Exception.hpp>
#include <Reaktoro/Common/ReactionEquation.hpp>
#include <Reaktoro/Common/ThermoScalar.hpp>

namespace Reaktoro {
namespace PhreeqcUtils {

auto load(PHREEQC& phreeqc, std::string database) -> void
{
    // Initialize the phreeqc instance
    int errors = phreeqc.do_initialize();
    Assert(errors == 0, "Could not initialize PHREEQC.",
        "Call to method `Phreeqc::do_initialize` failed.");

    // Initialize the phreeqc database
    std::istream* db_cookie = new std::ifstream(database, std::ios_base::in);
    phreeqc.Get_phrq_io()->push_istream(db_cookie);
    errors = phreeqc.read_database();
    phreeqc.Get_phrq_io()->clear_istream();
    phreeqc.Get_phrq_io()->Set_error_ostream(&std::cerr);
    phreeqc.Get_phrq_io()->Set_output_ostream(&std::cout);
    Assert(errors == 0, "Could not load the PHREEQC database file `" + database + "`.",
        "Ensure `" + database + "` points to the right path to the database file.");
}

auto execute(PHREEQC& phreeqc, std::string input) -> void
{
    // Check if the input is (1) a filename (without the newline char \n) or
    // (2) a input script coded in a string.
    std::istream* in_cookie;
    if(input.find('\n') == std::string::npos)
        in_cookie = new std::ifstream(input, std::ios_base::in);
    else in_cookie = new std::istringstream(input);

    phreeqc.Get_phrq_io()->push_istream(in_cookie);
    int errors = phreeqc.run_simulations();
    phreeqc.Get_phrq_io()->clear_istream();
    Assert(errors == 0, "Failed to execute the PHREEQC input script `" + input + "`.",
        "There was a Phreeqc error when executing this input script file.");
}

auto findElement(const PHREEQC& phreeqc, std::string name) -> PhreeqcElement*
{
    for(int i = 0; i < phreeqc.count_elements; ++i)
        if(phreeqc.elements[i]->name == name)
            return phreeqc.elements[i];
    return nullptr;
}

auto findSpecies(const PHREEQC& phreeqc, std::string name) -> PhreeqcSpecies*
{
    for(int i = 0; i < phreeqc.count_s; ++i)
        if(phreeqc.s[i]->name == name)
            return phreeqc.s[i];
    return nullptr;
}

auto findPhase(const PHREEQC& phreeqc, std::string name) -> PhreeqcPhase*
{
    for(int i = 0; i < phreeqc.count_phases; ++i)
        if(phreeqc.phases[i]->name == name)
            return phreeqc.phases[i];
    return nullptr;
}

auto elements(const PhreeqcSpecies* species) -> std::map<PhreeqcElement*, double>
{
    std::map<PhreeqcElement*, double> elements;
    for(auto iter = species->next_elt; iter->elt != nullptr; iter++)
        elements.emplace(iter->elt, iter->coef);
    return elements;
}

auto elements(const PhreeqcPhase* phase) -> std::map<PhreeqcElement*, double>
{
    std::map<PhreeqcElement*, double> elements;
    for(auto iter = phase->next_elt; iter->elt != nullptr; iter++)
        elements.emplace(iter->elt, iter->coef);
    return elements;
}

auto stoichiometry(std::string element, const PhreeqcSpecies* species) -> double
{
    // Return species charge if element is electrical charge Z
    if(element == "Z") return species->z;

    // Iterate over all elements in the species
    for(auto iter = species->next_elt; iter->elt != nullptr; iter++)
        if(iter->elt->name == element)
            return iter->coef;
    return 0.0;
}

auto stoichiometry(std::string element, const PhreeqcPhase* phase) -> double
{
    // Iterate over all elements in the phase (gasesous or mineral species)
    for(auto iter = phase->next_elt; iter->elt != nullptr; iter++)
        if(iter->elt->name == element)
            return iter->coef;
    return 0.0;
}

auto name(const PhreeqcElement* element) -> std::string
{
    return element->name;
}

auto name(const PhreeqcSpecies* species) -> std::string
{
    const double charge = species->z;
    const std::string phreeqc_name = species->name;
    if(phreeqc_name == "H2O") return "H2O(l)";
    if(charge == 0) return phreeqc_name + "(aq)";
    if(charge < 0) return phreeqc_name.substr(0, phreeqc_name.rfind('-')) + std::string(std::abs(charge), '-');
    else return phreeqc_name.substr(0, phreeqc_name.rfind('+')) + std::string(std::abs(charge), '+');
}

auto name(const PhreeqcPhase* phase) -> std::string
{
    return phase->name;
}

auto isGaseousSpecies(const PhreeqcPhase* phase) -> bool
{
	std::string name(phase->name);
	return name.find("(g)") < name.size();
}

auto isMineralSpecies(const PhreeqcPhase* phase) -> bool
{
	return !isGaseousSpecies(phase);
}

auto reactionEquation(const PhreeqcSpecies* species) -> ReactionEquation
{
    // Check if there is any reaction defined by this species.
    if(species->rxn == nullptr) return {};

    // The reaction equation as pairs of names and stoichiometries
    std::map<std::string, double> pairs;

    // Iterate over all species in the reaction, get their names and stoichiometries.
    for(auto iter = species->rxn->token; iter->s != nullptr; iter++)
        pairs.emplace(iter->s->name, -iter->coef);

    // Check if the reaction is a trivial reaction (e.g., H+ = H+)
    // Phreeqc generates this reactions for master species. Here, we
    // return a empty reaction equation in this case.
    if(pairs.size() <= 1) return {};

    return ReactionEquation(pairs);
}

auto reactionEquation(const PhreeqcPhase* phase) -> ReactionEquation
{
    // Check if there is any reaction defined by this species.
    if(phase->rxn == nullptr) return {};

    // The reaction equation as pairs of names and stoichiometries
    std::map<std::string, double> pairs;

    // Iterate over all species in the reaction, get their names and stoichiometries.
    // Note that the reaction equation for a Phreeqc phase is read differently from a
    // Phreeqc species instance.
    pairs.emplace(phase->rxn->token->name, phase->rxn->token->coef);
    for(auto iter = phase->rxn->token + 1; iter->s != nullptr; iter++)
        pairs.emplace(iter->s->name, iter->coef);

    // Check if the reaction is a trivial reaction (e.g., X- = X-)
    // Phreeqc generates this reactions for master species. Here, we
    // return a empty reaction equation in this case.
    if(pairs.size() <= 1) return {};

    return ReactionEquation(pairs);
}

auto isAqueousSpecies(const PhreeqcSpecies* species) -> bool
{
    return species->type != EX && species->type != SURF;
}

auto isExchangeSpecies(const PhreeqcSpecies* species) -> bool
{
    return species->type == EX;
}

auto index(std::string name, const std::vector<PhreeqcSpecies*>& species) -> unsigned
{
    auto compare = [=](PhreeqcSpecies* entry) { return entry->name == name; };
    return std::find_if(species.begin(), species.end(), compare) - species.begin();
}

auto index(std::string name, const std::vector<PhreeqcPhase*>& phases) -> unsigned
{
    auto compare = [=](PhreeqcPhase* entry) { return entry->name == name; };
    return std::find_if(phases.begin(), phases.end(), compare) - phases.begin();
}

auto activeAqueousSpecies(const PHREEQC& phreeqc) -> std::vector<PhreeqcSpecies*>
{
    // Inspired by method `int Phreeqc::print_species(void)`
    std::set<PhreeqcSpecies*> species;

    // Loop over all species in `species_list` data-member of PHREEQC
    for(int i = 0; i < phreeqc.count_species_list; ++i)
        if(phreeqc.species_list[i].s->type != EX &&
           phreeqc.species_list[i].s->type != SURF)
            species.insert(phreeqc.species_list[i].s);

    // Loop over all master species in `master` data-member of PHREEQC
    for(int i = 0; i < phreeqc.count_master; ++i)
        if(phreeqc.master[i]->in &&
           phreeqc.master[i]->type != EX &&
           phreeqc.master[i]->type != SURF)
            species.insert(phreeqc.master[i]->s);

    return {species.begin(), species.end()};
}

auto activeExchangeSpecies(const PHREEQC& phreeqc) -> std::vector<PhreeqcSpecies*>
{
    // Inspired by method `int Phreeqc::print_exchange(void)`
    std::set<PhreeqcSpecies*> species;
    for(int i = 0; i < phreeqc.count_species_list; ++i)
        if(phreeqc.species_list[i].s->type == EX)
            species.insert(phreeqc.species_list[i].s);
    return {species.begin(), species.end()};
}

auto activeProductSpecies(const PHREEQC& phreeqc) -> std::vector<PhreeqcSpecies*>
{
    std::vector<PhreeqcSpecies*> species;
    for(PhreeqcSpecies* s : activeAqueousSpecies(phreeqc))
        if(!reactionEquation(s).empty()) //  a product species has a non-empty reaction equation
            species.push_back(s);
    return species;
}

auto activeGaseousSpecies(const PHREEQC& phreeqc) -> std::vector<PhreeqcPhase*>
{
    // Collect the gaseous species of the cxxGasPhase instances
    std::set<PhreeqcPhase*> gases;
    for(const auto& pair : phreeqc.Rxn_gas_phase_map)
    {
        const cxxGasPhase& gas_phase = pair.second;
        for(const cxxGasComp& component : gas_phase.Get_gas_comps())
            gases.insert(findPhase(phreeqc, component.Get_phase_name()));
    }
    return {gases.begin(), gases.end()};
}

auto activePhasesInEquilibriumPhases(const PHREEQC& phreeqc) -> std::vector<PhreeqcPhase*>
{
    // Inspired by method `int Phreeqc::print_pp_assemblage(void)`
    std::vector<PhreeqcPhase*> phases;
    unknown** x = phreeqc.x;
    for(int i = 0; i < phreeqc.count_unknowns; ++i)
        if(x[i]->type == PP && x[i]->phase->rxn_x != nullptr && x[i]->phase->in == true)
            phases.push_back(x[i]->phase);
    return phases;
}

auto activePhasesInSaturationList(const PHREEQC& phreeqc) -> std::vector<PhreeqcPhase*>
{
    std::vector<PhreeqcPhase*> phases;
    for(int i = 0; i < phreeqc.count_phases; ++i)
        if(phreeqc.phases[i]->in)
            phases.push_back(phreeqc.phases[i]);
    return phases;
}

auto speciesAmounts(const std::vector<PhreeqcSpecies*>& species) -> Vector
{
    const unsigned size = species.size();
    Vector n(size);
    for(unsigned i = 0; i < size; ++i)
        n[i] = species[i]->moles;
    return n;
}

auto speciesAmounts(const std::vector<PhreeqcPhase*>& phases) -> Vector
{
    const unsigned size = phases.size();
    Vector n(size);
    for(unsigned i = 0; i < size; ++i)
        n[i] = phases[i]->moles_x;
    return n;
}

template<typename SpeciesType>
auto delta_h_in_original_units(const SpeciesType* species) -> double
{
    const double delta_h_in_kjoule = species->logk[delta_h];
    const double kjoule_per_cal = 4.184e-3;
    const double kjoule_per_kcal = kjoule_per_cal * 1e+3;
    const double kjoule_per_joule = 1e-3;
    switch(species->original_units)
    {
        case kcal: return delta_h_in_kjoule/kjoule_per_kcal;
        case cal: return delta_h_in_kjoule/kjoule_per_cal;
        case kjoules: return delta_h_in_kjoule;
        case joules: return delta_h_in_kjoule/kjoule_per_joule;
    }
    RuntimeError("Could not convert PHREEQC reaction data `delta_h` to kilo joule.",
        "The species or phase `" + std::string(species->name) + "` does not have a valid"
            "`original_units` data-member value.");
}

// Returns true if the PHREEQC analytical logk expression can be used, false otherwise.
auto useAnalytic(const double* logk) -> bool
{
    for(int i = T_A1; i <= T_A6; ++i)
        if(logk[i] != 0.0)
            return true;
    return false;
}

template<typename SpeciesType>
auto lnEquilibriumConstantHelper(const SpeciesType* species, Temperature T, Pressure P) -> double
{
    //--------------------------------------------------------------------------------
    // The implementation of this method was inspired by the PHREEQC
    // method `Phreeqc::select_log_k_expression`
    //--------------------------------------------------------------------------------

    // Auxiliary variables
    const double R = 8.31470e-3; // universal gas constant in units of kJ/(K*mol)
    const double ln10 = std::log(10.0);
    const auto logk = species->logk;

    // Check if the PHREEQC analytical expression for logk should be used
    if(useAnalytic(logk))
        return (logk[T_A1] + logk[T_A2]*T + logk[T_A3]/T +
            logk[T_A4]*std::log10(T) + logk[T_A5]/(T*T) + logk[T_A6]*(T*T)) * ln10;

    // Use the Van't Hoff equation instead
    return logk[logK_T0]*ln10 - logk[delta_h] * (298.15 - T)/(R*T*298.15);
}

auto lnEquilibriumConstant(const PhreeqcSpecies* species, Temperature T, Pressure P) -> double
{
    return lnEquilibriumConstantHelper(species, T, P);
}

auto lnEquilibriumConstant(const PhreeqcPhase* phase, Temperature T, Pressure P) -> double
{
    return lnEquilibriumConstantHelper(phase, T, P);
}

} // namespace PhreeqcUtils
} // namespace Reaktoro
