//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		BSD License
//					Kratos default license: kratos/license.txt
//
//  Main authors:    Veronika Singer
//


// System includes

// External includes

// Project includes
#include "includes/define.h"
#include "custom_conditions/particle_based_conditions/mpm_particle_lagrange_dirichlet_condition.h"
#include "includes/kratos_flags.h"
#include "utilities/math_utils.h"
#include "custom_utilities/particle_mechanics_math_utilities.h"
#include "includes/checks.h"


namespace Kratos
{
//******************************* CONSTRUCTOR ****************************************
//************************************************************************************

MPMParticleLagrangeDirichletCondition::MPMParticleLagrangeDirichletCondition( IndexType NewId, GeometryType::Pointer pGeometry )
    : MPMParticleBaseDirichletCondition( NewId, pGeometry )
{
    //DO NOT ADD DOFS HERE!!!
}

//************************************************************************************
//************************************************************************************


MPMParticleLagrangeDirichletCondition::MPMParticleLagrangeDirichletCondition( IndexType NewId, GeometryType::Pointer pGeometry,  PropertiesType::Pointer pProperties)
    : MPMParticleBaseDirichletCondition( NewId, pGeometry, pProperties )
{
}

//********************************* CREATE *******************************************
//************************************************************************************

Condition::Pointer MPMParticleLagrangeDirichletCondition::Create(IndexType NewId,GeometryType::Pointer pGeometry,PropertiesType::Pointer pProperties) const
{
    return Kratos::make_intrusive<MPMParticleLagrangeDirichletCondition>(NewId, pGeometry, pProperties);
}

//************************************************************************************
//************************************************************************************

Condition::Pointer MPMParticleLagrangeDirichletCondition::Create( IndexType NewId, NodesArrayType const& ThisNodes,  PropertiesType::Pointer pProperties  ) const
{
    return Kratos::make_intrusive<MPMParticleLagrangeDirichletCondition>( NewId, GetGeometry().Create( ThisNodes ), pProperties);
}

//******************************* DESTRUCTOR *****************************************
//************************************************************************************

MPMParticleLagrangeDirichletCondition::~MPMParticleLagrangeDirichletCondition()
{
}

//************************************************************************************
//************************************************************************************

void MPMParticleLagrangeDirichletCondition::InitializeSolutionStep( ProcessInfo& rCurrentProcessInfo )
{
    MPMParticleBaseDirichletCondition::InitializeSolutionStep( rCurrentProcessInfo );

    GeometryType& r_geometry = GetGeometry();

    const unsigned int number_of_nodes = r_geometry.PointsNumber();
    const unsigned int dimension = r_geometry.WorkingSpaceDimension();
    auto pBoundaryParticle = GetValue(MPC_LAGRANGE_NODE);

    array_1d<double, 3 > & r_lagrange_multiplier  = pBoundaryParticle->FastGetSolutionStepValue(VECTOR_LAGRANGE_MULTIPLIER);

    for ( unsigned int j = 0; j < dimension; j++ )
    {
        r_lagrange_multiplier[0] *= 0.0;
    }

}

void MPMParticleLagrangeDirichletCondition::CalculateAll(
    MatrixType& rLeftHandSideMatrix, VectorType& rRightHandSideVector,
    ProcessInfo& rCurrentProcessInfo,
    bool CalculateStiffnessMatrixFlag,
    bool CalculateResidualVectorFlag
    )
{
    KRATOS_TRY

    GeometryType& r_geometry = GetGeometry();
    const unsigned int number_of_nodes = GetGeometry().size();
    const unsigned int dimension = GetGeometry().WorkingSpaceDimension();
    const unsigned int matrix_size = number_of_nodes * dimension + dimension;

    // Resizing as needed the LHS
    if ( CalculateStiffnessMatrixFlag == true )
    {
        if ( rLeftHandSideMatrix.size1() != matrix_size )
        {
            rLeftHandSideMatrix.resize( matrix_size, matrix_size, false );
        }

        noalias( rLeftHandSideMatrix ) = ZeroMatrix(matrix_size,matrix_size); //resetting LHS
    }

    // Resizing as needed the RHS
    if ( CalculateResidualVectorFlag == true ) //calculation of the matrix is required
    {
        if ( rRightHandSideVector.size( ) != matrix_size )
        {
            rRightHandSideVector.resize( matrix_size, false );
        }

        noalias( rRightHandSideVector ) = ZeroVector( matrix_size ); //resetting RHS
    }

    // Get imposed displacement and normal vector
    const array_1d<double, 3 > & xg_c = this->GetValue(MPC_COORD);
    const array_1d<double, 3 > & r_imposed_displacement = this->GetValue (MPC_IMPOSED_DISPLACEMENT);

    // Prepare variables
    GeneralVariables Variables;

    // Calculating shape function
    Variables.N = this->MPMShapeFunctionPointValues(Variables.N, xg_c);
    Variables.CurrentDisp = this->CalculateCurrentDisp(Variables.CurrentDisp, rCurrentProcessInfo);

    // Check contact: Check contact penetration: if <0 apply constraint, otherwise no
    bool apply_constraints = true;
    if (Is(CONTACT))
    {
        // NOTE: the unit_normal_vector is assumed always pointing outside the boundary
        array_1d<double, 3 > & r_unit_normal_vector = this->GetValue(MPC_NORMAL);
        ParticleMechanicsMathUtilities<double>::Normalize(r_unit_normal_vector);
        array_1d<double, 3 > field_displacement = ZeroVector(3);
        for ( unsigned int i = 0; i < number_of_nodes; i++ )
        {
            if (Variables.N[i] > std::numeric_limits<double>::epsilon() )
            {
                for ( unsigned int j = 0; j < dimension; j++)
                {
                    field_displacement[j] += Variables.N[i] * Variables.CurrentDisp(i,j);
                }
            }
        }

        const double penetration = MathUtils<double>::Dot((field_displacement - r_imposed_displacement), r_unit_normal_vector);

        // If penetrates, apply constraint, otherwise no
        if (penetration >= 0.00001)
        {
            apply_constraints = false;
        }
    }

    if (apply_constraints)
    {
        Matrix lagrange_matrix = ZeroMatrix(matrix_size, matrix_size);

        // Loop over shape functions of displacements
        for (unsigned int i = 0; i < number_of_nodes; i++)
        {

                const unsigned int ibase = dimension * number_of_nodes;

                // Matrix in following shape:
                // |0       N^T|
                // |N       0  |

                for (unsigned int k = 0; k < dimension; k++)
                {
                    lagrange_matrix(i* dimension+k, i* dimension+k) = 0.001/this->GetIntegrationWeight();
                    lagrange_matrix(i* dimension+k, ibase+k) = Variables.N[i];
                    lagrange_matrix(ibase+k, i*dimension + k) = Variables.N[i];
                }
        }

        lagrange_matrix  *= this->GetIntegrationWeight();

        // Calculate LHS Matrix and RHS Vector
        if ( CalculateStiffnessMatrixFlag == true )
        {
            rLeftHandSideMatrix = lagrange_matrix;
        }

        if ( CalculateResidualVectorFlag == true )
        {
            Vector gap_function = ZeroVector(matrix_size);
            for (unsigned int i = 0; i < number_of_nodes; i++)
            {
                const array_1d<double, 3>& r_displacement = r_geometry[i].FastGetSolutionStepValue(DISPLACEMENT);
                const int index = dimension * i;

                for (unsigned int j = 0; j < dimension; j++)
                    gap_function[index+j]          = r_displacement[j] - r_imposed_displacement[j];

            }
            auto pBoundaryParticle = GetValue(MPC_LAGRANGE_NODE);
            const array_1d<double, 3>& r_lagrange_multiplier = pBoundaryParticle->FastGetSolutionStepValue(VECTOR_LAGRANGE_MULTIPLIER);
            for (unsigned int j = 0; j < dimension; j++)
                    gap_function[dimension * number_of_nodes+j] = r_lagrange_multiplier[j];


            noalias(rRightHandSideVector) -= prod(lagrange_matrix, gap_function);
        }

    }
    else{
        // To improve stability: use identity matrix to avoid nonzero diagonal LHS matrix
        if ( CalculateStiffnessMatrixFlag == true )
        {
            noalias(rLeftHandSideMatrix) = IdentityMatrix(matrix_size);
        }
    }

    KRATOS_CATCH( "" )
}

//************************************************************************************
//************************************************************************************



void MPMParticleLagrangeDirichletCondition::EquationIdVector(
    EquationIdVectorType& rResult,
    ProcessInfo& rCurrentProcessInfo )
{
    KRATOS_TRY

    GeometryType& r_geometry = GetGeometry();
    const unsigned int number_of_nodes = r_geometry.size() ;
    const unsigned int dimension = r_geometry.WorkingSpaceDimension();
    if (rResult.size() != dimension * number_of_nodes + dimension)
    {
        rResult.resize(dimension * number_of_nodes + dimension ,false);
    }


    for (unsigned int i = 0; i < number_of_nodes; ++i)
    {
        unsigned int index = i * dimension;
        rResult[index    ] = r_geometry[i].GetDof(DISPLACEMENT_X).EquationId();
        rResult[index + 1] = r_geometry[i].GetDof(DISPLACEMENT_Y).EquationId();
        if(dimension == 3)
            rResult[index + 2] = r_geometry[i].GetDof(DISPLACEMENT_Z).EquationId();

    }

    unsigned int index = number_of_nodes * dimension;
    auto pBoundaryParticle = GetValue(MPC_LAGRANGE_NODE);

    if(!Is(SLIP))
        rResult[index    ] = pBoundaryParticle->GetDof(VECTOR_LAGRANGE_MULTIPLIER_X).EquationId();

    rResult[index + 1] = pBoundaryParticle->GetDof(VECTOR_LAGRANGE_MULTIPLIER_Y).EquationId();
    if(dimension == 3)
        rResult[index + 2] = pBoundaryParticle->GetDof(VECTOR_LAGRANGE_MULTIPLIER_Z).EquationId();


    KRATOS_CATCH("")
}

void MPMParticleLagrangeDirichletCondition::GetDofList(
    DofsVectorType& rElementalDofList,
    ProcessInfo& rCurrentProcessInfo
    )
{
    KRATOS_TRY

    GeometryType& r_geometry = GetGeometry();
    const unsigned int number_of_nodes = r_geometry.size();
    const unsigned int dimension =  r_geometry.WorkingSpaceDimension();
    rElementalDofList.resize(0);
    rElementalDofList.reserve(dimension * number_of_nodes + dimension);

    for (unsigned int i = 0; i < number_of_nodes; ++i)
    {
        rElementalDofList.push_back( r_geometry[i].pGetDof(DISPLACEMENT_X));
        rElementalDofList.push_back( r_geometry[i].pGetDof(DISPLACEMENT_Y));
        if(dimension == 3)
            rElementalDofList.push_back( r_geometry[i].pGetDof(DISPLACEMENT_Z));

    }
    auto pBoundaryParticle = GetValue(MPC_LAGRANGE_NODE);

    if(!Is(SLIP))
        rElementalDofList.push_back(pBoundaryParticle->pGetDof(VECTOR_LAGRANGE_MULTIPLIER_X));

    rElementalDofList.push_back(pBoundaryParticle->pGetDof(VECTOR_LAGRANGE_MULTIPLIER_Y));
    if(dimension == 3)
        rElementalDofList.push_back(pBoundaryParticle->pGetDof(VECTOR_LAGRANGE_MULTIPLIER_Z));



    KRATOS_CATCH("")
}

void MPMParticleLagrangeDirichletCondition::GetValuesVector(
    Vector& rValues,
    int Step
    )
{
    GeometryType& r_geometry = GetGeometry();
    const unsigned int number_of_nodes = r_geometry.size();
    const unsigned int dimension = r_geometry.WorkingSpaceDimension();
    const unsigned int mat_size = number_of_nodes * dimension + dimension;

    if (rValues.size() != mat_size)
    {
        rValues.resize(mat_size, false);
    }
    rValues = ZeroVector(mat_size);

    for (unsigned int i = 0; i < number_of_nodes; i++)
    {
        const array_1d<double, 3 > & r_displacement = r_geometry[i].FastGetSolutionStepValue(DISPLACEMENT, Step);
        KRATOS_WATCH(r_displacement)

        unsigned int index = i * dimension;

        for(unsigned int k = 0; k < dimension; ++k)
        {
            rValues[index + k] = r_displacement[k];
        }
    }
    auto pBoundaryParticle = GetValue(MPC_LAGRANGE_NODE);
    const array_1d<double, 3 > & r_lagrange_multiplier = pBoundaryParticle->FastGetSolutionStepValue(VECTOR_LAGRANGE_MULTIPLIER, Step);
    const unsigned int lagrange_index = number_of_nodes * dimension;
    for(unsigned int k = 0; k < dimension; ++k)
        {
            rValues[lagrange_index + k] = r_lagrange_multiplier[k];
        }
}

void MPMParticleLagrangeDirichletCondition::GetFirstDerivativesVector(
    Vector& rValues,
    int Step
    )
{
    GeometryType& r_geometry = GetGeometry();
    const unsigned int number_of_nodes = r_geometry.size();
    const unsigned int dimension = r_geometry.WorkingSpaceDimension();
    const unsigned int mat_size = number_of_nodes * dimension + dimension;

    if (rValues.size() != mat_size)
    {
        rValues.resize(mat_size, false);
    }
    rValues = ZeroVector(mat_size);

    for (unsigned int i = 0; i < number_of_nodes; i++)
    {
        const array_1d<double, 3 > & r_velocity = r_geometry[i].FastGetSolutionStepValue(VELOCITY, Step);
        const unsigned int index = i * dimension;
        for(unsigned int k = 0; k < dimension; ++k)
        {
            rValues[index + k] = r_velocity[k];
        }
    }
}

void MPMParticleLagrangeDirichletCondition::GetSecondDerivativesVector(
    Vector& rValues,
    int Step
    )
{
    GeometryType& r_geometry = GetGeometry();
    const unsigned int number_of_nodes = r_geometry.size();
    const unsigned int dimension = r_geometry.WorkingSpaceDimension();
    const unsigned int mat_size = number_of_nodes * dimension + dimension;

    if (rValues.size() != mat_size)
    {
        rValues.resize(mat_size, false);
    }
    rValues = ZeroVector(mat_size);

    for (unsigned int i = 0; i < number_of_nodes; i++)
    {
        const array_1d<double, 3 > & r_acceleration = r_geometry[i].FastGetSolutionStepValue(ACCELERATION, Step);
        const unsigned int index = i * dimension;
        for(unsigned int k = 0; k < dimension; ++k)
        {
            rValues[index + k] = r_acceleration[k];
        }
    }
}




} // Namespace Kratos