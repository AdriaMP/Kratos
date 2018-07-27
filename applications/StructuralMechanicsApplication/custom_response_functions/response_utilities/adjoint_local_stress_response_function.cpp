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

// ------------------------------------------------------------------------------
// System includes
// ------------------------------------------------------------------------------

// ------------------------------------------------------------------------------
// External includes
// ------------------------------------------------------------------------------

// ------------------------------------------------------------------------------
// Project includes
// ------------------------------------------------------------------------------
#include "adjoint_local_stress_response_function.h"

namespace Kratos
{
    /// Default constructor.
    AdjointLocalStressResponseFunction::AdjointLocalStressResponseFunction(ModelPart& rModelPart, Parameters& rParameters)
    : AdjointStructuralResponseFunction(rModelPart, rParameters)
    {
        ModelPart& r_model_part = this->GetModelPart();

        ResponseData stress_response_data;

        // Get traced element
        mIdOfTracedElement = rParameters["traced_element"].GetInt();
        mpTracedElement = r_model_part.pGetElement(mIdOfTracedElement);

        // Tell traced element the stress type
        TracedStressType traced_stress_type = stress_response_data.ConvertStressType(rParameters["stress_type"].GetString()); 
        KRATOS_ERROR_IF(traced_stress_type == StressTypeNotAvailible) << "Chosen stress type is not availible!" << std::endl;
        mpTracedElement->SetValue(TRACED_STRESS_TYPE, static_cast<int>(traced_stress_type) );		

        // Get info how and where to treat the stress
        mStressTreatment = stress_response_data.ConvertStressTreatment( rParameters["stress_treatment"].GetString() );
        KRATOS_ERROR_IF(mStressTreatment == StressTreatmentNotAvailible) << "Chosen option for stress treatmeant is not availible! Chose 'GP','node' or 'mean'!" << std::endl;

        if(mStressTreatment == GP || mStressTreatment == node)
        {
            mIdOfLocation = rParameters["stress_location"].GetInt();
            KRATOS_ERROR_IF(mIdOfLocation < 1) << "Chose a 'stress_location' > 0. Specified 'stress_location': " << mIdOfLocation << std::endl;
        }

        mStressValue = 0.0;
    }

    /// Destructor.
    AdjointLocalStressResponseFunction::~AdjointLocalStressResponseFunction(){}

    void AdjointLocalStressResponseFunction::Initialize()
    {
        KRATOS_TRY;

        BaseType::Initialize();

        KRATOS_CATCH("");
    }

    // ==============================================================================
    double AdjointLocalStressResponseFunction::CalculateValue(ModelPart& rModelPart)
    {
        KRATOS_TRY;

        // Working variables
        ProcessInfo &r_current_precess_info = rModelPart.GetProcessInfo();
        Vector element_stress;

        if(mStressTreatment == mean || mStressTreatment == GP)
            mpTracedElement->Calculate(STRESS_ON_GP, element_stress, r_current_precess_info);
        else
            mpTracedElement->Calculate(STRESS_ON_NODE, element_stress, r_current_precess_info);

        int stress_vec_size = element_stress.size();

        if(mStressTreatment == mean)
        {
            for(int i = 0; i < stress_vec_size; i++)
                mStressValue += element_stress[i];

            mStressValue /= stress_vec_size;
        }
        else if(mStressTreatment == GP)
        {
            if(stress_vec_size >= mIdOfLocation)
                mStressValue = element_stress[mIdOfLocation - 1];
            else
                KRATOS_ERROR << "Chosen Gauss-Point is not availible. Chose 'stress_location' between 1 and " <<
                                stress_vec_size  << "!"<< std::endl;
        }
        else if(mStressTreatment == node)
        {
            const int num_ele_nodes = mpTracedElement->GetGeometry().PointsNumber();
            if(num_ele_nodes >= mIdOfLocation)
                mStressValue = element_stress[mIdOfLocation - 1];
            else
                KRATOS_ERROR << "Chosen Node is not availible. The element has only " <<
                                num_ele_nodes  << " nodes."<< std::endl;

        }

        return mStressValue;

        KRATOS_CATCH("");
    }

    // ==============================================================================
    void AdjointLocalStressResponseFunction::CalculateGradient(const Element& rAdjointElem, const Matrix& rAdjointMatrix,
                                   Vector& rResponseGradient,
                                   ProcessInfo& rProcessInfo) 
    {
        if (rResponseGradient.size() != rAdjointMatrix.size1())
            rResponseGradient.resize(rAdjointMatrix.size1(), false);

        rResponseGradient.clear();

        if(rAdjointElem.Id() == mIdOfTracedElement)
        {
            Matrix stress_displ_deriv;
            if(mStressTreatment == mean || mStressTreatment == GP)
                mpTracedElement->Calculate(STRESS_DISP_DERIV_ON_GP, stress_displ_deriv, rProcessInfo);
            else
                mpTracedElement->Calculate(STRESS_DISP_DERIV_ON_NODE, stress_displ_deriv, rProcessInfo);

            int num_of_dofs = stress_displ_deriv.size1();
            int num_of_deriv = stress_displ_deriv.size2();
            double stress_displ_deriv_value = 0.0;

            KRATOS_ERROR_IF(rResponseGradient.size() != stress_displ_deriv.size1())
                 << "Size of stress displacement derivative does not fit!" << std::endl;
            std::cout << "************************************"<< std::endl;  
            for (int dof_it = 0 ; dof_it < num_of_dofs; dof_it++)
            {
                if(mStressTreatment == mean)
                {
                    for(int GP_it = 0; GP_it < num_of_deriv; GP_it++)
                        stress_displ_deriv_value += stress_displ_deriv(dof_it, GP_it);

                    stress_displ_deriv_value /= num_of_deriv;
                }
                else if(mStressTreatment == GP)
                {
                    if(num_of_deriv >= mIdOfLocation)
                        stress_displ_deriv_value = stress_displ_deriv(dof_it, (mIdOfLocation-1));
                    else
                        KRATOS_ERROR << "Chosen Gauss-Point is not availible. Chose 'stress_location' between 1 and " <<
                                    num_of_deriv  << "!"<< std::endl;
                }
                else if(mStressTreatment == node)
                {
                    if(num_of_deriv >= mIdOfLocation)
                        stress_displ_deriv_value = stress_displ_deriv(dof_it, (mIdOfLocation-1));
                    else
                        KRATOS_ERROR << "Chosen node is not availible. The element has only " <<
                                    num_of_deriv  << " nodes."<< std::endl;

                }
                rResponseGradient[dof_it] = (-1) * stress_displ_deriv_value;

                std::cout <<    rResponseGradient[dof_it] << std::endl;
                         
                stress_displ_deriv_value = 0.0;
            }
            std::cout << "************************************"<< std::endl;  
        }
    }

    // ==============================================================================
    void AdjointLocalStressResponseFunction::CalculateSensitivityGradient(Element& rAdjointElem,
                                      const Variable<double>& rVariable,
                                      const Matrix& rDerivativesMatrix,
                                      Vector& rResponseGradient,
                                      ProcessInfo& rProcessInfo) 
    {
          KRATOS_TRY


        if(rAdjointElem.Id() == mIdOfTracedElement)
        {
            rAdjointElem.SetValue(DESIGN_VARIABLE_NAME, rVariable.Name());

            Matrix stress_DV_deriv;
            if(mStressTreatment == mean || mStressTreatment == GP)
                rAdjointElem.Calculate(STRESS_DESIGN_DERIVATIVE_ON_GP, stress_DV_deriv, rProcessInfo);
            else
                rAdjointElem.Calculate(STRESS_DESIGN_DERIVATIVE_ON_NODE, stress_DV_deriv, rProcessInfo);

            int num_of_DV = stress_DV_deriv.size1();
            int num_of_deriv = stress_DV_deriv.size2();
            double stress_DV_deriv_value = 0.0;

            if(rResponseGradient.size() != stress_DV_deriv.size1())
                rResponseGradient.resize(stress_DV_deriv.size1(), false);
            KRATOS_ERROR_IF(rResponseGradient.size() != rDerivativesMatrix.size1())
                 << "Size of partial stress design variable derivative does not fit!" << std::endl;

            for (int dv_it = 0 ; dv_it < num_of_DV; dv_it++)
            {
                if(mStressTreatment == mean)
                {
                    for(int GP_it = 0; GP_it < num_of_deriv; GP_it++)
                        stress_DV_deriv_value += stress_DV_deriv(dv_it, GP_it);

                    stress_DV_deriv_value /= num_of_deriv;
                }
                else if(mStressTreatment == GP)
                {
                    if(num_of_deriv >= mIdOfLocation)
                        stress_DV_deriv_value = stress_DV_deriv(dv_it, (mIdOfLocation-1));
                    else
                        KRATOS_ERROR << "Chosen Gauss-Point is not availible. Chose 'stress_location' between 1 and " <<
                                    num_of_deriv  << "!"<< std::endl;
                }
                else if(mStressTreatment == node)
                {
                    if(num_of_deriv >= mIdOfLocation)
                        stress_DV_deriv_value = stress_DV_deriv(dv_it, (mIdOfLocation-1));
                    else
                        KRATOS_ERROR << "Chosen node is not availible. The element has only " <<
                                    num_of_deriv  << " nodes."<< std::endl;
                }
                rResponseGradient[dv_it] =  stress_DV_deriv_value;
                stress_DV_deriv_value = 0.0;
            }

            rAdjointElem.SetValue(DESIGN_VARIABLE_NAME, "");
        }
        else
        {
            if (rResponseGradient.size() != rDerivativesMatrix.size1())
                      rResponseGradient.resize(rDerivativesMatrix.size1(), false);
            rResponseGradient.clear();
        }

        KRATOS_CATCH("")
    }

    // ==============================================================================
    void AdjointLocalStressResponseFunction::CalculateSensitivityGradient(Condition& rAdjointCondition,
                                     const Variable<double>& rVariable,
                                     const Matrix& rDerivativesMatrix,
                                     Vector& rResponseGradient,
                                     ProcessInfo& rProcessInfo) 
    {
        KRATOS_TRY;

        if (rResponseGradient.size() != rDerivativesMatrix.size1())
                  rResponseGradient.resize(rDerivativesMatrix.size1(), false);
        rResponseGradient.clear();

        KRATOS_CATCH("");
    }

    // ==============================================================================
    void AdjointLocalStressResponseFunction::CalculateSensitivityGradient(Element& rAdjointElem,
                                      const Variable<array_1d<double,3>>& rVariable,
                                      const Matrix& rDerivativesMatrix,
                                      Vector& rResponseGradient,
                                      ProcessInfo& rProcessInfo)
    {
        KRATOS_TRY;

        if(rAdjointElem.Id() == mIdOfTracedElement)
        {
            rAdjointElem.SetValue(DESIGN_VARIABLE_NAME, rVariable.Name());

            Matrix stress_DV_deriv;
            if(mStressTreatment == mean || mStressTreatment == GP)
                rAdjointElem.Calculate(STRESS_DESIGN_DERIVATIVE_ON_GP, stress_DV_deriv, rProcessInfo);
            else
                rAdjointElem.Calculate(STRESS_DESIGN_DERIVATIVE_ON_NODE, stress_DV_deriv, rProcessInfo);

            int num_of_DV = stress_DV_deriv.size1();
            int num_of_deriv = stress_DV_deriv.size2();
            double stress_DV_deriv_value = 0.0;

            if(rResponseGradient.size() != stress_DV_deriv.size1())
                rResponseGradient.resize(stress_DV_deriv.size1(), false);
            KRATOS_ERROR_IF(rResponseGradient.size() != rDerivativesMatrix.size1())
                << "Size of partial stress design variable derivative does not fit!" << std::endl;

            for (int dv_it = 0 ; dv_it < num_of_DV; dv_it++)
            {
                if(mStressTreatment == mean)
                {
                    for(int GP_it = 0; GP_it < num_of_deriv; GP_it++)
                        stress_DV_deriv_value += stress_DV_deriv(dv_it, GP_it);

                    stress_DV_deriv_value /= num_of_deriv;
                }
                else if(mStressTreatment == GP)
                {
                    if(num_of_deriv >= mIdOfLocation)
                        stress_DV_deriv_value = stress_DV_deriv(dv_it, (mIdOfLocation-1));
                    else
                        KRATOS_ERROR << "Chosen Gauss-Point is not availible. Chose 'stress_location' between 1 and " <<
                                    num_of_deriv  << "!"<< std::endl;
                }
                else if(mStressTreatment == node)
                {
                    if(num_of_deriv >= mIdOfLocation)
                        stress_DV_deriv_value = stress_DV_deriv(dv_it, (mIdOfLocation-1));
                    else
                        KRATOS_ERROR << "Chosen node is not availible. The element has only " <<
                                    num_of_deriv  << " nodes."<< std::endl;
                }
                rResponseGradient[dv_it] = stress_DV_deriv_value;
                stress_DV_deriv_value = 0.0;
            }

            rAdjointElem.SetValue(DESIGN_VARIABLE_NAME, "");
        }
        else
        {
            if (rResponseGradient.size() != rDerivativesMatrix.size1())
                      rResponseGradient.resize(rDerivativesMatrix.size1(), false);
            rResponseGradient.clear();
        }

        KRATOS_CATCH("");
    }

    // ==============================================================================
    void AdjointLocalStressResponseFunction::CalculateSensitivityGradient(Condition& rAdjointCondition,
                                      const Variable<array_1d<double,3>>& rVariable,
                                      const Matrix& rDerivativesMatrix,
                                      Vector& rResponseGradient,
                                      ProcessInfo& rProcessInfo)
    {
        KRATOS_TRY;

        if(rResponseGradient.size() != rDerivativesMatrix.size1())
              rResponseGradient.resize(rDerivativesMatrix.size1(), false);
        rResponseGradient.clear();

        KRATOS_CATCH("");
    }

} // namespace Kratos.

