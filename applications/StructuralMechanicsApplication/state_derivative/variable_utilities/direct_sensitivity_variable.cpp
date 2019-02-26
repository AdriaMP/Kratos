// KRATOS  ___|  |                   |                   |
//       \___ \  __|  __| |   |  __| __| |   |  __| _` | |
//             | |   |    |   | (    |   |   | |   (   | |
//       _____/ \__|_|   \__,_|\___|\__|\__,_|_|  \__,_|_| MECHANICS
//
//  License:		 BSD License
//					 license: structural_mechanics_application/license.txt
//
//  Main authors:    Kevin Braun, https://github.com/MFusseder
//


// System includes

// External includes

// Project includes
#include "utilities/openmp_utils.h"
#include "utilities/variable_utils.h"
#include "direct_sensitivity_variable.h"

namespace Kratos
{

    /// Constructor.
    DirectSensitivityVariable::DirectSensitivityVariable(ModelPart& rModelPart, Parameters VariableSettings)
      : mrModelPart(rModelPart)
    {
        KRATOS_TRY;

       //Get perturbation size
        mDelta = VariableSettings["delta"].GetDouble();
        
        // Get the name of the design variable 
        mDesignVariableName = VariableSettings["design_variable_name"].GetString();

        // Get the type of the design variable
        mVariableType = VariableSettings["variable_type"].GetString();

        // Set the perturbation size on all elements and conditions
        VariableUtils().SetNonHistoricalVariable(PERTURBATION_SIZE, mDelta, mrModelPart.Elements());
        VariableUtils().SetNonHistoricalVariable(PERTURBATION_SIZE, mDelta, mrModelPart.Conditions());
        
        KRATOS_CATCH("");
    }

    /// Destructor.
    DirectSensitivityVariable::~DirectSensitivityVariable(){}
    
            
    void DirectSensitivityVariable::Initialize()
    {
        KRATOS_TRY;
        
        KRATOS_CATCH("");
    }

      
    void DirectSensitivityVariable::CalculatePseudoLoadVector(Element& rDirectElement, const Matrix& rRHS, 
                                    Vector& rPseudoLoadVector, const ProcessInfo& rProcessInfo)
    {
        KRATOS_ERROR << "CalculatePseudoLoadVector should be implemented in the derived class." << std::endl;
    }  


    void DirectSensitivityVariable::CalculatePseudoLoadVector(Condition& rDirectCondition, const Matrix& rLHS, 
                                    Vector& rPseudoLoadVector, const ProcessInfo& rProcessInfo)
    {
        KRATOS_ERROR << "CalculatePseudoLoadVector should be implemented in the derived class." << std::endl;
    }  

    std::string DirectSensitivityVariable::GetDesignVariableName() 
    {              
        return mDesignVariableName;
    }
    
    
    std::string DirectSensitivityVariable::GetDesignVariableType() 
    {              
        return mVariableType;
    }

    unsigned int DirectSensitivityVariable::GetTracedElementId() 
    {              
        KRATOS_ERROR << "GetTracedElementId() should be implemented in the derived class." << std::endl;
        return 0;
    }

    double DirectSensitivityVariable::GetPerturbationSize()
    {              
        return mDelta;
    }
}   