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

#include "ThermoStateUtils.hpp"

// C++ includes
#include <cmath>
#include <sstream>
#include <vector>
using std::pow;
using std::log;

// Reaktor includes
#include <Reaktor/Common/Constants.hpp>
#include <Reaktor/Common/ConvertUtils.hpp>
#include <Reaktor/Common/Exception.hpp>
#include <Reaktor/Species/AqueousSpecies.hpp>
#include <Reaktor/Species/GaseousSpecies.hpp>
#include <Reaktor/Species/MineralSpecies.hpp>
#include <Reaktor/Thermodynamics/AqueousElectroState.hpp>
#include <Reaktor/Thermodynamics/AqueousElectroStateUtils.hpp>
#include <Reaktor/Thermodynamics/ThermoState.hpp>
#include <Reaktor/Thermodynamics/WaterConstants.hpp>
#include <Reaktor/Thermodynamics/WaterElectroState.hpp>
#include <Reaktor/Thermodynamics/WaterElectroStateUtils.hpp>
#include <Reaktor/Thermodynamics/WaterThermoState.hpp>
#include <Reaktor/Thermodynamics/WaterThermoStateUtils.hpp>
#include <Reaktor/Thermodynamics/WaterUtils.hpp>

namespace Reaktor {
namespace internal {

/// The reference temperature assumed in the HKF equations of state (in units of K)
const double referenceTemperature = 298.15;

/// The reference temperature assumed in the HKF equations of state (in units of bar)
const double referencePressure = 1.0;

/// The reference dielectric constant of water \epsilon
const double referenceDielectricConstant = 78.24385513;

/// The reference Born function Z (dimensionless)
const double referenceBornZ = -1.278055636e-02;

/// The reference Born function Y (dimensionless)
const double referenceBornY = -5.795424563e-05;

/// The reference Born function Q (dimensionless)
const double referenceBornQ =  6.638388994e-12;

/// The reference Born function N (dimensionless)
const double referenceBornN = -2.321814455e-20;

/// The reference Born function U (dimensionless)
const double referenceBornU = 4.872982291e-14;

/// The reference Born function X (dimensionless)
const double referenceBornX = -3.060388224e-07;

/// The \eta constant in the HKF model (in units of (A*cal)/mol)
const double eta = 1.66027e+05;

/// The constant characteristics \Theta of the solvent (in units of K)
const double theta = 228;

/// The constant characteristics \Psi of the solvent (in units of bar)
const double psi = 2600;

template<class SpeciesType>
auto checkTemperatureValidityHKF(double T, const SpeciesType& species) -> void
{
    // Get the HKF thermodynamic data of the species
    const auto& hkf = species.thermoparams.hkf.get();

    // Check if given temperature is within the allowed range
	if(T < 0 or T > hkf.Tmax)
	{
	    Exception exception;
		exception.error << "Unable to calculate the thermodynamic properties of species "
		      << species.name << " using the revised HKF equations of state.";
		exception.reason << "The provided temperature, " << T << " K,"  << "is either negative "
              "or greater than the maximum allowed, " << hkf.Tmax << " K.";
		raise(exception);
	}
}

auto checkMineralDataHKF(const MineralSpecies& species) -> void
{
    const auto& hkf = species.thermoparams.hkf.get();

    if(not std::isfinite(hkf.Gf) or not std::isfinite(hkf.Hf) or
       not std::isfinite(hkf.Sr) or not std::isfinite(hkf.Vr))
    {
        Exception exception;
        exception.error << "Unable to calculate the thermodynamic properties of mineral species "
                        << species.name << " using the revised HKF equations of state.";
        exception.reason << "The database has incomplete thermodynamic data.";
        raise(exception);
    }
}

} /* namespace internal */

using namespace internal;

auto thermoStateSolventHKF(double T, double P, const WaterThermoState& wt) -> ThermoState
{
	// Auxiliary data from Helgeson and Kirkham (1974), on page 1098
	const double Ttr =  273.16;                   // unit: K
	const double Str =  15.1320 * calorieToJoule; // unit: J/(mol*K)
	const double Gtr = -56290.0 * calorieToJoule; // unit: J/mol
	const double Htr = -15971.0 * calorieToJoule; // unit: J/mol
	const double Utr = -15766.0 * calorieToJoule; // unit: J/mol
	const double Atr = -55415.0 * calorieToJoule; // unit: J/mol

	const double S = waterMolarMass * wt.entropy;         // unit: J/(mol*K)
	const double H = waterMolarMass * wt.enthalpy;        // unit: J/mol
	const double U = waterMolarMass * wt.internal_energy; // unit: J/mol

	// Calculate the standard molal thermodynamic properties of the aqueous species
	ThermoState state;

    state.entropy.val          = S + Str;
    state.enthalpy.val         = H + Htr;
    state.internal_energy.val  = U + Utr;
    state.gibbs_energy.val     = H - T * (S + Str) + Ttr * Str + Gtr;
    state.helmholtz_energy.val = U - T * (S + Str) + Ttr * Str + Atr;
    state.volume.val           = wt.volume * waterMolarMass;
    state.heat_capacity_cp.val = wt.cp * waterMolarMass;

	return state;
}

auto thermoStateSoluteHKF(double T, double P, const AqueousSpecies& species, const AqueousElectroState& aes, const WaterElectroState& wes) -> ThermoState
{
    // Get the HKF thermodynamic data of the species
    const auto& hkf = species.thermoparams.hkf();

	// Auxiliary variables
	const double Pbar = P * 1.0e-05;
	const double Tr   = referenceTemperature;
	const double Pr   = referencePressure;
	const double Zr   = referenceBornZ;
	const double Yr   = referenceBornY;
	const double Gf   = hkf.Gf;
	const double Hf   = hkf.Hf;
	const double Sr   = hkf.Sr;
	const double a1   = hkf.a1;
	const double a2   = hkf.a2;
	const double a3   = hkf.a3;
	const double a4   = hkf.a4;
	const double c1   = hkf.c1;
	const double c2   = hkf.c2;
	const double wr   = hkf.wref;
	const double w    = aes.w;
	const double wT   = aes.wT;
	const double wP   = aes.wP;
	const double wTT  = aes.wTT;
	const double Z    = wes.bornZ;
	const double Y    = wes.bornY;
	const double Q    = wes.bornQ;
	const double X    = wes.bornX;

	// Calculate the standard molal thermodynamic properties of the aqueous species
	ThermoState state;

	state.volume.val = a1 + a2/(psi + Pbar) +
		(a3 + a4/(psi + Pbar))/(T - theta) - w*Q - (Z + 1)*wP;

	state.gibbs_energy.val = Gf - Sr*(T - Tr) - c1*(T*log(T/Tr) - T + Tr)
		+ a1*(Pbar - Pr) + a2*log((psi + Pbar)/(psi + Pr))
		- c2*((1.0/(T - theta) - 1.0/(Tr - theta))*(theta - T)/theta
		- T/(theta*theta)*log(Tr/T * (T - theta)/(Tr - theta)))
		+ 1.0/(T - theta)*(a3*(Pbar - Pr) + a4*log((psi + Pbar)/(psi + Pr)))
		- w*(Z + 1) + wr*(Zr + 1) + wr*Yr*(T - Tr);

	state.enthalpy.val = Hf + c1*(T - Tr) - c2*(1.0/(T - theta) - 1.0/(Tr - theta))
		+ a1*(Pbar - Pr) + a2*log((psi + Pbar)/(psi + Pr))
		+ (2*T - theta)/pow(T - theta, 2)*(a3*(Pbar - Pr)
		+ a4*log((psi + Pbar)/(psi + Pr)))
		- w*(Z + 1) + w*T*Y + T*(Z + 1)*wT + wr*(Zr + 1) - wr*Tr*Yr;

	state.entropy.val = Sr + c1*log(T/Tr) - c2/theta*(1.0/(T - theta)
		- 1.0/(Tr - theta) + log(Tr/T * (T - theta)/(Tr - theta))/theta)
		+ 1.0/pow(T - theta, 2)*(a3*(Pbar - Pr) + a4*log((psi + Pbar)/(psi + Pr)))
		+ w*Y + (Z + 1)*wT - wr*Yr;

	state.heat_capacity_cp.val = c1 + c2/pow(T - theta, 2) - (2*T/pow(T - theta, 3))*(a3*(Pbar - Pr)
		+ a4*log((psi + Pbar)/(psi + Pr))) + w*T*X + 2*T*Y*wT + T*(Z + 1)*wTT;

	state.internal_energy.val = state.enthalpy.val - Pbar * state.volume.val;

	state.helmholtz_energy.val = state.internal_energy.val - T * state.entropy.val;

	// Convert the thermodynamic properties of the gas to the standard units
    state.volume.val           *= calorieToJoule / barToPascal;
    state.gibbs_energy.val     *= calorieToJoule;
    state.enthalpy.val         *= calorieToJoule;
    state.entropy.val          *= calorieToJoule;
    state.heat_capacity_cp.val *= calorieToJoule;
    state.internal_energy.val  *= calorieToJoule;
    state.helmholtz_energy.val *= calorieToJoule;

	return state;
}

auto thermoStateHKF(double T, double P, const AqueousSpecies& species) -> ThermoState
{
	WaterThermoState wt = waterThermoStateWagnerPruss(T, P);

	if(species.name == "H2O(l)")
		return thermoStateSolventHKF(T, P, wt);

    WaterElectroState wes = waterElectroStateJohnsonNorton(T, P, wt);

    FunctionG g = functionG(T, P, wt);

    AqueousElectroState aes = aqueousEletroStateHKF(g, species);

    return thermoStateSoluteHKF(T, P, species, aes, wes);
}

auto thermoStateHKF(double T, double P, const GaseousSpecies& species) -> ThermoState
{
	checkTemperatureValidityHKF(T, species);

	// Get the HKF thermodynamic data of the species
    const auto& hkf = species.thermoparams.hkf();

	// Auxiliary variables
	const double Tr = referenceTemperature;
	const double Gf = hkf.Gf;
	const double Hf = hkf.Hf;
	const double Sr = hkf.Sr;
	const double a  = hkf.a;
	const double b  = hkf.b;
	const double c  = hkf.c;

	// Calculate the integrals of the heal capacity function of the gas from Tr to T at constant pressure Pr
	const double Cp     = a + b*T + c/(T*T);
	const double CpdT   = a*(T - Tr) + 0.5*b*(T*T - Tr*Tr) - c*(1/T - 1/Tr);
	const double CpdlnT = a*log(T/Tr) + b*(T - Tr) - 0.5*c*(1/(T*T) - 1/(Tr*Tr));

	// Calculate the standard molal thermodynamic properties of the gas
	ThermoState ts;

    ts.volume.val           = 0.0;
    ts.gibbs_energy.val     = Gf - Sr * (T - Tr) + CpdT - T * CpdlnT;
    ts.enthalpy.val         = Hf + CpdT;
    ts.entropy.val          = Sr + CpdlnT;
    ts.heat_capacity_cp.val = Cp;
    ts.internal_energy.val  = ts.enthalpy.val;
    ts.helmholtz_energy.val = ts.internal_energy.val - T * ts.entropy.val;

	// Convert the thermodynamic properties of the gas to the standard units
    ts.volume.val           *= calorieToJoule / barToPascal;
    ts.gibbs_energy.val     *= calorieToJoule;
    ts.enthalpy.val         *= calorieToJoule;
    ts.entropy.val          *= calorieToJoule;
    ts.heat_capacity_cp.val *= calorieToJoule;
    ts.internal_energy.val  *= calorieToJoule;
    ts.helmholtz_energy.val *= calorieToJoule;

	return ts;
}

auto thermoStateHKF(double T, double P, const MineralSpecies& species) -> ThermoState
{
    // Check if the given temperature is valid for the HKF model of this species
	checkTemperatureValidityHKF(T, species);

	// Check if the HKF thermodynamic data of the mineral is indeed available
	checkMineralDataHKF(species);

	// Get the HKF thermodynamic data of the species
    const auto& hkf = species.thermoparams.hkf();

	// Auxiliary variables
	const auto  Pb   = P * 1.0e-5;
	const auto& Tr   = referenceTemperature;
	const auto& Pr   = referencePressure;
	const auto& Gf   = hkf.Gf;
	const auto& Hf   = hkf.Hf;
	const auto& Sr   = hkf.Sr;
	const auto& Vr   = hkf.Vr;
	const auto& nt   = hkf.nptrans;
	const auto& a    = hkf.a;
	const auto& b    = hkf.b;
	const auto& c    = hkf.c;
	const auto& Tt   = hkf.Ttr;
	const auto& dHt  = hkf.Htr;
	const auto& dVt  = hkf.Vtr;
	const auto& dPdT = hkf.dPdTtr;

	// Collect the temperature points used for the integrals along the pressure line P = Pr
	std::vector<double> Ti;

	Ti.push_back(Tr);

	for(int i = 0; i < nt; ++i)
		if(T > Tt[i]) Ti.push_back(Tt[i]);

	Ti.push_back(T);

	// Collect the pressure intercepts along the temperature line T for every phase transition boundary (see
	std::vector<double> Pt;
	for(int i = 0; i < nt; ++i)
	{
		if(dPdT[i] != 0.0)
			Pt.push_back(Pr + dPdT[i]*(T - Tt[i]));
	}

	// Calculate the integrals of the heal capacity function of the mineral from Tr to T at constant pressure Pr
	double CpdT = 0.0;
	double CpdlnT = 0.0;
	for(unsigned i = 0; i+1 < Ti.size(); ++i)
	{
		const double T0 = Ti[i];
		const double T1 = Ti[i+1];

		CpdT += a[i]*(T1 - T0) + 0.5*b[i]*(T1*T1 - T0*T0) - c[i]*(1/T1 - 1/T0);
		CpdlnT += a[i]*log(T1/T0) + b[i]*(T1 - T0) - 0.5*c[i]*(1/(T1*T1) - 1/(T0*T0));
	}

	// Calculate the volume and other auxiliary quantities for the thermodynamic properties of the mineral
	double V = Vr;
	double GdH = 0.0;
	double HdH = 0.0;
	double SdH = 0.0;
	for(unsigned i = 1; i+1 < Ti.size(); ++i)
	{
		GdH += dHt[i-1]*(T - Ti[i])/Ti[i];
		HdH += dHt[i-1];
		SdH += dHt[i-1]/Ti[i];

		V += dVt[i-1];
	}

	// Calculate the volume integral from Pr to P at constant temperature T
	double VdP = 0.023901488*V*(Pb - Pr);
	for(unsigned i = 0; i < Pt.size(); ++i)
	{
		if(0.0 < Pt[i] and Pt[i] < Pb)
		{
			V   -= dVt[i];
			VdP -= 0.023901488*dVt[i]*(Pb - Pt[i]);
		}
	}

	// Calculate the standard molal thermodynamic properties of the mineral
	ThermoState st;

    st.gibbs_energy.val = Gf - Sr * (T - Tr) + CpdT - T * CpdlnT + VdP - GdH;
    st.enthalpy.val     = Hf + CpdT + VdP + HdH;
    st.entropy.val      = Sr + CpdlnT + SdH;
    st.volume.val       = V;

	// Convert the thermodynamic properties of the mineral to the standard
    st.volume.val           *= cubicCentimeterToCubicMeter;
    st.gibbs_energy.val     *= calorieToJoule;
    st.enthalpy.val         *= calorieToJoule;
    st.entropy.val          *= calorieToJoule;
    st.heat_capacity_cp.val *= calorieToJoule;
    st.internal_energy.val  *= calorieToJoule;
    st.helmholtz_energy.val *= calorieToJoule;

	return st;
}

} // namespace Reaktor
