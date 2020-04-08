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

#if !defined(KRATOS_RANS_EPSILON_Y_PLUS_WALL_FUNCTION_PROCESS_H_INCLUDED)
#define KRATOS_RANS_EPSILON_Y_PLUS_WALL_FUNCTION_PROCESS_H_INCLUDED

// System includes
#include <string>

// External includes

// Project includes
#include "containers/model.h"
#include "processes/process.h"

namespace Kratos
{
///@addtogroup RANSApplication
///@{

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

class KRATOS_API(RANS_APPLICATION) RansEpsilonYPlusWallFunctionProcess : public Process
{
public:
    ///@name Type Definitions
    ///@{

    using NodeType = ModelPart::NodeType;
    using ConditionType = ModelPart::ConditionType;
    using ConditionGeometryType = ModelPart::ConditionType::GeometryType;

    /// Pointer definition of RansEpsilonYPlusWallFunctionProcess
    KRATOS_CLASS_POINTER_DEFINITION(RansEpsilonYPlusWallFunctionProcess);

    ///@}
    ///@name Life Cycle
    ///@{

    /// Constructor

    RansEpsilonYPlusWallFunctionProcess(Model& rModel, Parameters rParameters);

    /// Destructor.
    ~RansEpsilonYPlusWallFunctionProcess() override = default;

    ///@}
    ///@name Operators
    ///@{

    ///@}
    ///@name Operations
    ///@{

    int Check() override;

    void ExecuteInitialize() override;

    void Execute() override;

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
    std::string Info() const override;

    /// Print information about this object.
    void PrintInfo(std::ostream& rOStream) const override;

    /// Print object's data.
    void PrintData(std::ostream& rOStream) const override;

    ///@}
    ///@name Friends
    ///@{

    ///@}

protected:
    ///@name Protected static Member Variables
    ///@{

    ///@}
    ///@name Protected member Variables
    ///@{

    ///@}
    ///@name Protected Operators
    ///@{

    ///@}
    ///@name Protected Operations
    ///@{

    ///@}
    ///@name Protected  Access
    ///@{

    ///@}
    ///@name Protected Inquiry
    ///@{

    ///@}
    ///@name Protected LifeCycle
    ///@{

    ///@}

private:
    ///@name Static Member Variables
    ///@{

    ///@}
    ///@name Member Variables
    ///@{

    Model& mrModel;
    Parameters mrParameters;
    std::string mModelPartName;

    int mEchoLevel;

    double mLimitYPlus;
    double mMinValue;

    double mCmu;
    double mVonKarman;
    double mBeta;

    ///@}
    ///@name Private Operators
    ///@{

    ///@}
    ///@name Private Operations
    ///@{

    void CalculateConditionNeighbourCount();

    ///@}
    ///@name Private  Access
    ///@{

    ///@}
    ///@name Private Inquiry
    ///@{

    ///@}
    ///@name Un accessible methods
    ///@{

    /// Assignment operator.
    RansEpsilonYPlusWallFunctionProcess& operator=(RansEpsilonYPlusWallFunctionProcess const& rOther);

    /// Copy constructor.
    RansEpsilonYPlusWallFunctionProcess(RansEpsilonYPlusWallFunctionProcess const& rOther);

    ///@}

}; // Class RansEpsilonYPlusWallFunctionProcess

///@}

///@name Type Definitions
///@{

///@}
///@name Input and output
///@{

/// output stream function
inline std::ostream& operator<<(std::ostream& rOStream,
                                const RansEpsilonYPlusWallFunctionProcess& rThis);

///@}

///@} addtogroup block

} // namespace Kratos.

#endif // KRATOS_RANS_EPSILON_Y_PLUS_WALL_FUNCTION_PROCESS_H_INCLUDED defined
