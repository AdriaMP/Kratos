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

#if !defined(KRATOS_STANDARD_SCHEME_FACTORY_H_INCLUDED )
#define  KRATOS_STANDARD_SCHEME_FACTORY_H_INCLUDED

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
 * @class StandardSchemeFactory
 * @ingroup KratosCore
 * @brief Here we add the functions needed for the registration of convergence criterias
 * @details Defines the standard convergence criteria factory
 * @author Vicente Mataix Ferrandiz
 * @tparam TSchemeType The convergence criteria type
 * @tparam TCustomSchemeType The convergence criteria type (derived class)
 */
template <typename TSchemeType, typename TCustomSchemeType>
class StandardSchemeFactory
    : public BaseFactory<TSchemeType>
{
public:
    ///@name Type Definitions
    ///@{

    /// The definition of the scheme
    typedef TSchemeType SchemeType;

    ///@}
    ///@name Operations
    ///@{

    /**
     * @brief This method returns the name of the class stored
     * @return The name of the class stored (as defined in settings)
     */
    std::string Name() const override
    {
        return TCustomSchemeType::Name();
    }

    /**
     * @brief This method is an auxiliar method to create a new convergence criteria
     * @return The pointer to the convergence criteria of interest
     */
    typename SchemeType::Pointer CreateClass(Kratos::Parameters Settings) const override
    {
        return Kratos::make_shared<TCustomSchemeType>(Settings);
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
        return "StandardSchemeFactory";
    }

    /// Print information about this object.
    void PrintInfo(std::ostream& rOStream) const override
    {
        rOStream << this->Info() << std::endl;
        const auto factory_components = KratosComponents<StandardSchemeFactory>::GetComponents();
        for (const auto& r_comp : factory_components) {
            rOStream << "\t" << r_comp.first << std::endl;
        }
    }

    /// Print object's data.
    void PrintData(std::ostream& rOStream) const override
    {
//         rOStream << this->Info() << std::endl;
//         const auto factory_components = KratosComponents<StandardSchemeFactory>::GetComponents();
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
template <typename TSchemeType, typename TCustomSchemeType>
inline std::ostream& operator << (std::ostream& rOStream,
                                  const StandardSchemeFactory<TSchemeType, TCustomSchemeType>& rThis)
{
    rThis.PrintInfo(rOStream);
    rOStream << std::endl;
    rThis.PrintData(rOStream);

    return rOStream;
}

///@}
///@name Input and output

void RegisterSchemes();

///@}

}  // namespace Kratos.

#endif // KRATOS_STANDARD_SCHEME_FACTORY_H_INCLUDED  defined
