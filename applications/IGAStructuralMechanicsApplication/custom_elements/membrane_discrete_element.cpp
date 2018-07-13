//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:         BSD License
//                     Kratos default license: kratos/IGAStructuralMechanicsApplication/license.txt
//
//  Main authors:    Tobias Teschemacher
//                   Riccardo Rossi
//


// System includes
#include "includes/define.h"
#include "utilities/math_utils.h"
#include "geometries/geometry.h"

// External includes


// Project includes
#include "custom_elements/membrane_discrete_element.h"

// Application includes
#include "iga_structural_mechanics_application.h"
#include "iga_structural_mechanics_application_variables.h"

namespace Kratos
{
    //************************************************************************************
    //************************************************************************************
    void MembraneDiscreteElement::EquationIdVector(
        EquationIdVectorType& rResult,
        ProcessInfo& rCurrentProcessInfo
    )
    {
        KRATOS_TRY;
        unsigned int number_of_nodes = GetGeometry().size();
        unsigned int dim = number_of_nodes * 3;

        if (rResult.size() != dim)
            rResult.resize(dim);

        for (unsigned int i = 0; i < number_of_nodes; i++)
        {
            int index = i * 3;
            rResult[index]     = GetGeometry()[i].GetDof(DISPLACEMENT_X).EquationId();
            rResult[index + 1] = GetGeometry()[i].GetDof(DISPLACEMENT_Y).EquationId();
            rResult[index + 2] = GetGeometry()[i].GetDof(DISPLACEMENT_Z).EquationId();
        }
        KRATOS_CATCH("")
    };

    //************************************************************************************
    //************************************************************************************
    void MembraneDiscreteElement::GetDofList(
        DofsVectorType& rElementalDofList,
        ProcessInfo& rCurrentProcessInfo
    )
    {
        KRATOS_TRY;

        rElementalDofList.resize(0);

        for (unsigned int i = 0; i < GetGeometry().size(); i++)
        {
            rElementalDofList.push_back(GetGeometry()[i].pGetDof(DISPLACEMENT_X));
            rElementalDofList.push_back(GetGeometry()[i].pGetDof(DISPLACEMENT_Y));
            rElementalDofList.push_back(GetGeometry()[i].pGetDof(DISPLACEMENT_Z));
        }

        KRATOS_CATCH("")
    };
    //***********************************************************************************
    //***********************************************************************************
    void MembraneDiscreteElement::FinalizeSolutionStep(
        ProcessInfo& rCurrentProcessInfo)
    {
        mConstitutiveLaw->FinalizeSolutionStep(GetProperties(),
            GetGeometry(),
            this->GetValue(SHAPE_FUNCTION_VALUES),
            rCurrentProcessInfo);
    }
    //************************************************************************************
    //************************************************************************************
    void MembraneDiscreteElement::InitializeMaterial()
    {
        KRATOS_TRY

            if (GetProperties()[CONSTITUTIVE_LAW] != NULL)
            {
                //get shape functions for evaluation of stresses inside the constitutive law
                const Vector&  N = this->GetValue(SHAPE_FUNCTION_VALUES);
                //const Matrix& DN_De = this->GetValue(SHAPE_FUNCTION_LOCAL_DERIVATIVES);
                //const double integration_weight = this->GetValue(INTEGRATION_WEIGHT);

                mConstitutiveLaw = GetProperties()[CONSTITUTIVE_LAW]->Clone();
                ProcessInfo emptyProcessInfo = ProcessInfo();
                mConstitutiveLaw->InitializeMaterial(GetProperties(), GetGeometry(), N);
            }
            else
            {
                KRATOS_ERROR << "A constitutive law needs to be specified for the element with ID " << this->Id() << std::endl;
            }

        KRATOS_CATCH("");
    }
    //************************************************************************************
    //************************************************************************************
    void MembraneDiscreteElement::CalculateAll(
        MatrixType& rLeftHandSideMatrix,
        VectorType& rRightHandSideVector,
        ProcessInfo& rCurrentProcessInfo,
        const bool CalculateStiffnessMatrixFlag,
        const bool CalculateResidualVectorFlag
    )
    {
        KRATOS_TRY
        // definition of problem size
        const int number_of_control_points = GetGeometry().size();
        const int mat_size = number_of_control_points * 3;

        //set up properties for Constitutive Law
        ConstitutiveLaw::Parameters Values(GetGeometry(), GetProperties(), rCurrentProcessInfo);
        Values.GetOptions().Set(ConstitutiveLaw::USE_ELEMENT_PROVIDED_STRAIN, true);
        Values.GetOptions().Set(ConstitutiveLaw::COMPUTE_STRESS);
        Values.GetOptions().Set(ConstitutiveLaw::COMPUTE_CONSTITUTIVE_TENSOR);

        //resizing as needed the LHS
        if (CalculateStiffnessMatrixFlag == true) //calculation of the matrix is required
        {
            if (rLeftHandSideMatrix.size1() != mat_size)
                rLeftHandSideMatrix.resize(mat_size, mat_size);
            noalias(rLeftHandSideMatrix) = ZeroMatrix(mat_size, mat_size); //resetting LHS
        }
        //resizing as needed the RHS
        if (CalculateResidualVectorFlag == true) //calculation of the matrix is required
        {
            if (rRightHandSideVector.size() != mat_size)
                rRightHandSideVector.resize(mat_size);
            rRightHandSideVector = ZeroVector(mat_size); //resetting RHS
        }

        //reading in of integration weight, shape function values and shape function derivatives
        const double integration_weight = this->GetValue(INTEGRATION_WEIGHT);
        const Vector& N = this->GetValue(SHAPE_FUNCTION_VALUES);
        const Matrix& DN_De = this->GetValue(SHAPE_FUNCTION_LOCAL_DERIVATIVES);

        MetricVariables actual_metric(3);
        CalculateMetric(actual_metric);
        ConstitutiveVariables constitutive_variables(3);
        CalculateConstitutiveVariables(actual_metric, constitutive_variables, Values, ConstitutiveLaw::StressMeasure_PK2);

        // calculate B MATRICES
        Matrix BMembrane = ZeroMatrix(3, mat_size);
        CalculateBMembrane(BMembrane, actual_metric);

        // Nonlinear Deformation
        SecondVariations SecondVariationsStrain = SecondVariations(mat_size);
        CalculateSecondVariationStrainMembrane(SecondVariationsStrain, actual_metric);

        double weighting = integration_weight * mInitialMetric.detJ * GetProperties()[THICKNESS];

        // LEFT HAND SIDE MATRIX
        if (CalculateStiffnessMatrixFlag == true)
        {
            //adding membrane contributions to the stiffness matrix
            CalculateAndAddKm(rLeftHandSideMatrix, 
                BMembrane, 
                constitutive_variables.DMembrane, 
                weighting);

            //adding non-linear-contribution to Stiffness-Matrix
            CalculateAndAddNonlinearKm(rLeftHandSideMatrix,
                SecondVariationsStrain,
                constitutive_variables.StressVector,
                weighting);
        }

        // RIGHT HAND SIDE VECTOR
        if (CalculateResidualVectorFlag == true) //calculation of the matrix is required
        {
            // operation performed: rRightHandSideVector -= Weight*IntForce
            noalias(rRightHandSideVector) -= weighting * prod(trans(BMembrane), constitutive_variables.StressVector);
        }
        KRATOS_CATCH("");
    }

    //************************************************************************************
    //************************************************************************************
    void MembraneDiscreteElement::Calculate(
        const Variable<double>& rVariable,
        double& rOutput,
        const ProcessInfo& rCurrentProcessInfo
    )
    {
        if (rVariable == VON_MISES_STRESS)
        {
            // Create constitutive law parameters:
            ConstitutiveLaw::Parameters Values(GetGeometry(), GetProperties(), rCurrentProcessInfo);
            // Set constitutive law flags:
            Flags& ConstitutiveLawOptions = Values.GetOptions();
            ConstitutiveLawOptions.Set(ConstitutiveLaw::USE_ELEMENT_PROVIDED_STRAIN, false);
            ConstitutiveLawOptions.Set(ConstitutiveLaw::COMPUTE_STRESS, true);
            ConstitutiveLawOptions.Set(ConstitutiveLaw::COMPUTE_CONSTITUTIVE_TENSOR, true);

            MetricVariables actual_metric(3);
            CalculateMetric(actual_metric);
            ConstitutiveVariables constitutive_variables(3);
            CalculateConstitutiveVariables(actual_metric, constitutive_variables, Values, ConstitutiveLaw::StressMeasure_PK2);

            double detF = actual_metric.dA / mInitialMetric.dA;

            Vector n_pk2_ca = prod(constitutive_variables.DMembrane, constitutive_variables.StrainVector);
            Vector n_pk2_con = prod(mInitialMetric.T, n_pk2_ca);
            Vector n_cau = 1.0 / detF*n_pk2_con;

            Vector n = ZeroVector(8);
            // Cauchy normal force in normalized g1,g2
            n[0] = sqrt(actual_metric.gab[0] / actual_metric.gab_con[0])*n_cau[0];
            n[1] = sqrt(actual_metric.gab[1] / actual_metric.gab_con[1])*n_cau[1];
            n[2] = sqrt(actual_metric.gab[0] / actual_metric.gab_con[1])*n_cau[2];
            // Cauchy normal force in local cartesian e1,e2
            array_1d<double, 3> n_e = prod(actual_metric.T, n_cau);
            n[3] = n_e[0];
            n[4] = n_e[1];
            n[5] = n_e[2];
            // Principal normal forces
            n[6] = 0.5*(n_e[0] + n_e[1] + sqrt(pow(n_e[0] - n_e[1], 2) + 4.0*pow(n_e[2], 2)));
            n[7] = 0.5*(n_e[0] + n_e[1] - sqrt(pow(n_e[0] - n_e[1], 2) + 4.0*pow(n_e[2], 2)));


            double thickness = this->GetProperties().GetValue(THICKNESS);

            double W = pow(thickness, 2) / 6.0;
            double sigma_1_top = n[3] / thickness;
            double sigma_2_top = n[4] / thickness;
            double sigma_3_top = n[5] / thickness;
            double vMises = pow(pow(sigma_1_top, 2) + pow(sigma_2_top, 2) - sigma_1_top*sigma_2_top + 3 * pow(sigma_3_top, 2), 0.5);

            rOutput = vMises;
        }
        else if (rVariable == DAMAGE_T)
        {
            mConstitutiveLaw->GetValue(DAMAGE_T, rOutput);
        }
        else if (rVariable == DAMAGE_C)
        {
            mConstitutiveLaw->GetValue(DAMAGE_C, rOutput);
        }
        else
        {
            SurfaceBaseDiscreteElement::Calculate(rVariable, rOutput, rCurrentProcessInfo);
        }
    }

    void MembraneDiscreteElement::Calculate(
        const Variable<Vector>& rVariable,
        Vector& rValues,
        const ProcessInfo& rCurrentProcessInfo
    )
    {
        if (rValues.size() != 1)
        {
            rValues.resize(1);
        }

        if (rVariable == STRESSES)
        {
            // Create constitutive law parameters:
            ConstitutiveLaw::Parameters Values(GetGeometry(), GetProperties(), rCurrentProcessInfo);
            // Set constitutive law flags:
            Flags& ConstitutiveLawOptions = Values.GetOptions();
            ConstitutiveLawOptions.Set(ConstitutiveLaw::USE_ELEMENT_PROVIDED_STRAIN, false);
            ConstitutiveLawOptions.Set(ConstitutiveLaw::COMPUTE_STRESS, true);
            ConstitutiveLawOptions.Set(ConstitutiveLaw::COMPUTE_CONSTITUTIVE_TENSOR, true);

            MetricVariables actual_metric(3);
            CalculateMetric(actual_metric);
            ConstitutiveVariables constitutive_variables(3);
            CalculateConstitutiveVariables(actual_metric, constitutive_variables, Values, ConstitutiveLaw::StressMeasure_PK2);

            double detF = actual_metric.dA / mInitialMetric.dA;

            Vector n_pk2_ca = prod(constitutive_variables.DMembrane, constitutive_variables.StrainVector);
            Vector n_pk2_con = prod(mInitialMetric.T, n_pk2_ca);
            Vector n_cau = 1.0 / detF*n_pk2_con;

            Vector n = ZeroVector(8);
            // Cauchy normal force in normalized g1,g2
            n[0] = sqrt(actual_metric.gab[0] / actual_metric.gab_con[0])*n_cau[0];
            n[1] = sqrt(actual_metric.gab[1] / actual_metric.gab_con[1])*n_cau[1];
            n[2] = sqrt(actual_metric.gab[0] / actual_metric.gab_con[1])*n_cau[2];
            // Cauchy normal force in local cartesian e1,e2
            array_1d<double, 3> n_e = prod(actual_metric.T, n_cau);
            n[3] = n_e[0];
            n[4] = n_e[1];
            n[5] = n_e[2];
            // Principal normal forces
            n[6] = 0.5*(n_e[0] + n_e[1] + sqrt(pow(n_e[0] - n_e[1], 2) + 4.0*pow(n_e[2], 2)));
            n[7] = 0.5*(n_e[0] + n_e[1] - sqrt(pow(n_e[0] - n_e[1], 2) + 4.0*pow(n_e[2], 2)));

            double thickness = this->GetProperties().GetValue(THICKNESS);

            Vector sigma_top = ZeroVector(3);
            sigma_top(0) = n[3] / thickness;
            sigma_top(1) = n[4] / thickness;
            sigma_top(2) = n[5] / thickness;

            rValues = sigma_top;
        }
        else
        {
            SurfaceBaseDiscreteElement::Calculate(rVariable, rValues, rCurrentProcessInfo);
        }
    }


    void MembraneDiscreteElement::CalculateMassMatrix(
        MatrixType& rMassMatrix,
        ProcessInfo& rCurrentProcessInfo
    )
    {
        KRATOS_TRY;
        double integration_weight = this->GetValue(INTEGRATION_WEIGHT);
        Vector ShapeFunctionsN = this->GetValue(SHAPE_FUNCTION_VALUES);
        Matrix DN_De = this->GetValue(SHAPE_FUNCTION_LOCAL_DERIVATIVES);

        double thickness = this->GetProperties().GetValue(THICKNESS);
        double density = this->GetProperties().GetValue(DENSITY);
        double mass = thickness * density * mInitialMetric.dA * integration_weight;

        const int number_of_control_points = ShapeFunctionsN.size();
        const int mat_size = 3 * number_of_control_points;

        if (rMassMatrix.size1() != mat_size)
            rMassMatrix.resize(mat_size, mat_size, false);
        rMassMatrix = ZeroMatrix(mat_size, mat_size);

        for (int r = 0; r<number_of_control_points; r++)
        {
            for (int s = 0; s<number_of_control_points; s++)
            {
                rMassMatrix(3 * s,     3 * r)     = ShapeFunctionsN(s)*ShapeFunctionsN(r) * mass;
                rMassMatrix(3 * s + 1, 3 * r + 1) = rMassMatrix(3 * s, 3 * r);
                rMassMatrix(3 * s + 2, 3 * r + 2) = rMassMatrix(3 * s, 3 * r);
            }
        }
        KRATOS_CATCH("")
    }
    //************************************************************************************
    //************************************************************************************
    void MembraneDiscreteElement::CalculateMetric(
        MetricVariables& metric
    )
    {
        const Matrix& DN_De = this->GetValue(SHAPE_FUNCTION_LOCAL_DERIVATIVES);
        const Matrix& DDN_DDe = this->GetValue(SHAPE_FUNCTION_LOCAL_SECOND_DERIVATIVES);

        Jacobian(DN_De, metric.J);


        metric.g1[0] = metric.J(0, 0);
        metric.g2[0] = metric.J(0, 1);
        metric.g1[1] = metric.J(1, 0);
        metric.g2[1] = metric.J(1, 1);
        metric.g1[2] = metric.J(2, 0);
        metric.g2[2] = metric.J(2, 1);

        //basis vector g3
        MathUtils<double>::CrossProduct(metric.g3, metric.g1, metric.g2);
        //differential area dA
        metric.dA = norm_2(metric.g3);
        //normal vector _n
        Vector n = metric.g3 / metric.dA;


        //GetCovariantMetric
        metric.gab[0] = pow(metric.g1[0], 2) + pow(metric.g1[1], 2) + pow(metric.g1[2], 2);
        metric.gab[1] = pow(metric.g2[0], 2) + pow(metric.g2[1], 2) + pow(metric.g2[2], 2);
        metric.gab[2] = metric.g1[0] * metric.g2[0] + metric.g1[1] * metric.g2[1] + metric.g1[2] * metric.g2[2];

        Hessian(metric.H, DDN_DDe);

        metric.curvature[0] = metric.H(0, 0)*n[0] + metric.H(1, 0)*n[1] + metric.H(2, 0)*n[2];
        metric.curvature[1] = metric.H(0, 1)*n[0] + metric.H(1, 1)*n[1] + metric.H(2, 1)*n[2];
        metric.curvature[2] = metric.H(0, 2)*n[0] + metric.H(1, 2)*n[1] + metric.H(2, 2)*n[2];


        //contravariant metric gab_con and base vectors g_con
        //Vector gab_con = ZeroVector(3);
        double invdetGab = 1.0 / (metric.gab[0] * metric.gab[1] - metric.gab[2] * metric.gab[2]);
        metric.gab_con[0] = invdetGab*metric.gab[1];
        metric.gab_con[2] = -invdetGab*metric.gab[2];
        metric.gab_con[1] = invdetGab*metric.gab[0];


        array_1d<double, 3> g_con_1 = metric.g1*metric.gab_con[0] + metric.g2*metric.gab_con[2];
        array_1d<double, 3> g_con_2 = metric.g1*metric.gab_con[2] + metric.g2*metric.gab_con[1];


        //local cartesian coordinates
        double lg1 = norm_2(metric.g1);
        array_1d<double, 3> e1 = metric.g1 / lg1;
        double lg_con2 = norm_2(g_con_2);
        array_1d<double, 3> e2 = g_con_2 / lg_con2;

        //Matrix T_G_E = ZeroMatrix(3, 3);
        //Transformation matrix T from contravariant to local cartesian basis
        double eG11 = inner_prod(e1, metric.g1);
        double eG12 = inner_prod(e1, metric.g2);
        double eG21 = inner_prod(e2, metric.g1);
        double eG22 = inner_prod(e2, metric.g2);

        metric.Q = ZeroMatrix(3, 3);
        metric.Q(0, 0) = eG11*eG11;
        metric.Q(0, 1) = eG12*eG12;
        metric.Q(0, 2) = 2.0*eG11*eG12;
        metric.Q(1, 0) = eG21*eG21;
        metric.Q(1, 1) = eG22*eG22;
        metric.Q(1, 2) = 2.0*eG21*eG22;
        metric.Q(2, 0) = 2.0*eG11*eG21;
        metric.Q(2, 1) = 2.0*eG12*eG22;
        metric.Q(2, 2) = 2.0*eG11*eG22 + eG12*eG21;

        metric.T = ZeroMatrix(3, 3);
        metric.T(0, 0) = eG11*eG11;
        metric.T(0, 1) = eG21*eG21;
        metric.T(0, 2) = 2.0*eG11*eG21;
        metric.T(1, 0) = eG12*eG12;
        metric.T(1, 1) = eG22*eG22;
        metric.T(1, 2) = 2.0*eG12*eG22;
        metric.T(2, 0) = eG11*eG12;
        metric.T(2, 1) = eG21*eG22;
        metric.T(2, 2) = eG11*eG22 + eG12*eG21;
    }
    //************************************************************************************
    //************************************************************************************
    void MembraneDiscreteElement::CalculateConstitutiveVariables(
        MetricVariables& rActualMetric,
        ConstitutiveVariables& rThisConstitutiveVariables,
        ConstitutiveLaw::Parameters& rValues,
        const ConstitutiveLaw::StressMeasure ThisStressMeasure
    )
    {
        Vector strain_vector = ZeroVector(3);

        CalculateStrain(strain_vector, rActualMetric.gab, mInitialMetric.gab);
        rThisConstitutiveVariables.StrainVector = prod(mInitialMetric.Q, strain_vector);

        //Constitive Matrices DMembrane and DCurvature
        rValues.SetStrainVector(rThisConstitutiveVariables.StrainVector); //this is the input parameter
        rValues.SetStressVector(rThisConstitutiveVariables.StressVector);    //this is an ouput parameter
        rValues.SetConstitutiveMatrix(rThisConstitutiveVariables.DMembrane); //this is an ouput parameter

        mConstitutiveLaw->CalculateMaterialResponse(rValues, ThisStressMeasure);

        //Local Cartesian Forces and Moments
        rThisConstitutiveVariables.StressVector = prod(
            trans(rThisConstitutiveVariables.DMembrane), rThisConstitutiveVariables.StrainVector);
    }
} // Namespace Kratos


