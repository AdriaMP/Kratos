//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ \.
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Ruben Zorrilla
//

#if !defined(KRATOS_TRILINOS_LEVELSET_FORWARD_CONVECTION_PROCESS_INCLUDED )
#define  KRATOS_TRILINOS_LEVELSET_FORWARD_CONVECTION_PROCESS_INCLUDED

// System includes

// External includes
#include "Epetra_MpiComm.h"

// Project includes
#include "containers/model.h"
#include "custom_strategies/builder_and_solvers/trilinos_block_builder_and_solver.h"
#include "processes/levelset_forward_convection_process.h"

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

/// Short class definition.
/**takes a model part full of SIMPLICIAL ELEMENTS (triangles and tetras) and convects a level set distance
 * on the top of it
*/
template< unsigned int TDim, class TSparseSpace, class TDenseSpace, class TLinearSolver >
class TrilinosLevelSetForwardConvectionProcess
    : public LevelSetForwardConvectionProcess<TDim, TSparseSpace, TDenseSpace, TLinearSolver>
{
public:

    KRATOS_DEFINE_LOCAL_FLAG(PERFORM_STEP1);
    KRATOS_DEFINE_LOCAL_FLAG(DO_EXPENSIVE_CHECKS);

    ///@name Type Definitions
    ///@{

    typedef LevelSetForwardConvectionProcess<TDim, TSparseSpace, TDenseSpace, TLinearSolver> BaseType;
    typedef typename TLinearSolver::Pointer LinearSolverPointerType;
    typedef typename BaseType::SchemeType::Pointer SchemePointerType;
    typedef typename BuilderAndSolver<TSparseSpace, TDenseSpace, TLinearSolver>::Pointer BuilderSolverPointerType;

    ///@}
    ///@name Pointer Definitions
    ///@{

    /// Pointer definition of TrilinosLevelSetForwardConvectionProcess
    KRATOS_CLASS_POINTER_DEFINITION(TrilinosLevelSetForwardConvectionProcess);

    ///@}
    ///@name Life Cycle
    ///@{

    /**
     */
    TrilinosLevelSetForwardConvectionProcess(
        Epetra_MpiComm& rEpetraCommunicator,
        Variable<double>& rLevelSetVar,
        ModelPart& rBaseModelPart,
        LinearSolverPointerType pLinearSolver,
        const double MaxCFL = 1.0,
        const double CrossWindStabilizationFactor = 0.7,
        const unsigned int MaxSubSteps = 0)
        : LevelSetForwardConvectionProcess<TDim, TSparseSpace, TDenseSpace, TLinearSolver>(
            rLevelSetVar,
            rBaseModelPart,
            MaxCFL,
            MaxSubSteps),
        mrEpetraCommunicator(rEpetraCommunicator)
    {

        KRATOS_TRY

        // Check that there is at least one element and node in the model
        int n_nodes = rBaseModelPart.NumberOfNodes();
        int n_elems = rBaseModelPart.NumberOfElements();

        if (n_nodes > 0){
            VariableUtils().CheckVariableExists< Variable< double > >(rLevelSetVar, rBaseModelPart.Nodes());
            VariableUtils().CheckVariableExists< Variable< array_1d < double, 3 > > >(VELOCITY, rBaseModelPart.Nodes());
        }

        if(TDim == 2){
            KRATOS_ERROR_IF(rBaseModelPart.ElementsBegin()->GetGeometry().GetGeometryFamily() != GeometryData::Kratos_Triangle) <<
                "In 2D the element type is expected to be a triangle" << std::endl;
        } else if(TDim == 3) {
            KRATOS_ERROR_IF(rBaseModelPart.ElementsBegin()->GetGeometry().GetGeometryFamily() != GeometryData::Kratos_Tetrahedra) <<
                "In 3D the element type is expected to be a tetrahedra" << std::endl;
        }

        rBaseModelPart.GetCommunicator().SumAll(n_nodes);
        rBaseModelPart.GetCommunicator().SumAll(n_elems);
        KRATOS_ERROR_IF(n_nodes == 0) << "The model has no nodes." << std::endl;
        KRATOS_ERROR_IF(n_elems == 0) << "The model has no elements." << std::endl;

        // Allocate if needed the variable DYNAMIC_TAU of the process info, and if it does not exist, set it to zero
        if( rBaseModelPart.GetProcessInfo().Has(DYNAMIC_TAU) == false){
            rBaseModelPart.GetProcessInfo().SetValue(DYNAMIC_TAU,0.0);
        }

        // Allocate if needed the variable CONVECTION_DIFFUSION_SETTINGS of the process info, and create it if it does not exist
        if(rBaseModelPart.GetProcessInfo().Has(CONVECTION_DIFFUSION_SETTINGS) == false){
            ConvectionDiffusionSettings::Pointer p_conv_diff_settings = Kratos::make_unique<ConvectionDiffusionSettings>();
            rBaseModelPart.GetProcessInfo().SetValue(CONVECTION_DIFFUSION_SETTINGS, p_conv_diff_settings);
            p_conv_diff_settings->SetUnknownVariable(rLevelSetVar);
            p_conv_diff_settings->SetConvectionVariable(VELOCITY);
        }

        // Generate an auxilary model part and populate it by elements of type DistanceCalculationElementSimplex
        (this->mDistancePartIsInitialized) = false;
        this->ReGenerateForwardConvectionModelPart(rBaseModelPart);

        // Generate a linear strategy
        SchemePointerType p_scheme = Kratos::make_shared< ResidualBasedIncrementalUpdateStaticScheme< TSparseSpace,TDenseSpace > >();

        const int row_size_guess = (TDim == 2 ? 15 : 40);
        BuilderSolverPointerType p_builder_and_solver = Kratos::make_shared< TrilinosBlockBuilderAndSolver< TSparseSpace,TDenseSpace,TLinearSolver > >(
            mrEpetraCommunicator,
            row_size_guess,
            pLinearSolver);

        const bool calculate_reactions = false;
        const bool reform_dof_at_each_iteration = false;
        const bool calculate_norm_dx_flag = false;

        (this->mpSolvingStrategy) = Kratos::make_unique< ResidualBasedLinearStrategy<TSparseSpace,TDenseSpace,TLinearSolver > >(
            *BaseType::mpDistanceAuxModelPart,
            p_scheme,
            pLinearSolver,
            p_builder_and_solver,
            calculate_reactions,
            reform_dof_at_each_iteration,
            calculate_norm_dx_flag);

        (this->mpSolvingStrategy)->SetEchoLevel(0);

        rBaseModelPart.GetProcessInfo().SetValue(CROSS_WIND_STABILIZATION_FACTOR, CrossWindStabilizationFactor);

        //TODO: check flag DO_EXPENSIVE_CHECKS
        (this->mpSolvingStrategy)->Check();

        KRATOS_CATCH("")
    }

    /// Destructor.
    ~TrilinosLevelSetForwardConvectionProcess() override {}

    ///@}
    ///@name Operators
    ///@{

    ///@}
    ///@name Operations
    ///@{

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
    std::string Info() const override {
        return "TrilinosLevelSetForwardConvectionProcess";
    }

    /// Print information about this object.
    void PrintInfo(std::ostream& rOStream) const override {
        rOStream << "TrilinosLevelSetForwardConvectionProcess";
    }

    // /// Print object's data.
    // void PrintData(std::ostream& rOStream) const override {
    // }

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

    Epetra_MpiComm& mrEpetraCommunicator;

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

    ///@}
    ///@name Private Operators
    ///@{

    ///@}
    ///@name Private Operations
    ///@{

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
    TrilinosLevelSetForwardConvectionProcess& operator=(TrilinosLevelSetForwardConvectionProcess const& rOther);

    /// Copy constructor.
    // TrilinosLevelSetForwardConvectionProcess(TrilinosLevelSetForwardConvectionProcess const& rOther);

    ///@}
}; // Class TrilinosLevelSetForwardConvectionProcess

// Avoiding using the macro since this has a template parameter. If there was no template plase use the KRATOS_CREATE_LOCAL_FLAG macro
template< unsigned int TDim, class TSparseSpace, class TDenseSpace, class TLinearSolver > const Kratos::Flags TrilinosLevelSetForwardConvectionProcess<TDim, TSparseSpace, TDenseSpace, TLinearSolver>::PERFORM_STEP1(Kratos::Flags::Create(0));
template< unsigned int TDim, class TSparseSpace, class TDenseSpace, class TLinearSolver > const Kratos::Flags TrilinosLevelSetForwardConvectionProcess<TDim, TSparseSpace, TDenseSpace, TLinearSolver>::DO_EXPENSIVE_CHECKS(Kratos::Flags::Create(1));

///@}
///@name Type Definitions
///@{

///@}
///@name Input and output
///@{

///@}
}  // namespace Kratos.

#endif // KRATOS_TRILINOS_LEVELSET_FORWARD_CONVECTION_PROCESS_INCLUDED  defined
