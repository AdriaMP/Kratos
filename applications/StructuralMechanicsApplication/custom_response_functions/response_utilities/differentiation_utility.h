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

#if !defined(KRATOS_DIFFERENTIATION_UTILITY_H_INCLUDED )
#define  KRATOS_DIFFERENTIATION_UTILITY_H_INCLUDED


// System includes

// External includes

// Project includes
#include "includes/define.h"
#include "includes/element.h"
#include "includes/condition.h"

namespace Kratos
{

/** \brief DifferentiationUtility
 *
 * This class calculates comnputes the derivatives of different element and condition entities
 * (e.g. RHS, LHS, mass-matrix, ...) with respect to a design variable (e.g. nodal-coordinate, property).
 */
class DifferentiationUtility
{
public:

    typedef std::size_t IndexType;
    typedef std::size_t SizeType;

    static void CalculateRigthHandSideDerivative(Element& rElement,
                                                const Variable<double>& rDesignVariable,
                                                Matrix& rOutput,
                                                const ProcessInfo& rCurrentProcessInfo);

    static void CalculateRigthHandSideDerivative(Element& rElement,
                                                const Variable<array_1d<double,3>>& rDesignVariable,
                                                Matrix& rOutput,
                                                const ProcessInfo& rCurrentProcessInfo);

private:

    static double GetPerturbationSize(const Variable<double>& rDesignVariable);

    static double GetPerturbationSize(const Variable<array_1d<double,3>>& rDesignVariable);

}; // class DifferentiationUtility.



}  // namespace Kratos.

#endif // KRATOS_DIFFERENTIATION_UTILITY_H_INCLUDED  defined


