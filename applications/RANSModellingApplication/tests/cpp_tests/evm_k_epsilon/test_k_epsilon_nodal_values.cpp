//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Suneth Warnakulasuriya
//
//

// System includes
#include <functional>
#include <vector>

// External includes

// Project includes
#include "containers/model.h"
#include "testing/testing.h"

// Application includes
#include "custom_elements/evm_k_epsilon/evm_k_epsilon_adjoint_utilities.h"
#include "custom_processes/y_plus_model_processes/rans_logarithmic_y_plus_model_process.h"
#include "custom_processes/y_plus_model_processes/rans_logarithmic_y_plus_model_sensitivities_process.h"
#include "custom_utilities/rans_calculation_utilities.h"
#include "test_k_epsilon_utilities.h"
#include "custom_utilities/test_utilities.h"

namespace Kratos
{
namespace Testing
{
KRATOS_TEST_CASE_IN_SUITE(RansEvmKEpsilonNodalTKESensitivities, RANSEvModelsKEpsilonNodalMatrices)
{
    Model model;
    ModelPart& r_model_part = model.CreateModelPart("RansSensitivities");
    RansEvmKEpsilonModel::GenerateRansEvmKEpsilonTestModelPart(r_model_part,
                                                               "RANSEVMK2D3N");

    Parameters empty_parameters = Parameters(R"({})");
    RansLogarithmicYPlusModelProcess y_plus_model_process(r_model_part, empty_parameters);

    auto perturb_variable = [](NodeType& rNode) -> double& {
        return rNode.FastGetSolutionStepValue(TURBULENT_KINETIC_ENERGY);
    };

    auto calculate_sensitivities = [](std::vector<Vector>& rValues,
                                      const ElementType& rElement,
                                      const ProcessInfo& rCurrentProcessInfo) {
        RansCalculationUtilities rans_calculation_utilities;

        const double c_mu = rCurrentProcessInfo[TURBULENCE_RANS_C_MU];

        const GeometryType& r_geometry = rElement.GetGeometry();
        const int number_of_nodes = r_geometry.PointsNumber();

        Vector nodal_y_plus(number_of_nodes);
        Vector nodal_tke(number_of_nodes);
        Vector nodal_epsilon(number_of_nodes);
        Vector nodal_nu_t(number_of_nodes);
        Vector nodal_f_mu(number_of_nodes);

        RansEvmKEpsilonModel::ReadNodalDataFromElement(
            nodal_y_plus, nodal_tke, nodal_epsilon, nodal_nu_t, nodal_f_mu, rElement);

        Vector nodal_nu_t_sensitivities(number_of_nodes);
        EvmKepsilonModelAdjointUtilities::CalculateNodalTurbulentViscosityTKESensitivities(
            nodal_nu_t_sensitivities, c_mu, nodal_tke, nodal_epsilon, nodal_f_mu);

        rValues.clear();
        rValues.push_back(nodal_nu_t_sensitivities);
    };

    auto calculate_primal_quantities = [](std::vector<double>& rSensitivities,
                                          const NodeType& rNode,
                                          const ProcessInfo& rCurrentProcessInfo) {
        rSensitivities.push_back(rNode.FastGetSolutionStepValue(TURBULENT_VISCOSITY));
    };

    RansModellingApplicationTestUtilities::RunNodalScalarSensitivityTest(
        r_model_part, y_plus_model_process, calculate_primal_quantities,
        calculate_sensitivities, RansEvmKEpsilonModel::UpdateVariablesInModelPart,
        perturb_variable, 1e-7, 1e-6);
}

KRATOS_TEST_CASE_IN_SUITE(RansEvmKEpsilonNodalEpsilonSensitivities, RANSEvModelsKEpsilonNodalMatrices)
{
    Model model;
    ModelPart& r_model_part = model.CreateModelPart("RansSensitivities");
    RansEvmKEpsilonModel::GenerateRansEvmKEpsilonTestModelPart(r_model_part,
                                                               "RANSEVMK2D3N");

    Parameters empty_parameters = Parameters(R"({})");
    RansLogarithmicYPlusModelProcess y_plus_model_process(r_model_part, empty_parameters);

    auto perturb_variable = [](NodeType& rNode) -> double& {
        return rNode.FastGetSolutionStepValue(TURBULENT_ENERGY_DISSIPATION_RATE);
    };

    auto calculate_sensitivities = [](std::vector<Vector>& rValues,
                                      const ElementType& rElement,
                                      const ProcessInfo& rCurrentProcessInfo) {
        RansCalculationUtilities rans_calculation_utilities;

        const double c_mu = rCurrentProcessInfo[TURBULENCE_RANS_C_MU];

        const GeometryType& r_geometry = rElement.GetGeometry();
        const int number_of_nodes = r_geometry.PointsNumber();

        Vector nodal_y_plus(number_of_nodes);
        Vector nodal_tke(number_of_nodes);
        Vector nodal_epsilon(number_of_nodes);
        Vector nodal_nu_t(number_of_nodes);
        Vector nodal_f_mu(number_of_nodes);

        RansEvmKEpsilonModel::ReadNodalDataFromElement(
            nodal_y_plus, nodal_tke, nodal_epsilon, nodal_nu_t, nodal_f_mu, rElement);

        Vector nodal_nu_t_sensitivities;
        EvmKepsilonModelAdjointUtilities::CalculateNodalTurbulentViscosityEpsilonSensitivities(
            nodal_nu_t_sensitivities, c_mu, nodal_tke, nodal_epsilon, nodal_f_mu);

        rValues.clear();
        rValues.push_back(nodal_nu_t_sensitivities);
    };

    auto calculate_primal_quantities = [](std::vector<double>& rSensitivities,
                                          const NodeType& rNode,
                                          const ProcessInfo& rCurrentProcessInfo) {
        rSensitivities.push_back(rNode.FastGetSolutionStepValue(TURBULENT_VISCOSITY));
    };

    RansModellingApplicationTestUtilities::RunNodalScalarSensitivityTest(
        r_model_part, y_plus_model_process, calculate_primal_quantities,
        calculate_sensitivities, RansEvmKEpsilonModel::UpdateVariablesInModelPart,
        perturb_variable, 1e-7, 1e-6);
}

KRATOS_TEST_CASE_IN_SUITE(RansEvmKEpsilonNodalVelocitySensitivities, RANSEvModelsKEpsilonNodalMatrices)
{
    Model model;
    ModelPart& r_model_part = model.CreateModelPart("RansSensitivities");
    RansEvmKEpsilonModel::GenerateRansEvmKEpsilonTestModelPart(r_model_part,
                                                               "RANSEVMK2D3N");

    Parameters empty_parameters = Parameters(R"({})");
    RansLogarithmicYPlusModelSensitivitiesProcess y_plus_model_sensitivities_process(
        r_model_part, empty_parameters);
    RansLogarithmicYPlusModelProcess y_plus_model_process(r_model_part, empty_parameters);

    y_plus_model_process.Check();
    y_plus_model_process.Execute();

    y_plus_model_sensitivities_process.Check();
    y_plus_model_sensitivities_process.Execute();

    auto perturb_variable = [](NodeType& rNode, const int Dim) -> double& {
        array_1d<double, 3>& r_velocity = rNode.FastGetSolutionStepValue(VELOCITY);
        return r_velocity[Dim];
    };

    auto calculate_sensitivities = [](std::vector<Matrix>& rValues,
                                      const ElementType& rElement,
                                      const ProcessInfo& rCurrentProcessInfo) {
        RansCalculationUtilities rans_calculation_utilities;

        const double c_mu = rCurrentProcessInfo[TURBULENCE_RANS_C_MU];

        const GeometryType& r_geometry = rElement.GetGeometry();
        const int number_of_nodes = r_geometry.PointsNumber();

        Vector nodal_y_plus(number_of_nodes);
        Vector nodal_tke(number_of_nodes);
        Vector nodal_epsilon(number_of_nodes);
        Vector nodal_nu_t(number_of_nodes);
        Vector nodal_f_mu(number_of_nodes);

        RansEvmKEpsilonModel::ReadNodalDataFromElement(
            nodal_y_plus, nodal_tke, nodal_epsilon, nodal_nu_t, nodal_f_mu, rElement);

        const Matrix& r_y_plus_sensitivities =
            rElement.GetValue(RANS_Y_PLUS_VELOCITY_DERIVATIVES);

        Matrix f_mu_velocity_sensitivities;
        EvmKepsilonModelAdjointUtilities::CalculateNodalFmuVectorSensitivities(
            f_mu_velocity_sensitivities, nodal_y_plus, r_y_plus_sensitivities);

        Matrix nodal_nu_t_sensitivities;
        EvmKepsilonModelAdjointUtilities::CalculateNodalTurbulentViscosityVectorSensitivities(
            nodal_nu_t_sensitivities, c_mu, nodal_tke, nodal_epsilon, f_mu_velocity_sensitivities);

        rValues.clear();
        rValues.push_back(nodal_nu_t_sensitivities);
    };

    auto calculate_primal_quantities = [](std::vector<double>& rSensitivities,
                                          const NodeType& rNode,
                                          const ProcessInfo& rCurrentProcessInfo) {
        rSensitivities.push_back(rNode.FastGetSolutionStepValue(TURBULENT_VISCOSITY));
    };

    RansModellingApplicationTestUtilities::RunNodalVectorSensitivityTest(
        r_model_part, y_plus_model_process, calculate_primal_quantities,
        calculate_sensitivities, RansEvmKEpsilonModel::UpdateVariablesInModelPart,
        perturb_variable, 1e-7, 1e-6);
}
} // namespace Testing
} // namespace Kratos