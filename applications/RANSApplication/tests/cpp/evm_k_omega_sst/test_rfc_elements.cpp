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
ModelPart& RansEvmKOmegaSSTKRFC2D3N_SetUp(Model& rModel)
{
    ModelPart& r_model_part = EvmKOmegaSSTTestUtilities::RansEvmKOmegaSSTK2D3N_SetUp(
        rModel, "RansEvmKOmegaSSTKRFC2D3N");

    StabilizationMethodTestUtilities::InitializeResidualBasedFluxCorrectedConstants(
        r_model_part.GetProcessInfo());

    return r_model_part;
}

ModelPart& RansEvmKOmegaSSTOmegaRFC2D3N_SetUp(Model& rModel)
{
    ModelPart& r_model_part = EvmKOmegaSSTTestUtilities::RansEvmKOmegaSSTOmega2D3N_SetUp(
        rModel, "RansEvmKOmegaSSTOmegaRFC2D3N");

    StabilizationMethodTestUtilities::InitializeResidualBasedFluxCorrectedConstants(
        r_model_part.GetProcessInfo());

    return r_model_part;
}

} // namespace

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaSSTKRFC2D3N_EquationIdVector, KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaSSTKRFC2D3N_SetUp(model);

    // Test:
    RansApplicationTestUtilities::TestEquationIdVector<ModelPart::ElementsContainerType>(r_model_part);
}

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaSSTKRFC2D3N_GetDofList, KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaSSTKRFC2D3N_SetUp(model);

    // Test:
    RansApplicationTestUtilities::TestGetDofList<ModelPart::ElementsContainerType>(r_model_part, TURBULENT_KINETIC_ENERGY);
}

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaSSTKRFC2D3N_CalculateLocalSystem, KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaSSTKRFC2D3N_SetUp(model);

    // Test:
    Matrix LHS, ref_LHS(3, 3);
    Vector RHS, ref_RHS(3);
    auto& r_element = r_model_part.Elements().front();
    r_element.CalculateLocalSystem(LHS, RHS, r_model_part.GetProcessInfo());

    // setting reference values
    ref_RHS[0] = 4.50994283894762126863e+00;
    ref_RHS[1] = 4.49055333132075862324e+00;
    ref_RHS[2] = 4.49880676662271739019e+00;
    ref_LHS = ZeroMatrix(3, 3);

    std::cout << std::scientific << std::setprecision(20) << std::endl
              << RHS[0] << std::endl
              << RHS[1] << std::endl
              << RHS[2] << std::endl;

    KRATOS_CHECK_VECTOR_NEAR(RHS, ref_RHS, 1e-12);
    KRATOS_CHECK_MATRIX_NEAR(LHS, ref_LHS, 1e-12);
}

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaSSTKRFC2D3N_CalculateRightHandSide, KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaSSTKRFC2D3N_SetUp(model);

    // Test:
    Vector RHS, ref_RHS(3);
    auto& r_element = r_model_part.Elements().front();
    r_element.CalculateRightHandSide(RHS, r_model_part.GetProcessInfo());

    // setting reference values
    ref_RHS[0] = 4.50994283894762126863e+00;
    ref_RHS[1] = 4.49055333132075862324e+00;
    ref_RHS[2] = 4.49880676662271739019e+00;

    KRATOS_CHECK_VECTOR_NEAR(RHS, ref_RHS, 1e-12);
}

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaSSTKRFC2D3N_CalculateLocalVelocityContribution, KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaSSTKRFC2D3N_SetUp(model);

    // Test:
    Matrix LHS, ref_LHS(3, 3);
    Vector RHS(3, 0.0), ref_RHS(3);
    auto& r_element = r_model_part.Elements().front();
    r_element.CalculateLocalVelocityContribution(LHS, RHS, r_model_part.GetProcessInfo());

    // setting reference values
    ref_LHS(0, 0) = 9.75445586550368261669e+02;
    ref_LHS(0, 1) = -1.70118535255743495327e+02;
    ref_LHS(0, 2) = -1.70048974160866123384e+02;
    ref_LHS(1, 0) = -1.69208108282016212343e+02;
    ref_LHS(1, 1) = 9.70129269135206186547e+02;
    ref_LHS(1, 2) = -1.68374322132341745828e+02;
    ref_LHS(2, 0) = -1.69526084110170643271e+02;
    ref_LHS(2, 1) = -1.68761859055373577121e+02;
    ref_LHS(2, 2) = 9.71997374561203514531e+02;

    ref_RHS[0] = -2.98303908390452597814e+04;
    ref_RHS[1] = -5.37937596174453428830e+04;
    ref_RHS[2] = -1.30856095068626382272e+04;

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

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaSSTKRFC2D3N_CalculateMassMatrix, KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaSSTKRFC2D3N_SetUp(model);

    // Test:
    Matrix M, ref_M(3, 3);
    auto& r_element = r_model_part.Elements().front();
    r_element.CalculateMassMatrix(M, r_model_part.GetProcessInfo());

    // setting reference values
    ref_M(0, 0) = 2.22472607942935968195e-01;
    ref_M(0, 1) = 5.58059412762692969023e-02;
    ref_M(0, 2) = 5.58059412762692969023e-02;
    ref_M(1, 0) = 5.53271670058922693247e-02;
    ref_M(1, 1) = 2.21993833672558898984e-01;
    ref_M(1, 2) = 5.53271670058922485080e-02;
    ref_M(2, 0) = 5.55309644689761239267e-02;
    ref_M(2, 1) = 5.55309644689761031100e-02;
    ref_M(2, 2) = 2.22197631135642753586e-01;

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

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaSSTKRFC2D3N_CalculateDampingMatrix, KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaSSTKRFC2D3N_SetUp(model);

    // Test:
    Matrix D, ref_D(3, 3);
    auto& r_element = r_model_part.Elements().front();
    r_element.CalculateDampingMatrix(D, r_model_part.GetProcessInfo());

    // setting reference values
    ref_D(0, 0) = 9.75445586550368261669e+02;
    ref_D(0, 1) = -1.70118535255743495327e+02;
    ref_D(0, 2) = -1.70048974160866123384e+02;
    ref_D(1, 0) = -1.69208108282016212343e+02;
    ref_D(1, 1) = 9.70129269135206186547e+02;
    ref_D(1, 2) = -1.68374322132341745828e+02;
    ref_D(2, 0) = -1.69526084110170643271e+02;
    ref_D(2, 1) = -1.68761859055373577121e+02;
    ref_D(2, 2) = 9.71997374561203514531e+02;

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

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaSSTOmegaRFC2D3N_EquationIdVector, KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaSSTOmegaRFC2D3N_SetUp(model);

    // Test:
    RansApplicationTestUtilities::TestEquationIdVector<ModelPart::ElementsContainerType>(r_model_part);
}

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaSSTOmegaRFC2D3N_GetDofList, KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaSSTOmegaRFC2D3N_SetUp(model);

    // Test:
    RansApplicationTestUtilities::TestGetDofList<ModelPart::ElementsContainerType>(
        r_model_part, TURBULENT_SPECIFIC_ENERGY_DISSIPATION_RATE);
}

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaSSTOmegaRFC2D3N_CalculateLocalSystem, KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaSSTOmegaRFC2D3N_SetUp(model);

    // Test:
    Matrix LHS, ref_LHS(3, 3);
    Vector RHS, ref_RHS(3);
    auto& r_element = r_model_part.Elements().front();
    r_element.CalculateLocalSystem(LHS, RHS, r_model_part.GetProcessInfo());

    // setting reference values
    ref_RHS[0] = -6.35771290260074056278e+03;
    ref_RHS[1] = -6.32265733820180412295e+03;
    ref_RHS[2] = -6.33757926599430902570e+03;

    ref_LHS = ZeroMatrix(3, 3);

    std::cout << std::scientific << std::setprecision(20) << std::endl
              << RHS[0] << std::endl
              << RHS[1] << std::endl
              << RHS[2] << std::endl;

    KRATOS_CHECK_VECTOR_NEAR(RHS, ref_RHS, 1e-12);
    KRATOS_CHECK_MATRIX_NEAR(LHS, ref_LHS, 1e-12);
}

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaSSTOmegaRFC2D3N_CalculateRightHandSide, KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaSSTOmegaRFC2D3N_SetUp(model);

    // Test:
    Vector RHS, ref_RHS(3);
    auto& r_element = r_model_part.Elements().front();
    r_element.CalculateRightHandSide(RHS, r_model_part.GetProcessInfo());

    // setting reference values
    ref_RHS[0] = -6.35771290260074056278e+03;
    ref_RHS[1] = -6.32265733820180412295e+03;
    ref_RHS[2] = -6.33757926599430902570e+03;

    KRATOS_CHECK_VECTOR_NEAR(RHS, ref_RHS, 1e-12);
}

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaSSTOmegaRFC2D3N_CalculateLocalVelocityContribution,
                          KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaSSTOmegaRFC2D3N_SetUp(model);

    // Test:
    Matrix LHS, ref_LHS(3, 3);
    Vector RHS(3, 0.0), ref_RHS(3);
    auto& r_element = r_model_part.Elements().front();
    r_element.CalculateLocalVelocityContribution(LHS, RHS, r_model_part.GetProcessInfo());

    // setting reference values
    ref_LHS(0, 0) = 7.79197663778924720646e+02;
    ref_LHS(0, 1) = -1.41933450678176399151e+02;
    ref_LHS(0, 2) = -1.41929569948109701727e+02;
    ref_LHS(1, 0) = -1.41023023704449059323e+02;
    ref_LHS(1, 1) = 7.73758542795050971108e+02;
    ref_LHS(1, 2) = -1.40132083802816481466e+02;
    ref_LHS(2, 0) = -1.41406679897414221614e+02;
    ref_LHS(2, 1) = -1.40519620725848341181e+02;
    ref_LHS(2, 2) = 7.75692315582619244196e+02;

    ref_RHS[0] = -1.74147484202450868906e+05;
    ref_RHS[1] = -5.46876620395242935047e+05;
    ref_RHS[2] = 1.22979071376479783794e+05;

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

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaSSTOmegaRFC2D3N_CalculateMassMatrix, KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaSSTOmegaRFC2D3N_SetUp(model);

    // Test:
    Matrix M, ref_M(3, 3);
    auto& r_element = r_model_part.Elements().front();
    r_element.CalculateMassMatrix(M, r_model_part.GetProcessInfo());

    // setting reference values
    ref_M(0, 0) = 2.22543171163027159176e-01;
    ref_M(0, 1) = 5.58765044963604809447e-02;
    ref_M(0, 2) = 5.58765044963604809447e-02;
    ref_M(1, 0) = 5.52620832701899850070e-02;
    ref_M(1, 1) = 2.21928749936856628544e-01;
    ref_M(1, 2) = 5.52620832701899711292e-02;
    ref_M(2, 0) = 5.55236208984752482865e-02;
    ref_M(2, 1) = 5.55236208984752344087e-02;
    ref_M(2, 2) = 2.22190287565141891823e-01;

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

KRATOS_TEST_CASE_IN_SUITE(RansEvmKOmegaSSTOmegaRFC2D3N_CalculateDampingMatrix, KratosRansFastSuite)
{
    // Setup:
    Model model;
    auto& r_model_part = RansEvmKOmegaSSTOmegaRFC2D3N_SetUp(model);

    // Test:
    Matrix D, ref_D(3, 3);
    auto& r_element = r_model_part.Elements().front();
    r_element.CalculateDampingMatrix(D, r_model_part.GetProcessInfo());

    // setting reference values
    ref_D(0, 0) = 7.79197663778924720646e+02;
    ref_D(0, 1) = -1.41933450678176399151e+02;
    ref_D(0, 2) = -1.41929569948109701727e+02;
    ref_D(1, 0) = -1.41023023704449059323e+02;
    ref_D(1, 1) = 7.73758542795050971108e+02;
    ref_D(1, 2) = -1.40132083802816481466e+02;
    ref_D(2, 0) = -1.41406679897414221614e+02;
    ref_D(2, 1) = -1.40519620725848341181e+02;
    ref_D(2, 2) = 7.75692315582619244196e+02;

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
