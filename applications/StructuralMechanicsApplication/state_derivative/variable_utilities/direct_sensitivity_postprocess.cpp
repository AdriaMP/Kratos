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


// System includes

// External includes

// Project includes
#include "utilities/openmp_utils.h"
#include "utilities/variable_utils.h"
#include "direct_sensitivity_postprocess.h"
#include "includes/kratos_parameters.h"
#include "utilities/math_utils.h"
//#include <iomanip>

namespace Kratos
{

    /// Constructor.
    DirectSensitivityPostprocess::DirectSensitivityPostprocess(ModelPart& rModelPart, DirectSensitivityResponseFunction& rResponseFunction, 
                                DirectSensitivityVariable& rDirectSensitivityVariable, Parameters SensitivitySettings)
      : mrModelPart(rModelPart) , mrResponseFunction(rResponseFunction) , mrDirectSensitivityVariable(rDirectSensitivityVariable)
    {
        KRATOS_TRY;

        Parameters default_settings(R"(
        {
            "sensitivity_model_part_name": "PLEASE_SPECIFY_SENSITIVITY_MODEL_PART",
            "build_mode": "static"
        })");

        SensitivitySettings.ValidateAndAssignDefaults(default_settings);

        auto sensitivity_model_part_name =
            SensitivitySettings["sensitivity_model_part_name"].GetString();
        if (sensitivity_model_part_name !=
            "PLEASE_SPECIFY_SENSITIVITY_MODEL_PART")
        {
            mpSensitivityModelPart = &mrModelPart.GetSubModelPart(sensitivity_model_part_name);
        }
        else
        {
            mpSensitivityModelPart = &mrModelPart;
        }

        mBuildMode = SensitivitySettings["build_mode"].GetString(); 

        //mVariableType = std::string("element_data_type");

        mVariableType = mrDirectSensitivityVariable.GetDesignVariableType();  

        //design_variable_name = std::string("I22");

        mDesignVariableName = mrDirectSensitivityVariable.GetDesignVariableName();       
        
        KRATOS_CATCH("");
    }



    /// Destructor.
    DirectSensitivityPostprocess::~DirectSensitivityPostprocess()
    {
    }



    void DirectSensitivityPostprocess::Initialize()
    {
        /*KRATOS_TRY;

        this->Clear();

        // Initialize flags.
        VariableUtils().SetNonHistoricalVariable(UPDATE_SENSITIVITIES, true, mpSensitivityModelPart->Nodes());
        VariableUtils().SetNonHistoricalVariable(UPDATE_SENSITIVITIES, true, mpSensitivityModelPart->Elements());
        VariableUtils().SetNonHistoricalVariable(UPDATE_SENSITIVITIES, true, mpSensitivityModelPart->Conditions());

        KRATOS_CATCH("");*/
    }



    void DirectSensitivityPostprocess::Clear()
    {
        /*KRATOS_TRY;    

        // Reset flags.
        VariableUtils().SetNonHistoricalVariable(UPDATE_SENSITIVITIES, false, mrModelPart.Nodes());
        VariableUtils().SetNonHistoricalVariable(UPDATE_SENSITIVITIES, false, mrModelPart.Elements());
        VariableUtils().SetNonHistoricalVariable(UPDATE_SENSITIVITIES, false, mrModelPart.Conditions());

        this->SetAllSensitivityVariablesToZero();         

        KRATOS_CATCH("");*/
    }



    void DirectSensitivityPostprocess::SetAllSensitivityVariablesToZero()
    {
        KRATOS_TRY;
                
        if (mVariableType == "nodal_data_type")
        {
            if (KratosComponents<Variable<double>>::Has(mDesignVariableName))                                          
                VariableUtils().SetToZero_ScalarVar( ReadScalarSensitivityVariables(mDesignVariableName), mrModelPart.Nodes() );
            else if (KratosComponents<Variable<array_1d<double,3>>>::Has(mDesignVariableName))
                VariableUtils().SetToZero_VectorVar( ReadVectorSensitivityVariables(mDesignVariableName), mrModelPart.Nodes() ); 
            else
                KRATOS_ERROR << "Unsupported variable: " <<  mDesignVariableName << "." << std::endl;
        }              
          
        // Set element sensitivity result variables to zero.
        if (mVariableType == "element_data_type")
        {
            #pragma omp parallel for
            for (int i = 0; i< static_cast<int> (mrModelPart.NumberOfElements()); ++i)
            {
                auto it = mrModelPart.ElementsBegin() + i;
                
                if ( KratosComponents<Variable<double>>::Has(mDesignVariableName) )
                    it->SetValue(ReadScalarSensitivityVariables(mDesignVariableName), ReadScalarSensitivityVariables(mDesignVariableName).Zero());
                else if ( KratosComponents<Variable<array_1d<double,3>>>::Has(mDesignVariableName) )
                    it->SetValue(ReadVectorSensitivityVariables(mDesignVariableName), ReadVectorSensitivityVariables(mDesignVariableName).Zero());  
                else
                    KRATOS_ERROR << "Unsupported variable: " <<  mDesignVariableName << "." << std::endl;              
            }
        }
        
        // Set conditional sensitivity result variables to zero.
        if (mVariableType == "condition_data_type")
        {
            #pragma omp parallel for
            for (int i = 0; i< static_cast<int> (mrModelPart.NumberOfConditions()); ++i)
            {
                auto it = mrModelPart.ConditionsBegin() + i;
                
                if ( KratosComponents<Variable<double>>::Has(mDesignVariableName) )
                    it->SetValue(ReadScalarSensitivityVariables(mDesignVariableName), ReadScalarSensitivityVariables(mDesignVariableName).Zero());
                else if ( KratosComponents<Variable<array_1d<double,3>>>::Has(mDesignVariableName) )
                    it->SetValue(ReadVectorSensitivityVariables(mDesignVariableName), ReadVectorSensitivityVariables(mDesignVariableName).Zero());
                else
                    KRATOS_ERROR << "Unsupported variable: " <<  mDesignVariableName << "." << std::endl;
            }
        }

        KRATOS_CATCH("");
    }    



    void DirectSensitivityPostprocess::UpdateSensitivities()
    {
        KRATOS_TRY;

        if (mBuildMode == "static")
        {
            // overwrite existing.
            this->SetAllSensitivityVariablesToZero();
        }
        else
        {
            KRATOS_ERROR << "Unsupported \"build_mode\": " << mBuildMode << std::endl;
        }
        
        // Define response variables ("MOMENT", "DISPLACEMENT" etc.) for which the sensitivity should get computed 
        /*std::vector<std::string> response_variables = mrResponseFunction.GetResponseSensitivityVariableVector();
        const SizeType num_resp_var = response_variables.size();
        
        // Define response output variables ("MOMENT_SENSITIVITY", "DISPLACEMENT_SENSITIVITY" etc.) to save the results 
        std::vector<std::string> response_output_variables;
        response_output_variables.resize(num_resp_var);

        for (IndexType i = 0; i < num_resp_var; ++i)
            response_output_variables[i] = response_variables[i] + std::string("_SENSITIVITY");*/
                
        //  Update the sensitivies
        /*for (IndexType i = 0; i < num_resp_var; ++i)
            if ( KratosComponents<Variable<double>>::Has(response_variables[i]) )
            {   
                if ( KratosComponents<Variable<double>>::Has(response_output_variables[i]) )
                {
                    this->UpdateElementContributionToSensitivity( ReadScalarSensitivityVariables(response_variables[i]), 
                                                    ReadScalarSensitivityVariables(response_output_variables[i]) );
                    this->UpdateConditionContributionToSensitivity( ReadScalarSensitivityVariables(response_variables[i]), 
                                                    ReadScalarSensitivityVariables(response_output_variables[i]) );
                }
                else
                    KRATOS_ERROR << "No matching output variable for " << response_variables[i] << "exist." << std::endl;
            }
            else if ( KratosComponents<Variable<array_1d<double,3>>>::Has(response_variables[i]) )
            {
                if ( KratosComponents<Variable<double>>::Has(response_output_variables[i]) )
                {
                    this->UpdateElementContributionToSensitivity( ReadVectorSensitivityVariables(response_variables[i]), 
                                                    ReadVectorSensitivityVariables(response_output_variables[i]) );
                    this->UpdateConditionContributionToSensitivity( ReadVectorSensitivityVariables(response_variables[i]), 
                                                    ReadVectorSensitivityVariables(response_output_variables[i]) );
                }
                else
                    KRATOS_ERROR << "No matching output variable for " << response_variables[i] << "exist." << std::endl;
            }
            else
                KRATOS_ERROR << "Unsupported variable: " <<  response_variables[i] << "." << std::endl;*/
        this->UpdateSensitivityOnGaussPoint(MOMENT, MOMENT);

        KRATOS_CATCH("");
    }



    template <typename TDataType>
    void DirectSensitivityPostprocess::UpdateSensitivityOnGaussPoint(Variable<TDataType> const& rResponseVariable, 
                        Variable<TDataType> const& rOutputVariable)
    {
        KRATOS_TRY;
        
        ProcessInfo& r_process_info = mrModelPart.GetProcessInfo();
        const int num_threads = 1;
        std::vector<std::vector<TDataType>> sensitivity_vector(num_threads);
        std::vector<std::vector<std::vector<TDataType>>> response_displacement_gradient(num_threads);
        std::vector<std::vector<TDataType>> response_sensitivity_gradient(num_threads);
        std::vector<Vector> displacement_gradient(num_threads);
        std::vector<std::vector<TDataType>> scalar_product(num_threads);
        
        int k = 0;
        
        for (auto& elem_i : mrModelPart.Elements())
        {   
            std::cout << "UpdateElement: ELEMENT: "<< elem_i.Id() << std::endl;
            //Element::GeometryType& r_geom = elem_i.GetGeometry();
            
            // No element contribution if the design variable is an condition data type
            if(mVariableType == "condition_data_type")
                break;  

            // Calculate derivative of the response function wrt. the displacements             
            mrResponseFunction.CalculateGradient(elem_i, rResponseVariable, response_displacement_gradient[k], r_process_info);

            OutputUtility::OutputOnTerminal("dg/du", response_displacement_gradient[k]);

            // Calculate derivative of the response function wrt. the design variable
            mrResponseFunction.CalculatePartialSensitivity(elem_i, mrDirectSensitivityVariable, rResponseVariable,
                                                response_sensitivity_gradient[k], r_process_info);            

            // Size sensitivity vector
            const SizeType num_traced_integr_pts = response_displacement_gradient[k][0].size();
            sensitivity_vector[k].resize(num_traced_integr_pts);
            scalar_product[k].resize(num_traced_integr_pts);
                                           
            // Get the displacement vector derived wrt. the design parameter
            elem_i.GetValuesVector(displacement_gradient[k]);
            
            OutputUtility::OutputOnTerminal("du/ds", displacement_gradient[k]);
                        
            if( (response_displacement_gradient[k].size() > 0) && (displacement_gradient[k].size() > 0) )
            {
                KRATOS_ERROR_IF_NOT( response_displacement_gradient[k].size() == displacement_gradient[k].size() ) << 
                    "Sizes of the response vector derived wrt. the displacement" <<
                    " and of the displacement vector derived wrt. the design parameter do not match!" << std::endl;
                 
                VectorMath::ScalarProduct(response_displacement_gradient[k], displacement_gradient[k], scalar_product[k]); 

                //OutputUtility::OutputOnTerminal("ScalarProduct", scalar_product[k]);

                VectorMath::SetToZero(sensitivity_vector[k]);

                VectorMath::Addition(sensitivity_vector[k], scalar_product[k]);
            }   
            if( response_sensitivity_gradient[k].size() > 0 )
            {
                KRATOS_ERROR_IF_NOT( response_sensitivity_gradient[k].size() == sensitivity_vector[k].size() ) << 
                    "Sizes of the sensitivity_vector and the response sensitivity gradient do not match!" << std::endl;
                
                //OutputUtility::OutputOnTerminal("dg/ds", response_sensitivity_gradient[k]);
                
                VectorMath::MultiplyByFactor(response_sensitivity_gradient[k], -1);

                VectorMath::Addition(sensitivity_vector[k], response_sensitivity_gradient[k]);

                OutputUtility::OutputOnTerminal("sensitivity_vector", sensitivity_vector[k]);
            }
            else
                OutputUtility::OutputOnTerminal("sensitivity_vector", sensitivity_vector[k]);   
        }

        KRATOS_CATCH("");
    }
    


    template <typename TDataType>
    void DirectSensitivityPostprocess::UpdateSensitivityOnNode(Variable<TDataType> const& rResponseVariable, 
                        Variable<TDataType> const& rOutputVariable)
    {
        KRATOS_TRY;
        
        ProcessInfo& r_process_info = mrModelPart.GetProcessInfo();
        const int num_threads = 1;
        std::vector<TDataType> sensitivity_vector(num_threads);
        std::vector<std::vector<TDataType>> response_displacement_gradient(num_threads);
        std::vector<TDataType> response_sensitivity_gradient(num_threads);
        std::vector<TDataType> displacement_gradient(num_threads);
        std::vector<TDataType> scalar_product(num_threads);
        
        int k = 0;
        
        for (auto& node_i : mrModelPart.Nodes())
        {   
            std::cout << "UpdateNode: Node: "<< node_i.Id() << std::endl; 

            // Calculate derivative of the response function wrt. the displacements             
            mrResponseFunction.CalculateGradient(node_i, rResponseVariable, response_displacement_gradient[k], r_process_info);

            OutputUtility::OutputOnTerminal("dg/du", response_displacement_gradient[k]);

            // Calculate derivative of the response function wrt. the design variable
            mrResponseFunction.CalculatePartialSensitivity(node_i, mrDirectSensitivityVariable, rResponseVariable,
                                                response_sensitivity_gradient[k], r_process_info);            

            // Size sensitivity vector
            
            sensitivity_vector[k].resize(0);
            scalar_product[k].resize(0);
                                           
            // Get the displacement vector derived wrt. the design parameter
            displacement_gradient[k] = node_i.FastGetSolutionStepValue(ADJOINT_ROTATION);
            
            OutputUtility::OutputOnTerminal("du/ds", displacement_gradient[k]);
                        
            if( (response_displacement_gradient[k].size() > 0) && (displacement_gradient[k].size() > 0) )
            {
                KRATOS_ERROR_IF_NOT( response_displacement_gradient[k].size() == displacement_gradient[k].size() ) << 
                    "Sizes of the response vector derived wrt. the displacement" <<
                    " and of the displacement vector derived wrt. the design parameter do not match!" << std::endl;
                 
                VectorMath::ScalarProduct(response_displacement_gradient[k], displacement_gradient[k], scalar_product[k]); 

                OutputUtility::OutputOnTerminal("ScalarProduct", scalar_product[k]);

                VectorMath::SetToZero(sensitivity_vector[k]);

                VectorMath::Addition(sensitivity_vector[k], scalar_product[k]);
            }   
            if( response_sensitivity_gradient[k].size() > 0 )
            {
                KRATOS_ERROR_IF_NOT( response_sensitivity_gradient[k].size() == sensitivity_vector[k].size() ) << 
                    "Sizes of the sensitivity_vector and the response sensitivity gradient do not match!" << std::endl;
                
                //OutputUtility::OutputOnTerminal("dg/ds", response_sensitivity_gradient[k]);
                
                VectorMath::MultiplyByFactor(response_sensitivity_gradient[k], -1);

                VectorMath::Addition(sensitivity_vector[k], response_sensitivity_gradient[k]);

                OutputUtility::OutputOnTerminal("sensitivity_vector", sensitivity_vector[k]);
            }
            else
                OutputUtility::OutputOnTerminal("sensitivity_vector", sensitivity_vector[k]);   
        }

        KRATOS_CATCH("");
    }
    
  

    void DirectSensitivityPostprocess::AssembleNodalSensitivityContribution(Variable<double> const& rSensitivityVariable,
                                              Vector const& rSensitivityVector, Element::GeometryType& rGeom)
    {
        IndexType index = 0;
        for (IndexType i_node = 0; i_node < rGeom.PointsNumber(); ++i_node)
        {
            if (rGeom[i_node].GetValue(UPDATE_SENSITIVITIES) == true)
            {
                double& r_sensitivity = rGeom[i_node].FastGetSolutionStepValue(rSensitivityVariable);
                rGeom[i_node].SetLock();
                r_sensitivity += rSensitivityVector[index++];
                rGeom[i_node].UnSetLock();
            }
            else
                ++index;
        }
    }


    void DirectSensitivityPostprocess::AssembleNodalSensitivityContribution(Variable<array_1d<double, 3>> const& rSensitivityVariable,
                                              Vector const& rSensitivityVector, Element::GeometryType& rGeom)
    {
        IndexType index = 0;
        for (IndexType i_node = 0; i_node < rGeom.PointsNumber(); ++i_node)
        {
            if (rGeom[i_node].GetValue(UPDATE_SENSITIVITIES) == true)
            {
                array_1d<double, 3>& r_sensitivity =
                    rGeom[i_node].FastGetSolutionStepValue(rSensitivityVariable);
                rGeom[i_node].SetLock();
                for (IndexType d = 0; d < rGeom.WorkingSpaceDimension(); ++d)
                    r_sensitivity[d] += rSensitivityVector[index++];
                rGeom[i_node].UnSetLock();
            }
            else
                index += rGeom.WorkingSpaceDimension();
        }
    }


    void DirectSensitivityPostprocess::AssembleElementSensitivityContribution(Variable<double> const& rVariable,
                                                Vector const& rSensitivityVector, Element& rElement)
    {
        std::cout << "In AssembleElementSensitivityContribution!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;

        KRATOS_DEBUG_ERROR_IF(rSensitivityVector.size() != 1) << "rSensitivityVector.size() = " << rSensitivityVector.size() << std::endl;
        rElement.GetValue(rVariable) += rSensitivityVector[0];
    }


    void DirectSensitivityPostprocess::AssembleElementSensitivityContribution(Variable<array_1d<double, 3>> const& rVariable,
                                                Vector const& rSensitivityVector, Element& rElement)
    {
        array_1d<double, 3>& r_sensitivity = rElement.GetValue(rVariable);
        const auto ws_dim = rElement.GetGeometry().WorkingSpaceDimension();
        KRATOS_DEBUG_ERROR_IF(rSensitivityVector.size() != ws_dim) << "rSensitivityVector.size() = " << rSensitivityVector.size() << std::endl;
        for (unsigned d = 0; d < ws_dim; ++d)
            r_sensitivity[d] += rSensitivityVector[d];
    }


    void DirectSensitivityPostprocess::AssembleConditionSensitivityContribution(Variable<double> const& rSensitivityVariable,
                                              Vector const& rSensitivityVector,
                                              Condition& rCondition)
    {
        KRATOS_DEBUG_ERROR_IF(rSensitivityVector.size() != 1) << "rSensitivityVector.size() = " << rSensitivityVector.size() << std::endl;
        rCondition.GetValue(rSensitivityVariable) += rSensitivityVector[0];
    }


    void DirectSensitivityPostprocess::AssembleConditionSensitivityContribution(Variable<array_1d<double, 3>> const& rSensitivityVariable,
                                              Vector const& rSensitivityVector,
                                              Condition& rCondition)
    {
        array_1d<double, 3>& r_sensitivity = rCondition.GetValue(rSensitivityVariable);
        const auto ws_dim = rCondition.GetGeometry().WorkingSpaceDimension();
        KRATOS_DEBUG_ERROR_IF(rSensitivityVector.size() != ws_dim) << "rSensitivityVector.size() = " << rSensitivityVector.size() << std::endl;
        for (unsigned d = 0; d < ws_dim; ++d)
            r_sensitivity[d] += rSensitivityVector[d];
    }

    Variable<double> DirectSensitivityPostprocess::ReadScalarSensitivityVariables(std::string const& rVariableName)
    {
        const Variable<double>& r_variable = KratosComponents<Variable<double>>::Get(rVariableName);
            return r_variable;                      
    }

    Variable<array_1d<double,3>> DirectSensitivityPostprocess::ReadVectorSensitivityVariables(std::string const& rVariableName)
    {
        const Variable<array_1d<double,3>>& r_variable =
                KratosComponents<Variable<array_1d<double,3>>>::Get(rVariableName);
            return r_variable;                  
    }
    

};


