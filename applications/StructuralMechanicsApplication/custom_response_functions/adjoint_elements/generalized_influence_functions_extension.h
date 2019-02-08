//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:   Martin Fusseder, https://github.com/MFusseder
//
//

#if !defined(KRATOS_GENERALIZED_INFLUENCE_FUNCTIONS_EXTENSION_INCLUDED)
#define  KRATOS_GENERALIZED_INFLUENCE_FUNCTIONS_EXTENSION_INCLUDED

// System includes
#include <iosfwd>
#include <vector>

// Project includes
#include "includes/define.h"
#include "includes/element.h"


namespace Kratos
{

///@name Kratos Classes
///@{

/**
 * @class GeneralizedInfluenceFunctionsExtension
 * @brief MFusseder add description
 */
class GeneralizedInfluenceFunctionsExtension
{
public:
    KRATOS_CLASS_POINTER_DEFINITION(GeneralizedInfluenceFunctionsExtension);

    typedef std::size_t IndexType;

    typedef std::size_t SizeType;

    GeneralizedInfluenceFunctionsExtension()
    {
    }

    GeneralizedInfluenceFunctionsExtension(Parameters AnalysisSettings);

    virtual ~GeneralizedInfluenceFunctionsExtension();

    void CalculatePseudoQuantityOnIntegrationPoints(Element& rElement, const Variable<array_1d<double, 3>>& rPseudoQuantityVariable,
                                            std::vector< array_1d<double, 3> >& rOutput, const ProcessInfo& rCurrentProcessInfo) const;


    void CalculateSensitivityOnIntegrationPoints(Element& rPrimalElement, Element& rAdjointElement, std::vector<double>& rOutput, const ProcessInfo& rCurrentProcessInfo) const;


private:
     std::string mDesignVariableName;
     double mDelta;

    friend class Serializer;

    void save(Serializer& rSerializer) const
    {
        rSerializer.save("DesignVariableName",mDesignVariableName);
        rSerializer.save("Delta",mDelta);
    }

    void load(Serializer& rSerializer)
    {
        rSerializer.load("DesignVariableName",mDesignVariableName);
        rSerializer.load("Delta",mDelta);
    }

};

///@} // Kratos Classes

}  // namespace Kratos.

#endif // KRATOS_GENERALIZED_INFLUENCE_FUNCTIONS_EXTENSION_INCLUDED  defined
