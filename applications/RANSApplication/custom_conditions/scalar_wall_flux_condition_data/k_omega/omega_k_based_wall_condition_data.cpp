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

// System includes

// Project includes
#include "includes/cfd_variables.h"
#include "includes/checks.h"
#include "includes/define.h"
#include "includes/variables.h"

// Application includes
#include "custom_utilities/rans_calculation_utilities.h"
#include "rans_application_variables.h"

// Include base h
#include "omega_k_based_wall_condition_data.h"

namespace Kratos
{
namespace KOmegaWallConditionData
{
const Variable<double>& OmegaKBasedWallConditionData::GetScalarVariable()
{
    return TURBULENT_SPECIFIC_ENERGY_DISSIPATION_RATE;
}

const Variable<double>& OmegaKBasedWallConditionData::GetScalarRateVariable()
{
    return TURBULENT_SPECIFIC_ENERGY_DISSIPATION_RATE_2;
}

void OmegaKBasedWallConditionData::Check(const GeometryType& rGeometry,
                                         const ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY
    const int number_of_nodes = rGeometry.PointsNumber();

    for (int i_node = 0; i_node < number_of_nodes; ++i_node)
    {
        const NodeType& r_node = rGeometry[i_node];
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(KINEMATIC_VISCOSITY, r_node);
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(TURBULENT_VISCOSITY, r_node);
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(TURBULENT_KINETIC_ENERGY, r_node);
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(
            TURBULENT_SPECIFIC_ENERGY_DISSIPATION_RATE, r_node);
        KRATOS_CHECK_VARIABLE_IN_NODAL_DATA(
            TURBULENT_SPECIFIC_ENERGY_DISSIPATION_RATE_2, r_node);

        KRATOS_CHECK_DOF_IN_NODE(TURBULENT_SPECIFIC_ENERGY_DISSIPATION_RATE, r_node);
    }

    KRATOS_CATCH("");
}
GeometryData::IntegrationMethod OmegaKBasedWallConditionData::GetIntegrationMethod()
{
    return GeometryData::GI_GAUSS_1;
}

void OmegaKBasedWallConditionData::CalculateConstants(const ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY

    mOmegaSigma = rCurrentProcessInfo[TURBULENT_SPECIFIC_ENERGY_DISSIPATION_RATE_SIGMA];
    mKappa = rCurrentProcessInfo[WALL_VON_KARMAN];
    mCmu25 = std::pow(rCurrentProcessInfo[TURBULENCE_RANS_C_MU], 0.25);

    KRATOS_ERROR_IF(!(this->GetGeometry().Has(RANS_Y_PLUS)))
        << "RANS_Y_PLUS value is not set at " << this->GetGeometry() << "\n";

    const double y_plus_limit = rCurrentProcessInfo[RANS_LINEAR_LOG_LAW_Y_PLUS_LIMIT];
    mYPlus = std::max(this->GetGeometry().GetValue(RANS_Y_PLUS), y_plus_limit);

    KRATOS_CATCH("");
}

bool OmegaKBasedWallConditionData::IsWallFluxComputable() const
{
    return true;
}

double OmegaKBasedWallConditionData::CalculateWallFlux(const Vector& rShapeFunctions) const
{
    const double nu = RansCalculationUtilities::EvaluateInPoint(
        this->GetGeometry(), KINEMATIC_VISCOSITY, rShapeFunctions);
    const double nu_t = RansCalculationUtilities::EvaluateInPoint(
        this->GetGeometry(), TURBULENT_VISCOSITY, rShapeFunctions);
    const double tke = RansCalculationUtilities::EvaluateInPoint(
        this->GetGeometry(), TURBULENT_KINETIC_ENERGY, rShapeFunctions);
    const double u_tau = mCmu25 * std::sqrt(std::max(tke, 0.0));

    return (nu + mOmegaSigma * nu_t) * std::pow(u_tau, 3) /
           (mKappa * std::pow(mCmu25 * mYPlus * nu, 2));
}

} // namespace KOmegaWallConditionData

} // namespace Kratos