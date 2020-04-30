//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Dharmin Shah (https://github.com/sdharmin)
//                   Bence Rochlitz (https://github.com/bencerochlitz)
//
//  Supervised by:   Jordi Cotela (https://github.com/jcotela)
//                   Suneth Warnakulasuriya (https://github.com/sunethwarna)




#if !defined(KRATOS_RANS_OMEGA_TURBULENT_MIXING_LENGTH_INLET_PROCESS_H_INCLUDED)
#define KRATOS_RANS_OMEGA_TURBULENT_MIXING_LENGTH_INLET_PROCESS_H_INCLUDED

// System includes
#include <string>

// External includes

// Project includes
#include "containers/model.h"
#include "processes/process.h"

namespace Kratos
{
///@addtogroup RANSModellingApplication
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

/**
 * @brief Sets omega value best on turbulent mixing length
 *
 * This process sets omega values based on the following formula
 *
 * \[
 *
 *  \omega = \frac{k^{0.5}}{C_\mu * L}
 *
 * \]
 *
 * In here $k$ is turbulent kinetic energy, $\omega$ is turbulent specific energy
 * dissipation rate, and $L$ is turbulent mixing length.
 *
 */

class KRATOS_API(RANS_APPLICATION) RansOmegaTurbulentMixingLengthInletProcess : public Process
{
public:
    ///@name Type Definitions
    ///@{

    using NodeType = ModelPart::NodeType;

    using NodesContainerType = ModelPart::NodesContainerType;

    /// Pointer definition of RansOmegaTurbulentMixingLengthInletProcess
    KRATOS_CLASS_POINTER_DEFINITION(RansOmegaTurbulentMixingLengthInletProcess);

    ///@}
    ///@name Life Cycle
    ///@{

    /// Constructor
	RansOmegaTurbulentMixingLengthInletProcess(Model& rModel, Parameters& rParameters);

    /// Destructor.
    ~RansOmegaTurbulentMixingLengthInletProcess() override;

    ///@}
    ///@name Operators
    ///@{

    ///@}
    ///@name Operations
    ///@{

    void ExecuteInitialize() override;

    void ExecuteInitializeSolutionStep() override;

    void Execute() override;

    int Check() override;

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
    Parameters& mrParameters;
    std::string mModelPartName;

    double mTurbulentMixingLength;
    double mCmu_75;
    bool mIsConstrained;
    int mEchoLevel;

    ///@}
    ///@name Private Operators
    ///@{

    ///@}
    ///@name Private Operations
    ///@{

    void CalculateTurbulentValues(NodeType& rNode);

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
	RansOmegaTurbulentMixingLengthInletProcess& operator=(
        RansOmegaTurbulentMixingLengthInletProcess const& rOther);

    /// Copy constructor.
	RansOmegaTurbulentMixingLengthInletProcess(RansOmegaTurbulentMixingLengthInletProcess const& rOther);

    ///@}

}; // Class RansOmegaTurbulentMixingLengthInletProcess

///@}

///@name Type Definitions
///@{

///@}
///@name Input and output
///@{

/// output stream function
inline std::ostream& operator<<(std::ostream& rOStream,
                                const RansOmegaTurbulentMixingLengthInletProcess& rThis);

///@}

///@} addtogroup block

} // namespace Kratos.

#endif // KRATOS_RANS_OMEGA_TURBULENT_MIXING_LENGTH_INLET_PROCESS_H_INCLUDED defined