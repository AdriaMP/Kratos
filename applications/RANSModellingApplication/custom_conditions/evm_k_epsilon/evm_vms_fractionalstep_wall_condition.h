//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ \.
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Suneth Warnakulasuriya
//

#ifndef KRATOS_EVM_VMS_FRACTIONALSTEP_WALL_CONDITION_H
#define KRATOS_EVM_VMS_FRACTIONALSTEP_WALL_CONDITION_H

// System includes
#include <iostream>
#include <string>

// External includes

// Project includes
#include "includes/condition.h"
#include "includes/define.h"
#include "includes/process_info.h"
#include "includes/serializer.h"

// Application includes
#include "custom_conditions/fs_generalized_wall_condition.h"
#include "custom_utilities/rans_calculation_utilities.h"
#include "includes/cfd_variables.h"
#include "rans_modelling_application_variables.h"

namespace Kratos
{
///@addtogroup FluidDynamicsApplication
///@{

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

///@}
///@name Kratos Classes
///@{

/// Implements a wall condition for the monolithic formulation.
/**
  It is intended to be used in combination with ASGS and VMS elements or their derived classes
  and the ResidualBasedPredictorCorrectorVelocityBossakSchemeTurbulent time scheme, which supports
  slip conditions.
  This condition will add a wall stress term to all nodes identified with IS_STRUCTURE!=0.0 (in the
  non-historic database, that is, assigned using Node.SetValue()). This stress term is determined
  according to the wall distance provided as Y_WALL.
  @see ASGS2D,ASGS3D,VMS,ResidualBasedPredictorCorrectorVelocityBossakSchemeTurbulent
 */
template <unsigned int TDim, unsigned int TNumNodes = TDim>
class EVMVMSFractionalStepWallCondition : public FSGeneralizedWallCondition<TDim, TNumNodes>
{
public:
    ///@name Type Definitions
    ///@{

    /// Pointer definition of EVMVMSFractionalStepWallCondition
    KRATOS_CLASS_INTRUSIVE_POINTER_DEFINITION(EVMVMSFractionalStepWallCondition);

    typedef FSGeneralizedWallCondition<TDim, TNumNodes> BaseType;

    typedef Node<3> NodeType;

    typedef Properties PropertiesType;

    typedef Geometry<NodeType> GeometryType;

    typedef Geometry<NodeType>::PointsArrayType NodesArrayType;

    typedef Vector VectorType;

    typedef Matrix MatrixType;

    typedef std::size_t IndexType;

    typedef std::size_t SizeType;

    typedef std::vector<std::size_t> EquationIdVectorType;

    typedef std::vector<Dof<double>::Pointer> DofsVectorType;

    typedef PointerVectorSet<Dof<double>, IndexedObject> DofsArrayType;

    ///@}
    ///@name Life Cycle
    ///@{

    /// Default constructor.
    /** Admits an Id as a parameter.
      @param NewId Index for the new condition
      */
    explicit EVMVMSFractionalStepWallCondition(IndexType NewId = 0) : BaseType(NewId)
    {
    }

    /// Constructor using an array of nodes
    /**
     @param NewId Index of the new condition
     @param ThisNodes An array containing the nodes of the new condition
     */
    EVMVMSFractionalStepWallCondition(IndexType NewId, const NodesArrayType& ThisNodes)
        : BaseType(NewId, ThisNodes)
    {
    }

    /// Constructor using Geometry
    /**
     @param NewId Index of the new condition
     @param pGeometry Pointer to a geometry object
     */
    EVMVMSFractionalStepWallCondition(IndexType NewId, GeometryType::Pointer pGeometry)
        : BaseType(NewId, pGeometry)
    {
    }

    /// Constructor using Properties
    /**
     @param NewId Index of the new element
     @param pGeometry Pointer to a geometry object
     @param pProperties Pointer to the element's properties
     */
    EVMVMSFractionalStepWallCondition(IndexType NewId,
                                  GeometryType::Pointer pGeometry,
                                  PropertiesType::Pointer pProperties)
        : BaseType(NewId, pGeometry, pProperties)
    {
    }

    /// Copy constructor.
    EVMVMSFractionalStepWallCondition(EVMVMSFractionalStepWallCondition const& rOther)
        : BaseType(rOther)
    {
    }

    /// Destructor.
    ~EVMVMSFractionalStepWallCondition() override
    {
    }

    ///@}
    ///@name Operators
    ///@{

    /// Assignment operator
    EVMVMSFractionalStepWallCondition& operator=(EVMVMSFractionalStepWallCondition const& rOther)
    {
        Condition::operator=(rOther);

        return *this;
    }

    ///@}
    ///@name Operations
    ///@{

    /// Create a new EVMVMSFractionalStepWallCondition object.
    /**
      @param NewId Index of the new condition
      @param ThisNodes An array containing the nodes of the new condition
      @param pProperties Pointer to the element's properties
      */
    Condition::Pointer Create(IndexType NewId,
                              NodesArrayType const& ThisNodes,
                              PropertiesType::Pointer pProperties) const override
    {
        return Kratos::make_intrusive<EVMVMSFractionalStepWallCondition>(
            NewId, this->GetGeometry().Create(ThisNodes), pProperties);
    }

    Condition::Pointer Create(IndexType NewId,
                              GeometryType::Pointer pGeom,
                              PropertiesType::Pointer pProperties) const override
    {
        return Kratos::make_intrusive<EVMVMSFractionalStepWallCondition>(NewId, pGeom, pProperties);
    }

    /**
     * Clones the selected element variables, creating a new one
     * @param NewId the ID of the new element
     * @param ThisNodes the nodes of the new element
     * @param pProperties the properties assigned to the new element
     * @return a Pointer to the new element
     */

    Condition::Pointer Clone(IndexType NewId, NodesArrayType const& rThisNodes) const override
    {
        Condition::Pointer pNewCondition = Create(
            NewId, this->GetGeometry().Create(rThisNodes), this->pGetProperties());

        pNewCondition->SetData(this->GetData());
        pNewCondition->SetFlags(this->GetFlags());

        return pNewCondition;
    }

    void Initialize() override
    {
	array_1d<double, 3> normal;
	this->CalculateNormal(normal);
	this->SetValue(NORMAL, normal);

	BaseType::Initialize();
    }

    // /// Provides the global indices for each one of this element's local rows.
    // /** This determines the elemental equation ID vector for all elemental DOFs
    //  * @param rResult A vector containing the global Id of each row
    //  * @param rCurrentProcessInfo the current process info object (unused)
    //  */
    // void EquationIdVector(EquationIdVectorType& rResult,
    //                       ProcessInfo& rCurrentProcessInfo) override;

    // /// Returns a list of the element's Dofs
    // /**
    //  * @param ElementalDofList the list of DOFs
    //  * @param rCurrentProcessInfo the current process info instance
    //  */
    // void GetDofList(DofsVectorType& ConditionDofList, ProcessInfo& CurrentProcessInfo) override;

    // void CalculateLocalVelocityContribution(MatrixType& rDampingMatrix,
    //                                         VectorType& rRightHandSideVector,
    //                                         ProcessInfo& rCurrentProcessInfo) override;

    ///@}
    ///@name Access
    ///@{

    ///@}
    ///@name Inquiry
    ///@{

    ///@}
    ///@name Input and output
    ///@{

    /// Turn back information as a string.
    std::string Info() const override
    {
        std::stringstream buffer;
        buffer << "EVMVMSFractionalStepWallCondition" << TDim << "D";
        return buffer.str();
    }

    /// Print information about this object.
    void PrintInfo(std::ostream& rOStream) const override
    {
        rOStream << "EVMVMSFractionalStepWallCondition";
    }

    /// Print object's data.
    void PrintData(std::ostream& rOStream) const override
    {
    }

    ///@}
    ///@name Friends
    ///@{

    ///@}

protected:
    ///@name Protected static Member Variables
    ///@{

    ///@}
    ///@name Protected member Variables
    ///@{

    ///@}
    ///@name Protected Operators
    ///@{

    ///@}
    ///@name Protected Operations
    ///@{

    void ApplyWallLaw(MatrixType& rLocalMatrix,
                      VectorType& rLocalVector,
                      ProcessInfo& rCurrentProcessInfo) override
    {
        if (!rCurrentProcessInfo[IS_CO_SOLVING_PROCESS_ACTIVE])
        {
            BaseType::ApplyWallLaw(rLocalMatrix, rLocalVector, rCurrentProcessInfo);
            return;
        }

        if (!this->Is(SLIP))
            return;

        RansCalculationUtilities rans_calculation_utilities;

        GeometryType& r_geometry = this->GetGeometry();

        const GeometryType::IntegrationPointsArrayType& integration_points =
            r_geometry.IntegrationPoints(GeometryData::GI_GAUSS_2);
        const std::size_t number_of_gauss_points = integration_points.size();
        MatrixType shape_functions =
            r_geometry.ShapeFunctionsValues(GeometryData::GI_GAUSS_2);

        array_1d<double, 3> normal;
        this->CalculateNormal(normal); // this already contains the area
        double A = norm_2(normal);

        // CAUTION: "Jacobian" is 2.0*A for triangles but 0.5*A for lines
        double J = (TDim == 2) ? 0.5 * A : 2.0 * A;

        const size_t block_size = TDim;

        const double c_mu_25 = std::pow(rCurrentProcessInfo[TURBULENCE_RANS_C_MU], 0.25);
        const double inv_von_karman = 1.0 / rCurrentProcessInfo[WALL_VON_KARMAN];
        const double beta = rCurrentProcessInfo[WALL_SMOOTHNESS_BETA];

        const double eps = std::numeric_limits<double>::epsilon();

        for (size_t g = 0; g < number_of_gauss_points; ++g)
        {
            const Vector& gauss_shape_functions = row(shape_functions, g);
            const double weight = J * integration_points[g].Weight();

            const array_1d<double, 3>& r_wall_velocity =
                rans_calculation_utilities.EvaluateInPoint(
                    r_geometry, VELOCITY, gauss_shape_functions);
            const double wall_velocity_magnitude = norm_2(r_wall_velocity);

            const double tke = rans_calculation_utilities.EvaluateInPoint(
                r_geometry, TURBULENT_KINETIC_ENERGY, gauss_shape_functions);
            const double y_plus = rans_calculation_utilities.EvaluateInPoint(
                r_geometry, RANS_Y_PLUS, gauss_shape_functions);
            const double rho = rans_calculation_utilities.EvaluateInPoint(
                r_geometry, DENSITY, gauss_shape_functions);

            if (wall_velocity_magnitude > eps)
            {
                const double u_tau = std::max(
                    c_mu_25 * std::sqrt(std::max(tke, 0.0)),
                    wall_velocity_magnitude / (inv_von_karman * std::log(y_plus) + beta));
                const double value = rho * std::pow(u_tau, 2) * weight / wall_velocity_magnitude;

                for (size_t a = 0; a < r_geometry.PointsNumber(); ++a)
                {
                    for (size_t dim = 0; dim < TDim; ++dim)
                    {
                        for (size_t b = 0; b < r_geometry.PointsNumber(); ++b)
                        {
                            rLocalMatrix(a * block_size + dim, b * block_size + dim) +=
                                gauss_shape_functions[a] * gauss_shape_functions[b] * value;
                        }
                        rLocalVector[a * block_size + dim] -=
                            gauss_shape_functions[a] * value * r_wall_velocity[dim];
                    }
                }
            }
        }
    }

    ///@}
    ///@name Protected  Access
    ///@{

    ///@}
    ///@name Protected Inquiry
    ///@{

    ///@}
    ///@name Protected LifeCycle
    ///@{

    ///@}

private:
    ///@name Static Member Variables
    ///@{

    ///@}
    ///@name Member Variables
    ///@{

    ///@}
    ///@name Serialization
    ///@{

    friend class Serializer;

    void save(Serializer& rSerializer) const override
    {
        KRATOS_SERIALIZE_SAVE_BASE_CLASS(rSerializer, Condition);
    }

    void load(Serializer& rSerializer) override
    {
        KRATOS_SERIALIZE_LOAD_BASE_CLASS(rSerializer, Condition);
    }

    ///@}
    ///@name Private Operators
    ///@{

    ///@}
    ///@name Private Operations
    ///@{

    void CalculateNormal(array_1d<double,3>& An );

    ///@}
    ///@name Private  Access
    ///@{

    ///@}
    ///@name Private Inquiry
    ///@{

    ///@}
    ///@name Un accessible methods
    ///@{

    ///@}

}; // Class EVMVMSFractionalStepWallCondition

///@}

///@name Type Definitions
///@{

///@}
///@name Input and output
///@{

/// input stream function
template <unsigned int TDim, unsigned int TNumNodes>
inline std::istream& operator>>(std::istream& rIStream,
                                EVMVMSFractionalStepWallCondition<TDim, TNumNodes>& rThis)
{
    return rIStream;
}

/// output stream function
template <unsigned int TDim, unsigned int TNumNodes>
inline std::ostream& operator<<(std::ostream& rOStream,
                                const EVMVMSFractionalStepWallCondition<TDim, TNumNodes>& rThis)
{
    rThis.PrintInfo(rOStream);
    rOStream << std::endl;
    rThis.PrintData(rOStream);

    return rOStream;
}

///@}

///@} addtogroup block

} // namespace Kratos.

#endif // KRATOS_EVM_VMS_FRACTIONALSTEP_WALL_CONDITION_H
