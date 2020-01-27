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
#include <cmath>
#include <limits>

// External includes

// Project includes
#include "includes/cfd_variables.h"
#include "includes/define.h"

// Application includes
#include "custom_elements/evm_k_epsilon/evm_k_epsilon_utilities.h"
#include "custom_utilities/rans_check_utilities.h"
#include "rans_application_variables.h"

// Include base h
#include "rans_nut_k_epsilon_high_re_calculation_process.h"

namespace Kratos
{
RansNutKEpsilonHighReCalculationProcess::RansNutKEpsilonHighReCalculationProcess(Model& rModel, Parameters rParameters)
    : mrModel(rModel), mrParameters(rParameters)
{
    KRATOS_TRY

    Parameters default_parameters = Parameters(R"(
        {
            "model_part_name"          : "PLEASE_SPECIFY_MODEL_PART_NAME",
            "echo_level"               : 0,
            "c_mu"                     : 0.09,
            "min_value"                : 1e-15,
            "admissible_mixing_length" : 0.0
        })");

    mrParameters.ValidateAndAssignDefaults(default_parameters);

    mEchoLevel = mrParameters["echo_level"].GetInt();
    mModelPartName = mrParameters["model_part_name"].GetString();
    mCmu = mrParameters["c_mu"].GetDouble();
    mMinValue = mrParameters["min_value"].GetDouble();
    mMixingLength = mrParameters["admissible_mixing_length"].GetDouble();

    KRATOS_ERROR_IF(mMixingLength <= 0.0)
        << "Admisible mixing length should be positive. "
           "[\"admissible_mixing_length\" = "
        << mMixingLength << " <= 0.0 ]\n";

    KRATOS_CATCH("");
}

int RansNutKEpsilonHighReCalculationProcess::Check()
{
    KRATOS_TRY

    RansCheckUtilities::CheckIfModelPartExists(mrModel, mModelPartName);

    const ModelPart& r_model_part = mrModel.GetModelPart(mModelPartName);

    RansCheckUtilities::CheckIfVariableExistsInModelPart(r_model_part, TURBULENT_KINETIC_ENERGY);
    RansCheckUtilities::CheckIfVariableExistsInModelPart(
        r_model_part, TURBULENT_ENERGY_DISSIPATION_RATE);
    RansCheckUtilities::CheckIfVariableExistsInModelPart(r_model_part, TURBULENT_VISCOSITY);

    return 0;

    KRATOS_CATCH("");
}

void RansNutKEpsilonHighReCalculationProcess::Execute()
{
    KRATOS_TRY

    ModelPart& r_model_part = mrModel.GetModelPart(mModelPartName);

    NodesContainerType& r_nodes = r_model_part.Nodes();
    const int number_of_nodes = r_nodes.size();

#pragma omp parallel for
    for (int i_node = 0; i_node < number_of_nodes; ++i_node)
    {
        NodeType& r_node = *(r_nodes.begin() + i_node);
        const double epsilon =
            r_node.FastGetSolutionStepValue(TURBULENT_ENERGY_DISSIPATION_RATE);
        const double tke = r_node.FastGetSolutionStepValue(TURBULENT_KINETIC_ENERGY);

        double& nu_t = r_node.FastGetSolutionStepValue(TURBULENT_VISCOSITY);
        double limiting_mixing_length = mMixingLength;

        if (tke >= 0.0)
        {
            const double value = mCmu * std::pow(tke, 1.5);
            if (value < (epsilon * mMixingLength))
            {
                limiting_mixing_length = value / epsilon;
            }
            nu_t = std::max(limiting_mixing_length * std::sqrt(tke), mMinValue);
        }
        else
        {
            nu_t = mMinValue;
        }
    }

    KRATOS_INFO_IF(this->Info(), mEchoLevel > 1)
        << "Calculated nu_t for nodes in" << mModelPartName << "\n";

    KRATOS_CATCH("");
}

std::string RansNutKEpsilonHighReCalculationProcess::Info() const
{
    return std::string("RansNutKEpsilonHighReCalculationProcess");
}

void RansNutKEpsilonHighReCalculationProcess::PrintInfo(std::ostream& rOStream) const
{
    rOStream << this->Info();
}

void RansNutKEpsilonHighReCalculationProcess::PrintData(std::ostream& rOStream) const
{
}

} // namespace Kratos.
