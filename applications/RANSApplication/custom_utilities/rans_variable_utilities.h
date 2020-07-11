//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Suneth Warnakulasuriya (https://github.com/sunethwarna)
//
//

#if !defined(KRATOS_RANS_VARIABLE_UTILS)
#define KRATOS_RANS_VARIABLE_UTILS

/* System includes */

/* External includes */

/* Project includes */
#include "includes/kratos_flags.h"
#include "includes/model_part.h"

namespace Kratos
{
///@name Kratos Globals
///@{

namespace RansVariableUtilities
{
void KRATOS_API(RANS_APPLICATION)
    ClipScalarVariable(unsigned int& rNumberOfNodesBelowMinimum,
                       unsigned int& rNumberOfNodesAboveMaximum,
                       unsigned int& rNumberOfSelectedNodes,
                       const double MinimumValue,
                       const double MaximumValue,
                       const Variable<double>& rVariable,
                       ModelPart& rModelPart);

double KRATOS_API(RANS_APPLICATION)
    GetMinimumScalarValue(const ModelPart& rModelPart, const Variable<double>& rVariable);

double KRATOS_API(RANS_APPLICATION)
    GetMaximumScalarValue(const ModelPart& rModelPart, const Variable<double>& rVariable);

void KRATOS_API(RANS_APPLICATION)
    GetNodalVariablesVector(Vector& rValues,
                            const ModelPart::NodesContainerType& rNodes,
                            const Variable<double>& rVariable);

void KRATOS_API(RANS_APPLICATION) GetNodalArray(Vector& rNodalValues,
                                                const Element& rElement,
                                                const Variable<double>& rVariable);

void KRATOS_API(RANS_APPLICATION)
    SetNodalVariables(ModelPart::NodesContainerType& rNodes,
                      const Vector& rValues,
                      const Variable<double>& rVariable);

void KRATOS_API(RANS_APPLICATION)
    CopyNodalSolutionStepVariablesList(ModelPart& rOriginModelPart,
                                       ModelPart& rDestinationModelPart);

template <typename TDataType>
KRATOS_API(RANS_APPLICATION)
void AssignConditionVariableValuesToNodes(ModelPart& rModelPart,
                                          const Variable<TDataType>& rVariable,
                                          const Flags& rFlag,
                                          const bool FlagValue = true);

KRATOS_API(RANS_APPLICATION)
void AddAnalysisStep(ModelPart& rModelPart, const std::string& rStepName);

KRATOS_API(RANS_APPLICATION)
bool IsAnalysisStepCompleted(const ModelPart& rModelPart, const std::string& rStepName);

KRATOS_API(RANS_APPLICATION)
void AssignBoundaryFlagsToGeometries(ModelPart& rModelPart);

KRATOS_API(RANS_APPLICATION)
void FixFlaggedDofs(ModelPart& rModelPart,
                    const Variable<double>& rFixingVariable,
                    const Flags& rFlag,
                    const bool CheckValue = true);

template <typename TDataType>
KRATOS_API(RANS_APPLICATION)
void CopyNodalFlaggedVariableFromNonHistoricalToHistorical(ModelPart& rModelPart,
                                                           const Variable<TDataType>& rVariable,
                                                           const Flags& rFlag,
                                                           const bool CheckValue = true);

template <typename TDataType>
KRATOS_API(RANS_APPLICATION)
void CopyNodalFlaggedVariableFromHistoricalToNonHistorical(ModelPart& rModelPart,
                                                           const Variable<TDataType>& rVariable,
                                                           const Flags& rFlag,
                                                           const bool CheckValue = true);

KRATOS_API(RANS_APPLICATION)
void CalculateMagnitudeSquareForNodal3DVariable(ModelPart& rModelPart,
                                                const Variable<array_1d<double, 3>>& r3DVariable,
                                                const Variable<double>& rOutputVariable);

template <typename TDataType>
KRATOS_API(RANS_APPLICATION)
double GetVariableValueNorm(const TDataType& rValue);

template <typename TDataType>
KRATOS_API(RANS_APPLICATION)
std::tuple<double, double> CalculateTransientVariableConvergence(
    const ModelPart& rModelPart, const Variable<TDataType>& rVariable);

///@}
} // namespace RansVariableUtilities

} /* namespace Kratos.*/

#endif /* KRATOS_RANS_VARIABLE_UTILS  defined */