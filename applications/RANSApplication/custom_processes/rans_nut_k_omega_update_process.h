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

#if !defined(KRATOS_RANS_NUT_K_OMEGA_UPDATE_PROCESS_H_INCLUDED)
#define KRATOS_RANS_NUT_K_OMEGA_UPDATE_PROCESS_H_INCLUDED

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

///@name Kratos Classes
///@{

/**
 * @brief Calculates turbulent kinematic viscosity
 *
 * This process uses following formula to calculate turbulent kinematic viscosity
 *
 * \[
 *      \nu_t = C_\mu\frac{k^2}{\epsilon}
 * \]
 *
 * $k$ is the turbulent kinetic energy, $\epsilon$ is the turbulent energy dissipation rate
 */

class KRATOS_API(RANS_APPLICATION) RansNutKOmegaUpdateProcess : public Process
{
public:
    ///@name Type Definitions
    ///@{

    using NodeType = ModelPart::NodeType;

    using NodesContainerType = ModelPart::NodesContainerType;

    /// Pointer definition of RansNutKOmegaUpdateProcess
    KRATOS_CLASS_POINTER_DEFINITION(RansNutKOmegaUpdateProcess);

    ///@}
    ///@name Life Cycle
    ///@{

    /// Constructor

    RansNutKOmegaUpdateProcess(Model& rModel, Parameters rParameters);

    RansNutKOmegaUpdateProcess(Model& rModel,
                               const std::string& rModelPartName,
                               const double MinValue,
                               const int EchoLevel);

    /// Destructor.
    ~RansNutKOmegaUpdateProcess() override = default;

    ///@}
    ///@name Operations
    ///@{

    int Check() override;

    void ExecuteInitializeSolutionStep() override;

    void Execute() override;

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

private:
    ///@name Member Variables
    ///@{

    Model& mrModel;
    std::string mModelPartName;
    double mMinValue;
    int mEchoLevel;
    bool mIsInitialized = false;

    ///@}
    ///@name Un accessible methods
    ///@{

    /// Assignment operator.
    RansNutKOmegaUpdateProcess& operator=(RansNutKOmegaUpdateProcess const& rOther);

    /// Copy constructor.
    RansNutKOmegaUpdateProcess(RansNutKOmegaUpdateProcess const& rOther);

    ///@}

}; // Class RansNutKOmegaUpdateProcess

///@}
///@name Input and output
///@{

/// output stream function
inline std::ostream& operator<<(std::ostream& rOStream,
                                const RansNutKOmegaUpdateProcess& rThis);

///@}

///@} addtogroup block

} // namespace Kratos.

#endif // KRATOS_RANS_NUT_K_OMEGA_UPDATE_PROCESS_H_INCLUDED defined
