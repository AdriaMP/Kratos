//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Vicente Mataix Ferrandiz
//
//

#if !defined(KRATOS_STANDARD_CONVERGENCE_CRITERIA_FACTORY_H_INCLUDED )
#define  KRATOS_STANDARD_CONVERGENCE_CRITERIA_FACTORY_H_INCLUDED

// System includes

// External includes

// Project includes
#include "includes/define.h"
#include "factories/base_factory.h"

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

/**
 * @class StandardConvergenceCriteriaFactory
 * @ingroup KratosCore
 * @brief Here we add the functions needed for the registration of convergence criterias
 * @details Defines the standard convergence criteria factory
 * @author Vicente Mataix Ferrandiz
 * @tparam TConvergenceCriteriaType The convergence criteria type
 * @tparam TCustomConvergenceCriteriaType The convergence criteria type (derived class)
 */
template <typename TConvergenceCriteriaType, typename TCustomConvergenceCriteriaType>
class StandardConvergenceCriteriaFactory
    : public BaseFactory<TConvergenceCriteriaType>
{
public:
    ///@name Type Definitions
    ///@{

    /// The definition of the convergence criteria
    typedef TConvergenceCriteriaType ConvergenceCriteriaType;

    ///@}
    ///@name Operations
    ///@{

    /**
     * @brief This method returns the name of the class stored
     * @return The name of the class stored (as defined in settings)
     */
    std::string Name() const override
    {
        return TCustomConvergenceCriteriaType::Name();
    }

    /**
     * @brief This method is an auxiliar method to create a new convergence criteria
     * @return The pointer to the convergence criteria of interest
     */
    typename ConvergenceCriteriaType::Pointer CreateClass(Kratos::Parameters Settings) const override
    {
        return Kratos::make_shared<TCustomConvergenceCriteriaType>(Settings);
    }

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
        return "StandardConvergenceCriteriaFactory";
    }

    /// Print information about this object.
    void PrintInfo(std::ostream& rOStream) const override
    {
        rOStream << this->Info() << std::endl;
        const auto factory_components = KratosComponents<StandardConvergenceCriteriaFactory>::GetComponents();
        for (const auto& r_comp : factory_components) {
            rOStream << "\t" << r_comp.first << std::endl;
        }
    }

    /// Print object's data.
    void PrintData(std::ostream& rOStream) const override
    {
//         rOStream << this->Info() << std::endl;
//         const auto factory_components = KratosComponents<StandardConvergenceCriteriaFactory>::GetComponents();
//         for (const auto& r_comp : factory_components) {
//             rOStream << "\t" << r_comp.first << std::endl;
//         }
    }
};

///@}

///@name Type Definitions
///@{

///@}
///@name Input and output
///@{

/// output stream function
template <typename TConvergenceCriteriaType, typename TCustomConvergenceCriteriaType>
inline std::ostream& operator << (std::ostream& rOStream,
                                  const StandardConvergenceCriteriaFactory<TConvergenceCriteriaType, TCustomConvergenceCriteriaType>& rThis)
{
    rThis.PrintInfo(rOStream);
    rOStream << std::endl;
    rThis.PrintData(rOStream);

    return rOStream;
}

///@}
///@name Input and output

void RegisterConvergenceCriterias();

///@}

}  // namespace Kratos.

#endif // KRATOS_STANDARD_CONVERGENCE_CRITERIA_FACTORY_H_INCLUDED  defined
