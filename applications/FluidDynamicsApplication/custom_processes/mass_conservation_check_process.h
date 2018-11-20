//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    under supervision of Ruben Zorrilla
//
//

#ifndef KRATOS_MASS_CONSERVATION_CHECK_PROCESS_H
#define KRATOS_MASS_CONSERVATION_CHECK_PROCESS_H

// System includes
#include <string>
#include <iostream>

// External includes

// Project includes
#include "includes/define.h"
#include "processes/process.h"
#include "includes/variables.h"
#include "includes/cfd_variables.h"
#include "custom_elements/two_fluid_navier_stokes.h"
#include "custom_utilities/two_fluid_navier_stokes_data.h"
#include "modified_shape_functions/tetrahedra_3d_4_modified_shape_functions.h"
#include "modified_shape_functions/triangle_2d_3_modified_shape_functions.h"

// Application includes


namespace Kratos
{
///@addtogroup FluidDynamicsApplication
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

/// Utility to modify497 the distances of an embedded object in order to avoid bad intersections
/// Besides, it also deactivate the full negative distance elements

class KRATOS_API(FLUID_DYNAMICS_APPLICATION) MassConservationCheckProcess : public Process
{
public:
    ///@name Type Definitions
    ///@{

    /// Pointer definition of DistanceModificationProcess
    KRATOS_CLASS_POINTER_DEFINITION(MassConservationCheckProcess);

    typedef Node<3> NodeType;
    typedef Geometry<NodeType> GeometryType;

    ///@}
    ///@name Life Cycle
    ///@{

    /// Constructor.
    MassConservationCheckProcess(
        ModelPart& rModelPart, 
        const int MassComputationFreq,
        const bool CompareToInitial, 
        const bool WriteToLogFile);

    /// Constructor with Kratos parameters.
    MassConservationCheckProcess(
        ModelPart& rModelPart,
        Parameters& rParameters);

    /// Destructor.
    ~MassConservationCheckProcess() override {}

    ///@}
    ///@name Operators
    ///@{

    ///@}
    ///@name Operations
    ///@{

    ///@}
    ///@name Access
    ///@{

    void ExecuteInitialize() override
    {    };

    void ExecuteBeforeSolutionLoop() override
    {    };

    void ExecuteInitializeSolutionStep() override
    {    };

    void ExecuteFinalizeSolutionStep() override
    {    };

    /**
     * @brief Get the Update Status object to determine whether the volumes were recaculated
     * 
     * @return true indicates a recent recomputation of the volume fractions
     * @return false indicates outdated values of the volume fractions
     */
    bool GetUpdateStatus();

    /**
     * @brief Get the Positive Volume object to obtain the volume fraction with positive distance value
     * 
     * @return double volume
     */
    double GetPositiveVolume(){  
        return mCurrentPositiveVolume;
    };

    /**
     * @brief Get the Negative Volume object to obtain the volume fraction with negative distance value
     * 
     * @return double volume
     */
    double GetNegativeVolume(){  
        return mCurrentNegativeVolume;
    };
    /**
     * @brief Get the Initial Positive Volume object to obtain the positive volume fraction at the beginning
     * 
     * @return double volume
     */
    double GetInitialPositiveVolume(){  
        return mInitialPositiveVolume;
    };

    /**
     * @brief Get the Initial Negative Volume object to obtain the negative volume fraction at the beginning
     * 
     * @return double volume
     */
    double GetInitialNegativeVolume(){  
        return mInitialNegativeVolume;
    };

    // ///@}
    // ///@name Inquiry
    // ///@{

    // ///@}
    // ///@name Input and output
    // ///@{

    /// Turn back information as a string.
    std::string Info() const override
    {
        std::stringstream buffer;
        buffer << "MassConservationCheckProcess";
        return buffer.str();
    }

    /// Print information about this object.
    void PrintInfo(std::ostream& rOStream) const override {rOStream << "MassConservationCheckProcess";}

    /// Print object's data.
    void PrintData(std::ostream& rOStream) const override {}


    ///@}
    ///@name Friends
    ///@{

    ///@}

private:
    ///@name Static Member Variables
    ///@{

    ///@}
    ///@name Member Variables
    ///@{

    ModelPart& mrModelPart;

    int mMassComputationFreq;
    bool mCompareToInitial; 
    bool mWriteToLogFile;

    bool mIsUpdated;

    double mInitialNegativeVolume = 1.0;
    
    double mInitialPositiveVolume = 1.0;

    double mCurrentNegativeVolume = -1.0;

    double mCurrentPositiveVolume = -1.0;

    ///@}
    ///@name Protected Operators
    ///@{

    ///@}
    ///@name Private Operations
    ///@{

    /**
     * @brief Computes the fractions of the fluid domain with positive or negative values of the distance functions
     * 
     * @param positiveVolume volume ("Air) with positive distance function (output) 
     * @param negativeVolume volume ("Water") with negative distance function (output)
     */
    void ComputeVolumesOfFluids( double& positiveVolume, double& negativeVolume );

    ///@}
    ///@name Private  Access
    ///@{


    ///@}
    ///@name Private Inquiry
    ///@{


    ///@}
    ///@name Un accessible methods
    ///@{

    /// Default constructor.
    // DistanceModificationProcess() = delete;

    /// Assignment operator.
    // DistanceModificationProcess& operator=(DistanceModificationProcess const& rOther) = delete;

    /// Copy constructor.
    // DistanceModificationProcess(DistanceModificationProcess const& rOther) = delete;

    ///@}

}; // Class DistanceModificationProcess

///@}
///@name Type Definitions
///@{

///@}
///@name Input and output
///@{

///@}

///@} addtogroup block

};  // namespace Kratos.

#endif // KRATOS_MASS_CONSERVATION_CHECK_PROCESS_H
