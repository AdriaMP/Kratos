// KRATOS  ___|  |                   |                   |
//       \___ \  __|  __| |   |  __| __| |   |  __| _` | |
//             | |   |    |   | (    |   |   | |   (   | |
//       _____/ \__|_|   \__,_|\___|\__|\__,_|_|  \__,_|_| MECHANICS
//
//  License:		 BSD License
//					 license: structural_mechanics_application/license.txt
//
//  Main authors:    Mahmoud Sesa, https://github.com/mahmoudsesa
//
// System includes

#ifndef ADJOINT_NONLINEAR_STRAIN_ENERGY_RESPONSE_FUNCTION_H
#define ADJOINT_NONLINEAR_STRAIN_ENERGY_RESPONSE_FUNCTION_H

// External includes

// Project includes
#include "adjoint_structural_response_function.h"

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

///@}
///@name Kratos Classes
///@{

/** \brief AdjointNonlinearStrainEnergyResponseFunction
*
* This is a response function which traces nonlinear strain energy as a response
* It is used for nonlinear adjoint sensitivity analysis
*/

class KRATOS_API(STRUCTURAL_MECHANICS_APPLICATION) AdjointNonlinearStrainEnergyResponseFunction : public AdjointStructuralResponseFunction
{
public:
    ///@name Type Definitions
    ///@{

    typedef AdjointStructuralResponseFunction BaseType;

    ///@}
    ///@name Pointer Definitions
    ///@{

    /// Pointer definition of AdjointNonlinearStrainEnergyResponseFunction
    KRATOS_CLASS_POINTER_DEFINITION(AdjointNonlinearStrainEnergyResponseFunction);



    ///@}
    ///@name Life Cycle
    ///@{
    /// Default constructor.
    AdjointNonlinearStrainEnergyResponseFunction(ModelPart& rModelPart, Parameters ResponseSettings);

    /// Destructor.
    ~AdjointNonlinearStrainEnergyResponseFunction();

    ///@}
    ///@name Operators
    ///@{

    ///@}
    ///@name Operations
    ///@{

    void FinalizeSolutionStep() override;

    double CalculateValue(ModelPart& rModelPart) override;

    void CalculateGradient(const Element& rAdjointElement,
                                   const Matrix& rResidualGradient,
                                   Vector& rResponseGradient,
                                   const ProcessInfo& rProcessInfo) override;

     void CalculateGradient(const Condition& rAdjointCondition,
                                   const Matrix& rResidualGradient,
                                   Vector& rResponseGradient,
                                   const ProcessInfo& rProcessInfo) override;

    void CalculatePartialSensitivity(Element& rAdjointElement,
                                             const Variable<double>& rVariable,
                                             const Matrix& rSensitivityMatrix,
                                             Vector& rSensitivityGradient,
                                             const ProcessInfo& rProcessInfo) override;

    void CalculatePartialSensitivity(Element& rAdjointElement,
                                             const Variable<array_1d<double, 3>>& rVariable,
                                             const Matrix& rSensitivityMatrix,
                                             Vector& rSensitivityGradient,
                                             const ProcessInfo& rProcessInfo) override;

    void CalculatePartialSensitivity(Condition& rAdjointCondition,
                                             const Variable<double>& rVariable,
                                             const Matrix& rSensitivityMatrix,
                                             Vector& rSensitivityGradient,
                                             const ProcessInfo& rProcessInfo) override;

    void CalculatePartialSensitivity(Condition& rAdjointCondition,
                                             const Variable<array_1d<double, 3>>& rVariable,
                                             const Matrix& rSensitivityMatrix,
                                             Vector& rSensitivityGradient,
                                             const ProcessInfo& rProcessInfo) override;

    double CalculateAdjointScalingFactor(const Condition& rAdjointCondition,
                                                   const Matrix& rResidualGradient,
                                                   const ProcessInfo& rProcessInfo);

    //void GetGradientVector(const Condition& rAdjointCondition, Vector& rResponseGradient);

    ///@}
    ///@name Access
    ///@{

    ///@}
    ///@name Inquiry
    ///@{

    ///@}
    ///@name Input and output
    ///@{

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

    ModelPart* mpModelPart = nullptr;
    double m_response_value = 0.0;

    //Matrix mExternalForceDisplacementDerivative;


    std::map<int, Vector> mConditionsRHS;
    std::map<int,  ModelPart::ConditionsContainerType::iterator> mConditions;
    std::map<int, Vector> mResponseGradient_1;
    std::map<int, Vector> mResponseGradient_0;
    std::map<int, Matrix> mExternalForceDesignVariableDerivative;
    std::map<int, Matrix> mExternalForceDisplacementDerivative;

    ///@}
    ///@name Private Operators
    ///@{

    ///@}
    ///@name Private Operations
    ///@{
    void CheckForBodyForces(ModelPart& rModelPart);
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
}; // Class AdjointNonlinearStrainEnergyResponseFunction

///@}

///@name Type Definitions
///@{

///@}
///@name Input and output
///@{

///@}

}  // namespace Kratos.

#endif // ADJOINT_LINEAR_STRAIN_ENERGY_RESPONSE_FUNCTION_H_INCLUDED