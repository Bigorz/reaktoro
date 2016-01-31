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
#include <Reaktoro/Common/ThermoScalar.hpp>

namespace Reaktoro {

/// A template base class to represent a chemical scalar and its partial derivatives.
/// A *chemical scalar* is a quantity that depends on temperature, pressure,
/// and molar amounts of species.
/// @see ThermoScalar, ChemicalVector, ThermoVector
template<typename V, typename N>
class ChemicalScalarBase
{
public:
    /// The value of the chemical scalar.
    V val;

    /// The partial temperature derivative of the chemical scalar.
    V ddt;

    /// The partial pressure derivative of the chemical scalar.
    V ddp;

    /// The partial molar derivatives of the chemical scalar.
    N ddn;

    /// Return a ChemicalScalar with zero and zero derivatives.
    /// @param nspecies The number of species for the molar derivatives
    static auto Zero(Index nspecies) -> ChemicalScalarBase
    {
        return ChemicalScalarBase<V,N>(nspecies, 0.0);
    }

    /// Return a ChemicalScalar with one and zero derivatives.
    /// @param nspecies The number of species for the molar derivatives
    static auto One(Index nspecies) -> ChemicalScalarBase
    {
        return ChemicalScalarBase<V,N>(nspecies, 1.0);
    }

    /// Return a ChemicalScalar with a given constant and zero derivatives.
    /// @param nspecies The number of species for the molar derivatives
    static auto Constant(Index nspecies, double val) -> ChemicalScalarBase
    {
        return ChemicalScalarBase<V,N>(nspecies, val);
    }

    /// Construct a default ChemicalScalarBase instance.
    ChemicalScalarBase() {}

    /// Construct a ChemicalScalarBase instance with given number of species.
    /// @param nspecies The number of species for the molar derivatives
    explicit ChemicalScalarBase(Index nspecies)
    : ChemicalScalarBase<V,N>(0.0, 0.0, 0.0, zeros(nspecies)) {}

    /// Construct a ChemicalScalarBase instance with given number of species and a constant value.
    /// @param nspecies The number of species for the molar derivatives
    /// @param val The constant value
    ChemicalScalarBase(Index nspecies, double val)
    : ChemicalScalarBase<V,N>(val, 0.0, 0.0, zeros(nspecies)) {}

    /// Construct a ChemicalScalarBase instance with given values and derivatives.
    /// @param val The value of the chemical scalar
    /// @param ddt The partial temperature derivative of the chemical scalar
    /// @param ddp The partial pressure derivative of the chemical scalar
    /// @param ddn The vector of partial molar derivatives of the chemical scalar
    ChemicalScalarBase(const V& val, const V& ddt, const V& ddp, const N& ddn)
    : val(val), ddt(ddt), ddp(ddp), ddn(ddn) {}

    /// Construct a ChemicalScalarBase instance from another.
    template<typename VR, typename NR>
    ChemicalScalarBase(const ChemicalScalarBase<VR,NR>& other)
    : val(other.val), ddt(other.ddt), ddp(other.ddp), ddn(other.ddn) {}

    /// Assign another ChemicalScalarBase instance to this.
    template<typename VR, typename NR>
    auto operator=(const ChemicalScalarBase<VR,NR>& other) -> ChemicalScalarBase&
    {
        val = other.val;
        ddt = other.ddt;
        ddp = other.ddp;
        ddn = other.ddn;
        return *this;
    }

    /// Assign a ThermoScalarBase instance to this.
    template<typename VR>
    auto operator=(const ThermoScalarBase<VR>& other) -> ChemicalScalarBase&
    {
        val = other.val;
        ddt = other.ddt;
        ddp = other.ddp;
        return *this;
    }

    /// Assign a scalar to this.
    auto operator=(double other) -> ChemicalScalarBase&
    {
        val = other;
        return *this;
    }

    /// Assign-addition of a ChemicalScalarBase instance to this.
    template<typename VR, typename NR>
    auto operator+=(const ChemicalScalarBase<VR,NR>& other) -> ChemicalScalarBase&
    {
        val += other.val;
        ddt += other.ddt;
        ddp += other.ddp;
        ddn += other.ddn;
        return *this;
    }

    /// Assign-addition of a ThermoScalarBase instance to this.
    template<typename VR>
    auto operator+=(const ThermoScalarBase<VR>& other) -> ChemicalScalarBase&
    {
        val += other.val;
        ddt += other.ddt;
        ddp += other.ddp;
        return *this;
    }

    /// Assign-addition of a scalar to this.
    auto operator+=(double other) -> ChemicalScalarBase&
    {
        val += other;
        return *this;
    }

    /// Assign-subtraction of a ChemicalScalarBase instance to this.
    template<typename VR, typename NR>
    auto operator-=(const ChemicalScalarBase<VR,NR>& other) -> ChemicalScalarBase&
    {
        val -= other.val;
        ddt -= other.ddt;
        ddp -= other.ddp;
        ddn -= other.ddn;
        return *this;
    }

    /// Assign-subtraction of a ThermoScalarBase instance to this.
    template<typename VR>
    auto operator-=(const ThermoScalarBase<VR>& other) -> ChemicalScalarBase&
    {
        val -= other.val;
        ddt -= other.ddt;
        ddp -= other.ddp;
        return *this;
    }

    /// Assign-subtraction of a scalar to this.
    auto operator-=(double other) -> ChemicalScalarBase&
    {
        val -= other;
        return *this;
    }

    /// Assign-multiplication of a ChemicalScalarBase instance to this.
    template<typename VR, typename NR>
    auto operator*=(const ChemicalScalarBase<VR,NR>& other) -> ChemicalScalarBase&
    {
        ddt = ddt * other.val + val * other.ddt;
        ddp = ddp * other.val + val * other.ddp;
        ddn = ddn * other.val + val * other.ddn;
        val *= other.val;
        return *this;
    }

    /// Assign-multiplication of a scalar to this.
    auto operator*=(double other) -> ChemicalScalarBase&
    {
        val *= other;
        ddt *= other;
        ddp *= other;
        ddn *= other;
        return *this;
    }

    /// Assign-division of a scalar to this.
    auto operator/=(double other) -> ChemicalScalarBase&
    {
        *this *= 1.0/other;
        return *this;
    }

    /// Explicitly converts this ChemicalScalarBase instance into a double.
    explicit operator double()
    {
        return val;
    }
};

/// A type that represents a chemical scalar and its derivatives.
/// A *chemical scalar* is a quantity that depends on temperature, pressure,
/// and molar amounts of species. A ChemicalScalar holds not only its value,
/// but also its temperature, pressure, and molar partial derivatives.
/// @see ThermoScalar, ChemicalVector, ThermoVector
using ChemicalScalar = ChemicalScalarBase<double,Vector>;

template<typename V, typename N>
auto operator+(const ChemicalScalarBase<V,N>& l) -> ChemicalScalarBase<V,N>
{
    return l;
}

template<typename V, typename N>
auto operator-(const ChemicalScalarBase<V,N>& l) -> ChemicalScalarBase<double, decltype(-l.ddn)>
{
    return {-l.val, -l.ddt, -l.ddp, -l.ddn};
}

template<typename VL, typename NL, typename VR, typename NR>
auto operator+(const ChemicalScalarBase<VL,NL>& l, const ChemicalScalarBase<VR,NR>& r) -> ChemicalScalarBase<double, decltype(l.ddn + r.ddn)>
{
    return {l.val + r.val, l.ddt + r.ddt, l.ddp + r.ddp, l.ddn + r.ddn};
}

template<typename VL, typename NL, typename VR>
auto operator+(const ChemicalScalarBase<VL,NL>& l, const ThermoScalarBase<VR>& r) -> ChemicalScalarBase<double, decltype(l.ddn)>
{
    return {l.val + r.val, l.ddt + r.ddt, l.ddp + r.ddp, l.ddn};
}

template<typename VL, typename VR, typename NR>
auto operator+(const ThermoScalarBase<VL>& l, const ChemicalScalarBase<VR,NR>& r) -> decltype(r + l)
{
    return r + l;
}

template<typename V, typename N>
auto operator+(const ChemicalScalarBase<V,N>& l, double r) -> ChemicalScalarBase<double, decltype(l.ddn)>
{
    return {l.val + r, l.ddt, l.ddp, l.ddn};
}

template<typename V, typename N>
auto operator+(double l, const ChemicalScalarBase<V,N>& r) -> decltype(r + l)
{
    return r + l;
}

template<typename VL, typename NL, typename VR, typename NR>
auto operator-(const ChemicalScalarBase<VL,NL>& l, const ChemicalScalarBase<VR,NR>& r) -> ChemicalScalarBase<double, decltype(l.ddn - r.ddn)>
{
    return {l.val - r.val, l.ddt - r.ddt, l.ddp - r.ddp, l.ddn - r.ddn};
}

template<typename VL, typename NL, typename VR>
auto operator-(const ChemicalScalarBase<VL,NL>& l, const ThermoScalarBase<VR>& r) -> ChemicalScalarBase<double, decltype(l.ddn)>
{
    return {l.val - r.val, l.ddt - r.ddt, l.ddp - r.ddp, l.ddn};
}

template<typename VL, typename VR, typename NR>
auto operator-(const ThermoScalarBase<VL>& l, const ChemicalScalarBase<VR,NR>& r) -> ChemicalScalarBase<double, decltype(-r.ddn)>
{
    return {l.val - r.val, l.ddt - r.ddt, l.ddp - r.ddp, -r.ddn};
}

template<typename V, typename N>
auto operator-(const ChemicalScalarBase<V,N>& l, double r) -> ChemicalScalarBase<double, decltype(l.ddn)>
{
    return {l.val - r, l.ddt, l.ddp, l.ddn};
}

template<typename V, typename N>
auto operator-(double l, const ChemicalScalarBase<V,N>& r) -> ChemicalScalarBase<double, decltype(-r.ddn)>
{
    return {l - r.val, -r.ddt, -r.ddp, -r.ddn};
}

template<typename V, typename N>
auto operator*(double l, const ChemicalScalarBase<V,N>& r) -> ChemicalScalarBase<double, decltype(l * r.ddn)>
{
    return {l * r.val, l * r.ddt, l * r.ddp, l * r.ddn};
}

template<typename V, typename N>
auto operator*(const ChemicalScalarBase<V,N>& l, double r) -> decltype(r * l)
{
    return r * l;
}

template<typename VL, typename VR, typename NR>
auto operator*(const ThermoScalarBase<VL>& l, const ChemicalScalarBase<VR,NR>& r) -> ChemicalScalarBase<double, decltype(l.val * r.ddn)>
{
    return {l.val * r.val, l.val * r.ddt + l.ddt * r.val, l.val * r.ddp + l.ddp * r.val, l.val * r.ddn};
}

template<typename VL, typename NL, typename VR>
auto operator*(const ChemicalScalarBase<VL,NL>& l, const ThermoScalarBase<VR>& r) -> decltype(r * l)
{
    return r * l;
}

template<typename VL, typename NL, typename VR, typename NR>
auto operator*(const ChemicalScalarBase<VL,NL>& l, const ChemicalScalarBase<VR,NR>& r) -> ChemicalScalarBase<double, decltype(l.val * r.ddn + l.ddn * r.val)>
{
    return {l.val * r.val, l.val * r.ddt + l.ddt * r.val, l.val * r.ddp + l.ddp * r.val, l.val * r.ddn + l.ddn * r.val};
}

template<typename V, typename N>
auto operator/(double l, const ChemicalScalarBase<V,N>& r) -> ChemicalScalarBase<double, decltype(double() * r.ddn)>
{
    const double tmp1 = 1.0/r.val;
    const double tmp2 = -l * tmp1 * tmp1;
    return {tmp1 * l, tmp2 * r.ddt, tmp2 * r.ddp, tmp2 * r.ddn};
}

template<typename V, typename N>
auto operator/(const ChemicalScalarBase<V,N>& l, double r) -> decltype((1.0/r) * l)
{
    return (1.0/r) * l;
}

template<typename VL, typename VR, typename NR>
auto operator/(const ThermoScalarBase<VL>& l, const ChemicalScalarBase<VR,NR>& r) -> ChemicalScalarBase<double, decltype(-(l.val * r.ddn) * double())>
{
    const double tmp = 1.0/(r.val * r.val);
    return {l.val/r.val, (l.ddt * r.val - l.val * r.ddt) * tmp, (l.ddp * r.val - l.val * r.ddp) * tmp, -(l.val * r.ddn) * tmp};
}

template<typename VL, typename NL, typename VR>
auto operator/(const ChemicalScalarBase<VL,NL>& l, const ThermoScalarBase<VR>& r) -> ChemicalScalarBase<double, decltype(l.ddn/r.val)>
{
    const double tmp = 1.0/(r.val * r.val);
    return {l.val/r.val, (l.ddt * r.val - l.val * r.ddt) * tmp, (l.ddp * r.val - l.val * r.ddp) * tmp, l.ddn/r.val};
}

template<typename VL, typename NL, typename VR, typename NR>
auto operator/(const ChemicalScalarBase<VL,NL>& l, const ChemicalScalarBase<VR,NR>& r) -> ChemicalScalarBase<double, decltype((l.ddn * r.val - l.val * r.ddn) * double())>
{
    const double tmp = 1.0/(r.val * r.val);
    return {l.val/r.val, (l.ddt * r.val - l.val * r.ddt) * tmp, (l.ddp * r.val - l.val * r.ddp) * tmp, (l.ddn * r.val - l.val * r.ddn) * tmp};
}

template<typename V, typename N>
auto sqrt(const ChemicalScalarBase<V,N>& l) -> ChemicalScalarBase<double, decltype(double() * l.ddn)>
{
    const double tmp1 = std::sqrt(l.val);
    const double tmp2 = 0.5 * tmp1/l.val;
    return {tmp1, tmp2 * l.ddt, tmp2 * l.ddp, tmp2 * l.ddn};
}

template<typename V, typename N>
auto pow(const ChemicalScalarBase<V,N>& l, double power) -> ChemicalScalarBase<double, decltype(double() * l.ddn)>
{
    const double tmp1 = std::pow(l.val, power);
    const double tmp2 = power * tmp1/l.val;
    return {tmp1, tmp2 * l.ddt, tmp2 * l.ddp, tmp2 * l.ddn};
}

template<typename V, typename N>
auto exp(const ChemicalScalarBase<V,N>& l) -> ChemicalScalarBase<double, decltype(double() * l.ddn)>
{
    const double tmp1 = std::exp(l.val);
    return {tmp1, tmp1 * l.ddt, tmp1 * l.ddp, tmp1 * l.ddn};
}

template<typename V, typename N>
auto log(const ChemicalScalarBase<V,N>& l) -> ChemicalScalarBase<double, decltype(double() * l.ddn)>
{
    const double tmp1 = std::log(l.val);
    const double tmp2 = 1.0/l.val;
    return {tmp1, tmp2 * l.ddt, tmp2 * l.ddp, tmp2 * l.ddn};
}

template<typename V, typename N>
auto log10(const ChemicalScalarBase<V,N>& l) -> decltype(log(l)/1.0)
{
    const double ln10 = 2.302585092994046;
    return log(l)/ln10;
}

template<typename VL, typename NL, typename VR, typename NR>
auto operator<(const ChemicalScalarBase<VL,NL>& l, const ChemicalScalarBase<VR,NR>& r) -> bool
{
    return l.val < r.val;
}

template<typename V, typename N>
auto operator<(const ChemicalScalarBase<V,N>& l, double r) -> bool
{
    return l.val < r;
}

template<typename V, typename N>
auto operator<(double l, const ChemicalScalarBase<V,N>& r) -> bool
{
    return l < r.val;
}

template<typename VL, typename NL, typename VR, typename NR>
auto operator<=(const ChemicalScalarBase<VL,NL>& l, const ChemicalScalarBase<VR,NR>& r) -> bool
{
    return l.val <= r.val;
}

template<typename V, typename N>
auto operator<=(const ChemicalScalarBase<V,N>& l, double r) -> bool
{
    return l.val <= r;
}

template<typename V, typename N>
auto operator<=(double l, const ChemicalScalarBase<V,N>& r) -> bool
{
    return l <= r.val;
}

template<typename VL, typename NL, typename VR, typename NR>
auto operator>(const ChemicalScalarBase<VL,NL>& l, const ChemicalScalarBase<VR,NR>& r) -> bool
{
    return l.val > r.val;
}

template<typename V, typename N>
auto operator>(const ChemicalScalarBase<V,N>& l, double r) -> bool
{
    return l.val > r;
}

template<typename V, typename N>
auto operator>(double l, const ChemicalScalarBase<V,N>& r) -> bool
{
    return l > r.val;
}

template<typename VL, typename NL, typename VR, typename NR>
auto operator>=(const ChemicalScalarBase<VL,NL>& l, const ChemicalScalarBase<VR,NR>& r) -> bool
{
    return l.val >= r.val;
}

template<typename V, typename N>
auto operator>=(const ChemicalScalarBase<V,N>& l, double r) -> bool
{
    return l.val >= r;
}

template<typename V, typename N>
auto operator>=(double l, const ChemicalScalarBase<V,N>& r) -> bool
{
    return l >= r.val;
}

template<typename VL, typename NL, typename VR, typename NR>
auto operator==(const ChemicalScalarBase<VL,NL>& l, const ChemicalScalarBase<VR,NR>& r) -> bool
{
    return l.val == r.val;
}

template<typename V, typename N>
auto operator==(const ChemicalScalarBase<V,N>& l, double r) -> bool
{
    return l.val == r;
}

template<typename V, typename N>
auto operator==(double l, const ChemicalScalarBase<V,N>& r) -> bool
{
    return l == r.val;
}

template<typename VL, typename NL, typename VR, typename NR>
auto operator!=(const ChemicalScalarBase<VL,NL>& l, const ChemicalScalarBase<VR,NR>& r) -> bool
{
    return l.val == r.val;
}

template<typename V, typename N>
auto operator!=(const ChemicalScalarBase<V,N>& l, double r) -> bool
{
    return l.val != r;
}

template<typename V, typename N>
auto operator!=(double l, const ChemicalScalarBase<V,N>& r) -> bool
{
    return l != r.val;
}

inline auto operator<<(std::ostream& out, const ChemicalScalar& scalar) -> std::ostream&
{
    out << scalar.val;
    return out;
}

} // namespace Reaktoro
