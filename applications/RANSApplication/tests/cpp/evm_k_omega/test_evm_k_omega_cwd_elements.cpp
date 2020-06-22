//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:
//

// System includes

// External includes

// Project includes
#include "containers/model.h"
#include "testing/testing.h"

// Application includes
#include "../stabilization_method_test_utilities.h"
#include "custom_utilities/test_utilities.h"
#include "evm_k_omega_test_utilities.h"
#include "rans_application_variables.h"

namespace Kratos
{
namespace Testing
{
namespace
{
ModelPart& RansEvmKOmegaKCrossWindStabilized2D3N_SetUp(Model& rModel)
{
    ModelPart& r_model_part = EvmKOmegaTestUtilities::RansEvmKOmegaK2D3N_SetUp(
        rModel, "RansEvmKOmegaKCrossWindStabilized2D3N");

    StabilizationMethodTestUtilities::InitializeCrossWindStabilizedConstants(
        r_model_part.GetProcessInfo());

    return r_model_part;
}

ModelPart& RansEvmKOmegaOmegaCrossWindStabilized2D3N_SetUp(Model& rModel)
{
    ModelPart& r_model_part = EvmKOmegaTestUtilities::RansEvmKOmegaOmega2D3N_SetUp(
        rModel, "RansEvmKOmegaOmegaCrossWindStabilized2D3N");

    StabilizationMethodTestUtilities::InitializeCrossWindStabilizedConstants(
        r_model_part.GetProcessInfo());

    return r_model_part;
}

} // namespace

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaKCrossWindStabilized2D3N_EquationIdVector, KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaKCrossWindStabilized2D3N_SetUp(model);

    // Test:
    RansModellingApplicationTestUtilities::TestEquationIdVector(r_model_part);
}

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaKCrossWindStabilized2D3N_GetDofList, KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaKCrossWindStabilized2D3N_SetUp(model);

    // Test:
    RansModellingApplicationTestUtilities::TestGetDofList(r_model_part, TURBULENT_KINETIC_ENERGY);
}

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaKCrossWindStabilized2D3N_CalculateLocalSystem, KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaKCrossWindStabilized2D3N_SetUp(model);

    // Test:
    Matrix LHS, ref_LHS;
    Vector RHS, ref_RHS(3);
    auto& r_element = r_model_part.Elements().front();
    r_element.CalculateLocalSystem(LHS, RHS, r_model_part.GetProcessInfo());

    // setting reference values
    ref_RHS[0] = 5.37345251236728316258e+00;
    ref_RHS[1] = 4.41760934379480474377e+00;
    ref_RHS[2] = 3.70832202664957133464e+00;
    ref_LHS = ZeroMatrix(3, 3);

    std::cout << std::scientific << std::setprecision(20) << std::endl
              << RHS[0] << std::endl
              << RHS[1] << std::endl
              << RHS[2] << std::endl;

    KRATOS_CHECK_VECTOR_NEAR(RHS, ref_RHS, 1e-12);
    KRATOS_CHECK_MATRIX_NEAR(LHS, ref_LHS, 1e-12);
}

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaKCrossWindStabilized2D3N_CalculateRightHandSide, KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaKCrossWindStabilized2D3N_SetUp(model);

    // Test:
    Vector RHS, ref_RHS(3);
    auto& r_element = r_model_part.Elements().front();
    r_element.CalculateRightHandSide(RHS, r_model_part.GetProcessInfo());

    // setting reference values
    ref_RHS[0] = 5.37345251236728316258e+00;
    ref_RHS[1] = 4.41760934379480474377e+00;
    ref_RHS[2] = 3.70832202664957133464e+00;

    KRATOS_CHECK_VECTOR_NEAR(RHS, ref_RHS, 1e-12);
}

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaKCrossWindStabilized2D3N_CalculateLocalVelocityContribution,
                          KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaKCrossWindStabilized2D3N_SetUp(model);

    // Test:
    Matrix LHS, ref_LHS(3, 3);
    Vector RHS(3, 0.0), ref_RHS(3);
    auto& r_element = r_model_part.Elements().front();
    r_element.CalculateLocalVelocityContribution(LHS, RHS, r_model_part.GetProcessInfo());

    // setting reference values
    ref_LHS(0, 0) = 4.28929951086860546638e+03;
    ref_LHS(0, 1) = -3.88524948008093178942e+03;
    ref_LHS(0, 2) = 1.62018598345824329954e+02;
    ref_LHS(1, 0) = -3.88408677266089125624e+03;
    ref_LHS(1, 1) = 8.46659607792858332687e+03;
    ref_LHS(1, 2) = -3.85622854303346412053e+03;
    ref_LHS(2, 0) = 1.62289207950206389341e+02;
    ref_LHS(2, 1) = -3.85636379951018261636e+03;
    ref_LHS(2, 2) = 4.44109821545958311617e+03;

    ref_RHS[0] = 5.69857230499429279007e+04;
    ref_RHS[1] = -2.70857710055126692168e+05;
    ref_RHS[2] = 1.10122260042898240499e+05;

    std::cout << std::scientific << std::setprecision(20) << std::endl
              << LHS(0, 0) << std::endl
              << LHS(0, 1) << std::endl
              << LHS(0, 2) << std::endl
              << LHS(1, 0) << std::endl
              << LHS(1, 1) << std::endl
              << LHS(1, 2) << std::endl
              << LHS(2, 0) << std::endl
              << LHS(2, 1) << std::endl
              << LHS(2, 2) << std::endl;

    std::cout << std::scientific << std::setprecision(20) << std::endl
              << RHS[0] << std::endl
              << RHS[1] << std::endl
              << RHS[2] << std::endl;

    KRATOS_CHECK_VECTOR_NEAR(RHS, ref_RHS, 1e-12);
    KRATOS_CHECK_MATRIX_NEAR(LHS, ref_LHS, 1e-12);
}

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaKCrossWindStabilized2D3N_CalculateMassMatrix, KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaKCrossWindStabilized2D3N_SetUp(model);

    // Test:
    Matrix M, ref_M(3, 3);
    auto& r_element = r_model_part.Elements().front();
    r_element.CalculateMassMatrix(M, r_model_part.GetProcessInfo());

    // setting reference values
    ref_M(0, 0) = 2.50532337253762005247e-01;
    ref_M(0, 1) = 4.19634783660165489039e-02;
    ref_M(0, 2) = 4.17743588551793137986e-02;
    ref_M(1, 0) = 4.12513970135350641066e-02;
    ref_M(1, 1) = 2.49710814803736025347e-01;
    ref_M(1, 2) = 4.15718439090168173644e-02;
    ref_M(2, 0) = 4.15488302295329986480e-02;
    ref_M(2, 1) = 4.16586587464395780023e-02;
    ref_M(2, 2) = 2.49986887084700876160e-01;

    std::cout << std::scientific << std::setprecision(20) << std::endl
              << M(0, 0) << std::endl
              << M(0, 1) << std::endl
              << M(0, 2) << std::endl
              << M(1, 0) << std::endl
              << M(1, 1) << std::endl
              << M(1, 2) << std::endl
              << M(2, 0) << std::endl
              << M(2, 1) << std::endl
              << M(2, 2) << std::endl;

    KRATOS_CHECK_MATRIX_NEAR(M, ref_M, 1e-12);
}

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaKCrossWindStabilized2D3N_CalculateDampingMatrix, KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaKCrossWindStabilized2D3N_SetUp(model);

    // Test:
    Matrix D, ref_D(3, 3);
    auto& r_element = r_model_part.Elements().front();
    r_element.CalculateDampingMatrix(D, r_model_part.GetProcessInfo());

    // setting reference values
    ref_D(0, 0) = 4.28929951086860546638e+03;
    ref_D(0, 1) = -3.88524948008093178942e+03;
    ref_D(0, 2) = 1.62018598345824329954e+02;
    ref_D(1, 0) = -3.88408677266089125624e+03;
    ref_D(1, 1) = 8.46659607792858332687e+03;
    ref_D(1, 2) = -3.85622854303346412053e+03;
    ref_D(2, 0) = 1.62289207950206389341e+02;
    ref_D(2, 1) = -3.85636379951018261636e+03;
    ref_D(2, 2) = 4.44109821545958311617e+03;

    std::cout << std::scientific << std::setprecision(20) << std::endl
              << D(0, 0) << std::endl
              << D(0, 1) << std::endl
              << D(0, 2) << std::endl
              << D(1, 0) << std::endl
              << D(1, 1) << std::endl
              << D(1, 2) << std::endl
              << D(2, 0) << std::endl
              << D(2, 1) << std::endl
              << D(2, 2) << std::endl;

    KRATOS_CHECK_MATRIX_NEAR(D, ref_D, 1e-12);
}

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaOmegaCrossWindStabilized2D3N_EquationIdVector, KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaOmegaCrossWindStabilized2D3N_SetUp(model);

    // Test:
    RansModellingApplicationTestUtilities::TestEquationIdVector(r_model_part);
}

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaOmegaCrossWindStabilized2D3N_GetDofList, KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaOmegaCrossWindStabilized2D3N_SetUp(model);

    // Test:
    RansModellingApplicationTestUtilities::TestGetDofList(
        r_model_part, TURBULENT_SPECIFIC_ENERGY_DISSIPATION_RATE);
}

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaOmegaCrossWindStabilized2D3N_CalculateLocalSystem,
                          KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaOmegaCrossWindStabilized2D3N_SetUp(model);

    // Test:
    Matrix LHS, ref_LHS;
    Vector RHS, ref_RHS(3);
    auto& r_element = r_model_part.Elements().front();
    r_element.CalculateLocalSystem(LHS, RHS, r_model_part.GetProcessInfo());

    // setting reference values
    ref_RHS[0] = 3.36464166690169292906e+02;
    ref_RHS[1] = 3.35283154556495048837e+02;
    ref_RHS[2] = 3.35732415089861490287e+02;
    ref_LHS = ZeroMatrix(3, 3);

    std::cout << std::scientific << std::setprecision(20) << std::endl
              << RHS[0] << std::endl
              << RHS[1] << std::endl
              << RHS[2] << std::endl;

    KRATOS_CHECK_VECTOR_NEAR(RHS, ref_RHS, 1e-12);
    KRATOS_CHECK_MATRIX_NEAR(LHS, ref_LHS, 1e-12);
}

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaOmegaCrossWindStabilized2D3N_CalculateRightHandSide,
                          KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaOmegaCrossWindStabilized2D3N_SetUp(model);

    // Test:
    Vector RHS, ref_RHS(3);
    auto& r_element = r_model_part.Elements().front();
    r_element.CalculateRightHandSide(RHS, r_model_part.GetProcessInfo());

    // setting reference values
    ref_RHS[0] = 3.36464166690169292906e+02;
    ref_RHS[1] = 3.35283154556495048837e+02;
    ref_RHS[2] = 3.35732415089861490287e+02;

    KRATOS_CHECK_VECTOR_NEAR(RHS, ref_RHS, 1e-12);
}

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaOmegaCrossWindStabilized2D3N_CalculateLocalVelocityContribution,
                          KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaOmegaCrossWindStabilized2D3N_SetUp(model);

    // Test:
    Matrix LHS, ref_LHS(3, 3);
    Vector RHS(3, 0.0), ref_RHS(3);
    auto& r_element = r_model_part.Elements().front();
    r_element.CalculateLocalVelocityContribution(LHS, RHS, r_model_part.GetProcessInfo());

    // setting reference values
    ref_LHS(0, 0) = 2.30095780860482227581e+03;
    ref_LHS(0, 1) = -1.70317607535540832941e+03;
    ref_LHS(0, 2) = 2.39627795739611713088e+02;
    ref_LHS(1, 0) = -1.70201336793536734149e+03;
    ref_LHS(1, 1) = 4.43578501238956596353e+03;
    ref_LHS(1, 2) = -1.65855819660940028371e+03;
    ref_LHS(2, 0) = 2.39898405343993772476e+02;
    ref_LHS(2, 1) = -1.65869345308611877954e+03;
    ref_LHS(2, 2) = 2.52407503555137736839e+03;

    ref_RHS[0] = 4.59038232288823754061e+05;
    ref_RHS[1] = -2.75459848789511527866e+06;
    ref_RHS[2] = 1.07982710128029272892e+06;

    std::cout << std::scientific << std::setprecision(20) << std::endl
              << LHS(0, 0) << std::endl
              << LHS(0, 1) << std::endl
              << LHS(0, 2) << std::endl
              << LHS(1, 0) << std::endl
              << LHS(1, 1) << std::endl
              << LHS(1, 2) << std::endl
              << LHS(2, 0) << std::endl
              << LHS(2, 1) << std::endl
              << LHS(2, 2) << std::endl;

    std::cout << std::scientific << std::setprecision(20) << std::endl
              << RHS[0] << std::endl
              << RHS[1] << std::endl
              << RHS[2] << std::endl;

    KRATOS_CHECK_VECTOR_NEAR(RHS, ref_RHS, 1e-12);
    KRATOS_CHECK_MATRIX_NEAR(LHS, ref_LHS, 1e-12);
}

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaOmegaCrossWindStabilized2D3N_CalculateMassMatrix,
                          KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaOmegaCrossWindStabilized2D3N_SetUp(model);

    // Test:
    Matrix M, ref_M(3, 3);
    auto& r_element = r_model_part.Elements().front();
    r_element.CalculateMassMatrix(M, r_model_part.GetProcessInfo());

    // setting reference values
    ref_M(0, 0) = 2.50359419104817682022e-01;
    ref_M(0, 1) = 4.18672012531570852945e-02;
    ref_M(0, 2) = 4.17393549640153460611e-02;
    ref_M(1, 0) = 4.13863888672168847571e-02;
    ref_M(1, 1) = 2.49804656599209751189e-01;
    ref_M(1, 2) = 4.16026865436841114532e-02;
    ref_M(2, 0) = 4.15871748163466822823e-02;
    ref_M(2, 1) = 4.16613013002525983675e-02;
    ref_M(2, 2) = 2.49991180740948670813e-01;

    std::cout << std::scientific << std::setprecision(20) << std::endl
              << M(0, 0) << std::endl
              << M(0, 1) << std::endl
              << M(0, 2) << std::endl
              << M(1, 0) << std::endl
              << M(1, 1) << std::endl
              << M(1, 2) << std::endl
              << M(2, 0) << std::endl
              << M(2, 1) << std::endl
              << M(2, 2) << std::endl;

    KRATOS_CHECK_MATRIX_NEAR(M, ref_M, 1e-12);
}

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaOmegaCrossWindStabilized2D3N_CalculateDampingMatrix,
                          KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaOmegaCrossWindStabilized2D3N_SetUp(model);

    // Test:
    Matrix D, ref_D(3, 3);
    auto& r_element = r_model_part.Elements().front();
    r_element.CalculateDampingMatrix(D, r_model_part.GetProcessInfo());

    // setting reference values
    ref_D(0, 0) = 2.30095780860482227581e+03;
    ref_D(0, 1) = -1.70317607535540832941e+03;
    ref_D(0, 2) = 2.39627795739611713088e+02;
    ref_D(1, 0) = -1.70201336793536734149e+03;
    ref_D(1, 1) = 4.43578501238956596353e+03;
    ref_D(1, 2) = -1.65855819660940028371e+03;
    ref_D(2, 0) = 2.39898405343993772476e+02;
    ref_D(2, 1) = -1.65869345308611877954e+03;
    ref_D(2, 2) = 2.52407503555137736839e+03;

    std::cout << std::scientific << std::setprecision(20) << std::endl
              << D(0, 0) << std::endl
              << D(0, 1) << std::endl
              << D(0, 2) << std::endl
              << D(1, 0) << std::endl
              << D(1, 1) << std::endl
              << D(1, 2) << std::endl
              << D(2, 0) << std::endl
              << D(2, 1) << std::endl
              << D(2, 2) << std::endl;

    KRATOS_CHECK_MATRIX_NEAR(D, ref_D, 1e-12);
}

} // namespace Testing
} // namespace Kratos.
