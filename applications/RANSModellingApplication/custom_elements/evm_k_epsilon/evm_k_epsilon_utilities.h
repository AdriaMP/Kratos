//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Suneth Warnakulasuriya (https://github.com/sunethwarna)
//

#if !defined(KRATOS_EVM_K_EPSILON_UTILITIES_H_INCLUDED)
#define KRATOS_EVM_K_EPSILON_UTILITIES_H_INCLUDED

// System includes

// Project includes
#include "custom_utilities/rans_calculation_utilities.h"
#include "geometries/geometry.h"
#include "includes/define.h"
#include "includes/ublas_interface.h"

namespace Kratos
{
///@name Kratos Globals
///@{

///@}
///@name Type Definitions
///@{

///@}
///@name  Enum's
///@{

///@}
///@name  Functions
///@{

namespace EvmKepsilonModelUtilities
{
double CalculateTurbulentViscosity(const double C_mu,
                                   const double turbulent_kinetic_energy,
                                   const double turbulent_energy_dissipation_rate,
                                   const double f_mu);

double CalculateFmu(const double y_plus);

double CalculateF2(const double turbulent_kinetic_energy,
                   const double kinematic_viscosity,
                   const double turbulent_energy_dissipation_rate);

template <unsigned int TDim>
double CalculateSourceTerm(const BoundedMatrix<double, TDim, TDim>& rVelocityGradient,
                           const double turbulent_kinematic_viscosity,
                           const double turbulent_kinetic_energy);

double CalculateGamma(const double C_mu,
                      const double f_mu,
                      const double turbulent_kinetic_energy,
                      const double turbulent_kinematic_viscosity);

void CalculateTurbulentValues(double& turbulent_kinetic_energy,
                              double& turbulent_energy_dissipation_rate,
                              const double y_plus,
                              const double kinematic_viscosity,
                              const double wall_distance,
                              const double c_mu,
                              const double von_karman);

void CalculateTurbulentValues(double& turbulent_kinetic_energy,
                              double& turbulent_energy_dissipation_rate,
                              const double velocity_mag,
                              const double turbulence_intensity,
                              const double mixing_length,
                              const double c_mu);

template <class NodeType>
bool HasConditionWithFlag(const typename Geometry<NodeType>::GeometriesArrayType& rConditionsArray,
                          const Flags& rFlag);

} // namespace EvmKepsilonModelUtilities

///@}

} // namespace Kratos

#endif