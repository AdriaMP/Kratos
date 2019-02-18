// KRATOS  ___|  |                   |                   |
//       \___ \  __|  __| |   |  __| __| |   |  __| _` | |
//             | |   |    |   | (    |   |   | |   (   | |
//       _____/ \__|_|   \__,_|\___|\__|\__,_|_|  \__,_|_| MECHANICS
//
//  License:		 BSD License
//					 license: structural_mechanics_application/license.txt
//
//  Main authors:    Martin Fusseder, https://github.com/MFusseder
//

#ifndef DIRECT_POSTPROCESS_H
#define DIRECT_POSTPROCESS_H

// System includes

// External includes

// Project includes
#include "includes/model_part.h"
#include "includes/kratos_parameters.h"
#include "structural_mechanics_application_variables.h"
#include "state_derivative/response_functions/direct_sensitivity_response_function.h"
#include "state_derivative/variable_utilities/direct_sensitivity_variable.h"
#include "state_derivative/math_functions/vector_math.h"
#include "state_derivative/output_utilities/output_utility.h"


//#include "state_derivative/new_adjoint_response_functions/new_adjoint_response_function.h"


namespace Kratos
{
///@addtogroup StructuralMechanicsApplication
///@{

///@name Kratos Classes
///@{

/** \brief DirectSensitivityPostprocess
*
* This class is responsible to perform the post-processing step in which the
* sensitvities are computed.
* It is designed to be used in adjoint sensitivity analysis.
*/
class KRATOS_API(STRUCTURAL_MECHANICS_APPLICATION) DirectSensitivityPostprocess
{
public:
    ///@name Type Definitions
    ///@{

    KRATOS_CLASS_POINTER_DEFINITION(DirectSensitivityPostprocess);

    typedef VariableComponent<VectorComponentAdaptor<array_1d<double, 3>>> VariableComponentType;

    typedef Variable<array_1d<double, 3>> VariableWithComponentsType;

    typedef ModelPart::NodesContainerType NodesContainerType;

    typedef ModelPart::ElementsContainerType ElementsContainerType;

    typedef ModelPart::ConditionsContainerType ConditionsContainerType;

    typedef std::size_t IndexType;

    typedef std::size_t SizeType;

    ///@}
    ///@name Pointer Definitions

    ///@}
    ///@name Life Cycle
    ///@{

    /// Constructor.
    DirectSensitivityPostprocess(ModelPart& rModelPart, DirectSensitivityResponseFunction& rResponseFunction, DirectSensitivityVariable& rDirectSensitivityVariable,
                       Parameters VariableSettings);

    /// Destructor.
    virtual ~DirectSensitivityPostprocess();

    ///@}
    ///@name Operators
    ///@{

    ///@}
    ///@name Operations
    ///@{

    virtual void Initialize();

    virtual void InitializeSolutionStep(){};

    virtual void FinalizeSolutionStep(){};

    virtual void Clear();

    virtual void UpdateSensitivities();

    ///@}

protected:
    ///@name Protected member Variables
    ///@{

    ///@}
    ///@name Protected Operators
    ///@{

    ///@}
    ///@name Protected Operations
    ///@{

            

    void SetAllSensitivityVariablesToZero();
    
    template <typename TDataType>
    void UpdateSensitivityOnGaussPoint(Variable<TDataType> const& rResponseVariable, 
                                            Variable<TDataType> const& rOutputVariable);
    
    template <typename TDataType>
    void UpdateSensitivityOnNode(Variable<TDataType> const& rResponseVariable, 
                                            Variable<TDataType> const& rOutputVariable);

    void AssembleNodalSensitivityContribution(Variable<double> const& rSensitivityVariable,
                                            Vector const& rSensitivityVector, Element::GeometryType& rGeom);

    void AssembleNodalSensitivityContribution(Variable<array_1d<double, 3>> const& rSensitivityVariable,
                                            Vector const& rSensitivityVector, Element::GeometryType& rGeom);

    void AssembleElementSensitivityContribution(Variable<double> const& rSensitivityVariable,
                                            Vector const& rSensitivityVector, Element& rElement);

    void AssembleElementSensitivityContribution(Variable<array_1d<double, 3>> const& rSensitivityVariable,
                                            Vector const& rSensitivityVector, Element& rElement);

    void AssembleConditionSensitivityContribution(Variable<double> const& rSensitivityVariable,
                                            Vector const& rSensitivityVector, Condition& rCondition);

    void AssembleConditionSensitivityContribution(Variable<array_1d<double, 3>> const& rSensitivityVariable,
                                            Vector const& rSensitivityVector, Condition& rCondition);

    
    
    ///@}

private:
    ///@name Member Variables
    ///@{

    ModelPart* mpSensitivityModelPart = nullptr;
    ModelPart& mrModelPart;
    DirectSensitivityResponseFunction& mrResponseFunction;
    DirectSensitivityVariable& mrDirectSensitivityVariable;
    std::string mBuildMode;
    std::string mVariableType;
    std::string mDesignVariableName;

    ///@}
    ///@name Private Operators
    ///@{

    ///@}
    ///@name Private Operations
    ///@{
        Variable<double> ReadScalarSensitivityVariables(std::string const& rVariableName);
    
        Variable<array_1d<double,3>> ReadVectorSensitivityVariables(std::string const& rVariableName);
        
    ///@}
};

///@} // Kratos Classes

///@} //Structural Mechanics Application group

} /* namespace Kratos.*/

#endif /* KRATOS_DIRECT_POSTPROCESS defined */
