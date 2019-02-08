//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Martin Fusseder, https://github.com/MFusseder
//

#if !defined(KRATOS_ELEMENT_FINITE_DIFFERENCE_UTILITY_H_INCLUDED )
#define  KRATOS_ELEMENT_FINITE_DIFFERENCE_UTILITY_H_INCLUDED


// System includes

// External includes

// Project includes
#include "includes/define.h"
#include "includes/element.h"
#include "includes/condition.h"
#include "structural_mechanics_application_variables.h"

namespace Kratos
{

/** \brief ElementFiniteDifferenceUtility
 *
 * This class calculates the derivatives of different element quantities (e.g. RHS, LHS, mass-matrix, ...)
 * with respect to a design variable (e.g. nodal-coordinate, property).
 */


class KRATOS_API(STRUCTURAL_MECHANICS_APPLICATION) ElementFiniteDifferenceUtility
{
public:

    typedef VariableComponent<VectorComponentAdaptor<array_1d<double, 3> > > array_1d_component_type;
    typedef std::size_t IndexType;
    typedef std::size_t SizeType;

    static void CalculateRightHandSideDerivative(Element& rElement,
                                                const Vector& rRHS,
                                                const Variable<double>& rDesignVariable,
                                                const double& rPertubationSize,
                                                Matrix& rOutput,
                                                ProcessInfo& rCurrentProcessInfo);

    static void CalculateRightHandSideDerivative(Element& rElement,
                                                const Vector& rRHS,
                                                const array_1d_component_type& rDesignVariable,
                                                Node<3>& rNode,
                                                const double& rPertubationSize,
                                                Vector& rOutput,
                                                ProcessInfo& rCurrentProcessInfo);

    static void CalculateLeftHandSideDerivative(Element& rElement,
                                                const Matrix& rLHS,
                                                const array_1d_component_type& rDesignVariable,
                                                Node<3>& rNode,
                                                const double& rPertubationSize,
                                                Matrix& rOutput,
                                                ProcessInfo& rCurrentProcessInfo);

    static void CalculateMassMatrixDerivative(Element& rElement,
                                                const Matrix& rMassMatrix,
                                                const array_1d_component_type& rDesignVariable,
                                                Node<3>& rNode,
                                                const double& rPertubationSize,
                                                Matrix& rOutput,
                                                ProcessInfo& rCurrentProcessInfo);




    template <typename TDataType>
    static void CalculateIntegrationPointsResultsDerivative(Element& rElement,
                                                const Variable<TDataType>& rQuantityOfInterestVariable,
                                                const Variable<double>& rDesignVariable,
                                                const double& rPertubationSize,
                                                std::vector< TDataType >& rOutput,
                                                const ProcessInfo& rCurrentProcessInfo)
    {
        KRATOS_TRY;

        std::vector< TDataType > undisturbed_values;

        // Compute undisturbed quantity of interest
        rElement.CalculateOnIntegrationPoints(rQuantityOfInterestVariable, undisturbed_values, rCurrentProcessInfo);

        if( rElement.GetProperties().Has(rDesignVariable) )
        {
            // Save property pointer
            Properties::Pointer p_global_properties = rElement.pGetProperties();

            // Create new property and assign it to the element
            Properties::Pointer p_local_property(Kratos::make_shared<Properties>(Properties(*p_global_properties)));
            rElement.SetProperties(p_local_property);

            // Perturb the design variable
            const double current_property_value = rElement.GetProperties()[rDesignVariable];
            p_local_property->SetValue(rDesignVariable, (current_property_value + rPertubationSize));

            // Compute quantity of interest after disturbance of design variable
            rElement.CalculateOnIntegrationPoints(rQuantityOfInterestVariable, rOutput, rCurrentProcessInfo);

            // Compute derivative of stress w.r.t. design variable with finite differences
            for(IndexType i = 0; i < rOutput.size(); ++i)
                ElementFiniteDifferenceUtility::ComputeFiniteDifferences(undisturbed_values[i], rOutput[i], rPertubationSize);

            // Give element original properties back
            rElement.SetProperties(p_global_properties);
        }
        else
            rOutput.clear();

        KRATOS_CATCH("")
    }

private:

    static std::size_t GetCoordinateDirection(const array_1d_component_type& rDesignVariable);

    static void ComputeFiniteDifferences( array_1d<double, 3 >& rArrayUndisturbed, array_1d<double, 3 >& rArrayDisturbed, const double& rDelta);


}; // class ElementFiniteDifferenceUtility.



}  // namespace Kratos.

#endif // KRATOS_ELEMENT_FINITE_DIFFERENCE_UTILITY_H_INCLUDED  defined


