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
#include "rans_application_variables.h"
#include "test_utilities.h"

namespace Kratos
{
namespace Testing
{
namespace
{
ModelPart& RansEvmKOmegaKRFC2D3N_SetUp(Model& rModel)
{
    ModelPart& r_model_part = EvmKOmegaTestUtilities::RansEvmKOmegaK2D3N_SetUp(
        rModel, "RansEvmKOmegaKRFC2D3N");

    StabilizationMethodTestUtilities::InitializeResidualBasedFluxCorrectedConstants(
        r_model_part.GetProcessInfo());

    return r_model_part;
}

ModelPart& RansEvmKOmegaOmegaRFC2D3N_SetUp(Model& rModel)
{
    ModelPart& r_model_part = EvmKOmegaTestUtilities::RansEvmKOmegaOmega2D3N_SetUp(
        rModel, "RansEvmKOmegaOmegaRFC2D3N");

    StabilizationMethodTestUtilities::InitializeResidualBasedFluxCorrectedConstants(
        r_model_part.GetProcessInfo());

    return r_model_part;
}

} // namespace

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaKRFC2D3N_EquationIdVector, KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaKRFC2D3N_SetUp(model);

    // Test:
    RansApplicationTestUtilities::TestEquationIdVector<ModelPart::ElementsContainerType>(r_model_part);
}

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaKRFC2D3N_GetDofList, KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaKRFC2D3N_SetUp(model);

    // Test:
    RansApplicationTestUtilities::TestGetDofList<ModelPart::ElementsContainerType>(r_model_part, TURBULENT_KINETIC_ENERGY);
}

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaKRFC2D3N_CalculateLocalSystem, KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaKRFC2D3N_SetUp(model);

    // Test:
    Matrix LHS, ref_LHS;
    Vector RHS, ref_RHS(3);
    auto& r_element = r_model_part.Elements().front();
    r_element.CalculateLocalSystem(LHS, RHS, r_model_part.GetProcessInfo());

    // setting reference values
    ref_RHS[0] = 4.50994278129411618750e+00;
    ref_RHS[1] = 4.49055327416187921585e+00;
    ref_RHS[2] = 4.49880670925329351206e+00;
    ref_LHS = ZeroMatrix(3, 3);

    std::cout << std::scientific << std::setprecision(20) << std::endl
              << RHS[0] << std::endl
              << RHS[1] << std::endl
              << RHS[2] << std::endl;

    KRATOS_CHECK_VECTOR_NEAR(RHS, ref_RHS, 1e-12);
    KRATOS_CHECK_MATRIX_NEAR(LHS, ref_LHS, 1e-12);
}

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaKRFC2D3N_CalculateRightHandSide, KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaKRFC2D3N_SetUp(model);

    // Test:
    Vector RHS, ref_RHS(3);
    auto& r_element = r_model_part.Elements().front();
    r_element.CalculateRightHandSide(RHS, r_model_part.GetProcessInfo());

    // setting reference values
    ref_RHS[0] = 4.50994278129411618750e+00;
    ref_RHS[1] = 4.49055327416187921585e+00;
    ref_RHS[2] = 4.49880670925329351206e+00;

    KRATOS_CHECK_VECTOR_NEAR(RHS, ref_RHS, 1e-12);
}

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaKRFC2D3N_CalculateLocalVelocityContribution, KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaKRFC2D3N_SetUp(model);

    // Test:
    Matrix LHS, ref_LHS(3, 3);
    Vector RHS(3, 0.0), ref_RHS(3);
    auto& r_element = r_model_part.Elements().front();
    r_element.CalculateLocalVelocityContribution(LHS, RHS, r_model_part.GetProcessInfo());

    // setting reference values

    ref_LHS(0, 0) = 9.75441058563366027556e+02;
    ref_LHS(0, 1) = -1.70114027289328078041e+02;
    ref_LHS(0, 2) = -1.70048962261448053823e+02;
    ref_LHS(1, 0) = -1.69203600315600766635e+02;
    ref_LHS(1, 1) = 9.70120245209386212082e+02;
    ref_LHS(1, 2) = -1.68369814224432104766e+02;
    ref_LHS(2, 0) = -1.69526072210752602132e+02;
    ref_LHS(2, 1) = -1.68757351147463964480e+02;
    ref_LHS(2, 2) = 9.71992846672722976109e+02;

    ref_RHS[0] = -2.98304856313582386065e+04;
    ref_RHS[1] = -5.37935024254663076135e+04;
    ref_RHS[2] = -1.30857706730086247262e+04;

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

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaKRFC2D3N_CalculateMassMatrix, KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaKRFC2D3N_SetUp(model);

    // Test:
    Matrix M, ref_M(3, 3);
    auto& r_element = r_model_part.Elements().front();
    r_element.CalculateMassMatrix(M, r_model_part.GetProcessInfo());

    // setting reference values
    ref_M(0, 0) = 2.22472606519330234898e-01;
    ref_M(0, 1) = 5.58059398526635566662e-02;
    ref_M(0, 2) = 5.58059398526635566662e-02;
    ref_M(1, 0) = 5.53271655945000231935e-02;
    ref_M(1, 1) = 2.21993832261166645914e-01;
    ref_M(1, 2) = 5.53271655945000023769e-02;
    ref_M(2, 0) = 5.55309630523850156214e-02;
    ref_M(2, 1) = 5.55309630523850017436e-02;
    ref_M(2, 2) = 2.22197629719051659158e-01;

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

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaKRFC2D3N_CalculateDampingMatrix, KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaKRFC2D3N_SetUp(model);

    // Test:
    Matrix D, ref_D(3, 3);
    auto& r_element = r_model_part.Elements().front();
    r_element.CalculateDampingMatrix(D, r_model_part.GetProcessInfo());

    // setting reference values
    ref_D(0, 0) = 9.75441058563366027556e+02;
    ref_D(0, 1) = -1.70114027289328078041e+02;
    ref_D(0, 2) = -1.70048962261448053823e+02;
    ref_D(1, 0) = -1.69203600315600766635e+02;
    ref_D(1, 1) = 9.70120245209386212082e+02;
    ref_D(1, 2) = -1.68369814224432104766e+02;
    ref_D(2, 0) = -1.69526072210752602132e+02;
    ref_D(2, 1) = -1.68757351147463964480e+02;
    ref_D(2, 2) = 9.71992846672722976109e+02;

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

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaOmegaRFC2D3N_EquationIdVector, KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaOmegaRFC2D3N_SetUp(model);

    // Test:
    RansApplicationTestUtilities::TestEquationIdVector<ModelPart::ElementsContainerType>(r_model_part);
}

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaOmegaRFC2D3N_GetDofList, KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaOmegaRFC2D3N_SetUp(model);

    // Test:
    RansApplicationTestUtilities::TestGetDofList<ModelPart::ElementsContainerType>(
        r_model_part, TURBULENT_SPECIFIC_ENERGY_DISSIPATION_RATE);
}

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaOmegaRFC2D3N_CalculateLocalSystem, KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaOmegaRFC2D3N_SetUp(model);

    // Test:
    Matrix LHS, ref_LHS;
    Vector RHS, ref_RHS(3);
    auto& r_element = r_model_part.Elements().front();
    r_element.CalculateLocalSystem(LHS, RHS, r_model_part.GetProcessInfo());

    // setting reference values
    ref_RHS[0] = 3.36338662939943844776e+02;
    ref_RHS[1] = 3.35360960309170650362e+02;
    ref_RHS[2] = 3.35777134126625469435e+02;
    ref_LHS = ZeroMatrix(3, 3);

    std::cout << std::scientific << std::setprecision(20) << std::endl
              << RHS[0] << std::endl
              << RHS[1] << std::endl
              << RHS[2] << std::endl;

    KRATOS_CHECK_VECTOR_NEAR(RHS, ref_RHS, 1e-12);
    KRATOS_CHECK_MATRIX_NEAR(LHS, ref_LHS, 1e-12);
}

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaOmegaRFC2D3N_CalculateRightHandSide, KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaOmegaRFC2D3N_SetUp(model);

    // Test:
    Vector RHS, ref_RHS(3);
    auto& r_element = r_model_part.Elements().front();
    r_element.CalculateRightHandSide(RHS, r_model_part.GetProcessInfo());

    // setting reference values
    ref_RHS[0] = 3.36338662939943844776e+02;
    ref_RHS[1] = 3.35360960309170650362e+02;
    ref_RHS[2] = 3.35777134126625469435e+02;

    KRATOS_CHECK_VECTOR_NEAR(RHS, ref_RHS, 1e-12);
}

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaOmegaRFC2D3N_CalculateLocalVelocityContribution, KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaOmegaRFC2D3N_SetUp(model);

    // Test:
    Matrix LHS, ref_LHS(3, 3);
    Vector RHS(3, 0.0), ref_RHS(3);
    auto& r_element = r_model_part.Elements().front();
    r_element.CalculateLocalVelocityContribution(LHS, RHS, r_model_part.GetProcessInfo());

    // setting reference values
    ref_LHS(0, 0) = 1.44051172690053545011e+03;
    ref_LHS(0, 1) = -2.50492855775676048324e+02;
    ref_LHS(0, 2) = -2.50439960775146118976e+02;
    ref_LHS(1, 0) = -2.49582428801948708497e+02;
    ref_LHS(1, 1) = 1.43518031739059460961e+03;
    ref_LHS(1, 2) = -2.48750239535026594240e+02;
    ref_LHS(2, 0) = -2.49917070724450582020e+02;
    ref_LHS(2, 1) = -2.49137776458058397111e+02;
    ref_LHS(2, 2) = 1.43706509865151019767e+03;

    ref_RHS[0] = -3.31910833529019204434e+05;
    ref_RHS[1] = -1.01944568759137822781e+06;
    ref_RHS[2] = 2.14518576254149142187e+05;

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

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaOmegaRFC2D3N_CalculateMassMatrix, KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaOmegaRFC2D3N_SetUp(model);

    // Test:
    Matrix M, ref_M(3, 3);
    auto& r_element = r_model_part.Elements().front();
    r_element.CalculateMassMatrix(M, r_model_part.GetProcessInfo());

    // setting reference values
    ref_M(0, 0) = 2.22391578916634025109e-01;
    ref_M(0, 1) = 5.57249122499673399389e-02;
    ref_M(0, 2) = 5.57249122499673399389e-02;
    ref_M(1, 0) = 5.54014311337135689772e-02;
    ref_M(1, 1) = 2.22068097800380198636e-01;
    ref_M(1, 2) = 5.54014311337135481605e-02;
    ref_M(2, 0) = 5.55391257320388548324e-02;
    ref_M(2, 1) = 5.55391257320388409546e-02;
    ref_M(2, 2) = 2.22205792398705498369e-01;

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

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaOmegaRFC2D3N_CalculateDampingMatrix, KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaOmegaRFC2D3N_SetUp(model);

    // Test:
    Matrix D, ref_D(3, 3);
    auto& r_element = r_model_part.Elements().front();
    r_element.CalculateDampingMatrix(D, r_model_part.GetProcessInfo());

    // setting reference values
    ref_D(0, 0) = 1.44051172690053545011e+03;
    ref_D(0, 1) = -2.50492855775676048324e+02;
    ref_D(0, 2) = -2.50439960775146118976e+02;
    ref_D(1, 0) = -2.49582428801948708497e+02;
    ref_D(1, 1) = 1.43518031739059460961e+03;
    ref_D(1, 2) = -2.48750239535026594240e+02;
    ref_D(2, 0) = -2.49917070724450582020e+02;
    ref_D(2, 1) = -2.49137776458058397111e+02;
    ref_D(2, 2) = 1.43706509865151019767e+03;

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
