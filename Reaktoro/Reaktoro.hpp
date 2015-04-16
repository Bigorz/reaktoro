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

#pragma once

#include <Reaktoro/Common/ChemicalScalar.hpp>
#include <Reaktoro/Common/ChemicalVector.hpp>
#include <Reaktoro/Common/Constants.hpp>
#include <Reaktoro/Common/ConvertUtils.hpp>
#include <Reaktoro/Common/ElementUtils.hpp>
#include <Reaktoro/Common/Exception.hpp>
#include <Reaktoro/Common/Functions.hpp>
#include <Reaktoro/Common/Index.hpp>
#include <Reaktoro/Common/InterpolationUtils.hpp>
#include <Reaktoro/Common/Matrix.hpp>
#include <Reaktoro/Common/OptimizationUtils.hpp>
#include <Reaktoro/Common/Optional.hpp>
#include <Reaktoro/Common/Outputter.hpp>
#include <Reaktoro/Common/ParseUtils.hpp>
#include <Reaktoro/Common/ReactionEquation.hpp>
#include <Reaktoro/Common/SetUtils.hpp>
#include <Reaktoro/Common/StringUtils.hpp>
#include <Reaktoro/Common/ThermoScalar.hpp>
#include <Reaktoro/Common/ThermoVector.hpp>
#include <Reaktoro/Common/TimeUtils.hpp>
#include <Reaktoro/Common/Units.hpp>
#include <Reaktoro/Core/ChemicalModels.hpp>
#include <Reaktoro/Core/ChemicalState.hpp>
#include <Reaktoro/Core/ChemicalSystem.hpp>
#include <Reaktoro/Core/Element.hpp>
#include <Reaktoro/Core/Partition.hpp>
#include <Reaktoro/Core/Phase.hpp>
#include <Reaktoro/Core/Reaction.hpp>
#include <Reaktoro/Core/ReactionSystem.hpp>
#include <Reaktoro/Core/Species.hpp>
#include <Reaktoro/Core/Utils.hpp>
#include <Reaktoro/Equilibrium/EquilibriumOptions.hpp>
#include <Reaktoro/Equilibrium/EquilibriumPath.hpp>
#include <Reaktoro/Equilibrium/EquilibriumProblem.hpp>
#include <Reaktoro/Equilibrium/EquilibriumResult.hpp>
#include <Reaktoro/Equilibrium/EquilibriumSolver.hpp>
#include <Reaktoro/Equilibrium/EquilibriumUtils.hpp>
#include <Reaktoro/Interfaces/Gems.hpp>
#include <Reaktoro/Interfaces/internal/PhreeqcUtils.hpp>
#include <Reaktoro/Interfaces/Phreeqx.hpp>
#include <Reaktoro/Kinetics/KineticOptions.hpp>
#include <Reaktoro/Kinetics/KineticProblem.hpp>
#include <Reaktoro/Kinetics/KineticResult.hpp>
#include <Reaktoro/Kinetics/KineticSolver.hpp>
#include <Reaktoro/Kinetics/KineticUtils.hpp>
#include <Reaktoro/Math/BilinearInterpolator.hpp>
#include <Reaktoro/Math/Derivatives.hpp>
#include <Reaktoro/Math/LagrangeInterpolator.hpp>
#include <Reaktoro/Math/MathUtils.hpp>
#include <Reaktoro/Math/ODE.hpp>
#include <Reaktoro/Math/Roots.hpp>
#include <Reaktoro/Optimization/Filter.hpp>
#include <Reaktoro/Optimization/Hessian.hpp>
#include <Reaktoro/Optimization/Jacobian.hpp>
#include <Reaktoro/Optimization/KktSolver.hpp>
#include <Reaktoro/Optimization/OptimumOptions.hpp>
#include <Reaktoro/Optimization/OptimumProblem.hpp>
#include <Reaktoro/Optimization/OptimumResult.hpp>
#include <Reaktoro/Optimization/OptimumSolver.hpp>
#include <Reaktoro/Optimization/OptimumSolverIpfeasible.hpp>
#include <Reaktoro/Optimization/OptimumSolverIpnewton.hpp>
#include <Reaktoro/Optimization/OptimumSolverIpopt.hpp>
#include <Reaktoro/Optimization/OptimumState.hpp>
#include <Reaktoro/Optimization/Utils.hpp>
#include <Reaktoro/Thermodynamics/Activity/AqueousActivity.hpp>
#include <Reaktoro/Thermodynamics/Activity/AqueousActivityDrummond.hpp>
#include <Reaktoro/Thermodynamics/Activity/AqueousActivityDuanSun.hpp>
#include <Reaktoro/Thermodynamics/Activity/AqueousActivityHKF.hpp>
#include <Reaktoro/Thermodynamics/Activity/AqueousActivityIdeal.hpp>
#include <Reaktoro/Thermodynamics/Activity/AqueousActivityPitzer.hpp>
#include <Reaktoro/Thermodynamics/Activity/AqueousActivityRumpf.hpp>
#include <Reaktoro/Thermodynamics/Activity/AqueousActivitySetschenow.hpp>
#include <Reaktoro/Thermodynamics/Activity/GaseousActivity.hpp>
#include <Reaktoro/Thermodynamics/Activity/GaseousActivityDuanMollerWeare.hpp>
#include <Reaktoro/Thermodynamics/Activity/GaseousActivityDuanSun.hpp>
#include <Reaktoro/Thermodynamics/Activity/GaseousActivityIdeal.hpp>
#include <Reaktoro/Thermodynamics/Activity/GaseousActivityPengRobinson.hpp>
#include <Reaktoro/Thermodynamics/Activity/GaseousActivitySpycherPruess.hpp>
#include <Reaktoro/Thermodynamics/Activity/GaseousActivitySpycherReed.hpp>
#include <Reaktoro/Thermodynamics/Activity/MineralActivity.hpp>
#include <Reaktoro/Thermodynamics/Activity/MineralActivityIdeal.hpp>
#include <Reaktoro/Thermodynamics/Core/ChemicalEditor.hpp>
#include <Reaktoro/Thermodynamics/Core/Database.hpp>
#include <Reaktoro/Thermodynamics/Core/Thermo.hpp>
#include <Reaktoro/Thermodynamics/Mixtures/AqueousMixture.hpp>
#include <Reaktoro/Thermodynamics/Mixtures/GaseousMixture.hpp>
#include <Reaktoro/Thermodynamics/Mixtures/GeneralMixture.hpp>
#include <Reaktoro/Thermodynamics/Mixtures/MineralMixture.hpp>
#include <Reaktoro/Thermodynamics/Models/SpeciesElectroState.hpp>
#include <Reaktoro/Thermodynamics/Models/SpeciesElectroStateHKF.hpp>
#include <Reaktoro/Thermodynamics/Models/SpeciesThermoState.hpp>
#include <Reaktoro/Thermodynamics/Models/SpeciesThermoStateHKF.hpp>
#include <Reaktoro/Thermodynamics/Phases/AqueousPhase.hpp>
#include <Reaktoro/Thermodynamics/Phases/GaseousPhase.hpp>
#include <Reaktoro/Thermodynamics/Phases/MineralPhase.hpp>
#include <Reaktoro/Thermodynamics/Reactions/MineralCatalyst.hpp>
#include <Reaktoro/Thermodynamics/Reactions/MineralMechanism.hpp>
#include <Reaktoro/Thermodynamics/Reactions/MineralReaction.hpp>
#include <Reaktoro/Thermodynamics/Species/AqueousSpecies.hpp>
#include <Reaktoro/Thermodynamics/Species/GaseousSpecies.hpp>
#include <Reaktoro/Thermodynamics/Species/MineralSpecies.hpp>
#include <Reaktoro/Thermodynamics/Species/ThermoData.hpp>
#include <Reaktoro/Thermodynamics/Water/WaterConstants.hpp>
#include <Reaktoro/Thermodynamics/Water/WaterElectroState.hpp>
#include <Reaktoro/Thermodynamics/Water/WaterElectroStateJohnsonNorton.hpp>
#include <Reaktoro/Thermodynamics/Water/WaterHelmholtzState.hpp>
#include <Reaktoro/Thermodynamics/Water/WaterHelmholtzStateHGK.hpp>
#include <Reaktoro/Thermodynamics/Water/WaterHelmholtzStateWagnerPruss.hpp>
#include <Reaktoro/Thermodynamics/Water/WaterThermoState.hpp>
#include <Reaktoro/Thermodynamics/Water/WaterThermoStateUtils.hpp>
#include <Reaktoro/Thermodynamics/Water/WaterUtils.hpp>
#include <Reaktoro/Utils/GeoUtils.hpp>
#include <Reaktoro/Utils/Rock.hpp>
