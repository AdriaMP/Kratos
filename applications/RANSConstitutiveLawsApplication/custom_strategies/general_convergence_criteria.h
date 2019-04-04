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

#ifndef KRATOS_GENERAL_CONVERGENCE_CRITERIA_H
#define KRATOS_GENERAL_CONVERGENCE_CRITERIA_H

/* Project includes */
#include "includes/define.h"
#include "includes/model_part.h"
#include "solving_strategies/convergencecriterias/convergence_criteria.h"
#include "utilities/openmp_utils.h"

namespace Kratos
{
///@addtogroup IncompressibleFluidApplication
///@{

///@name Kratos Classes
///@{

/// Convergence criteria for fluid problems.
/**
 This class implements a convergence control based on nodal velocity and
 pressure values. The error is evaluated separately for each of them, and
 relative and absolute tolerances for both must be specified.
 */
template <class TSparseSpace, class TDenseSpace>
class GeneralConvergenceCriteria : public ConvergenceCriteria<TSparseSpace, TDenseSpace>
{
public:
    ///@name Type Definitions
    ///@{

    KRATOS_CLASS_POINTER_DEFINITION(GeneralConvergenceCriteria);

    typedef ConvergenceCriteria<TSparseSpace, TDenseSpace> BaseType;

    typedef TSparseSpace SparseSpaceType;

    typedef typename BaseType::TDataType TDataType;

    typedef typename BaseType::DofsArrayType DofsArrayType;

    typedef typename BaseType::TSystemMatrixType TSystemMatrixType;

    typedef typename BaseType::TSystemVectorType TSystemVectorType;

    typedef OpenMPUtils::PartitionVector PartitionVector;

    typedef std::size_t KeyType;

    ///@}
    ///@name Life Cycle
    ///@{

    /// Constructor.
    /**
     * @param VelRatioTolerance Relative tolerance for velocity error
     * @param VelAbsTolerance Absolute tolerance for velocity error
     * @param PrsRatioTolerance Relative tolerance for presssure error
     * @param PrsAbsTolerance Absolute tolerance for presssure error
     */
    GeneralConvergenceCriteria(double rRatioTolerance, double rAbsTolerance)
        : ConvergenceCriteria<TSparseSpace, TDenseSpace>(),
          mRatioTolerance(rRatioTolerance),
          mAbsTolerance(rAbsTolerance)
    {
    }

    /// Destructor.
    ~GeneralConvergenceCriteria() override
    {
    }

    ///@}
    ///@name Operators
    ///@{

    /// Compute relative and absoute error.
    /**
     * @param rModelPart Reference to the ModelPart containing the fluid problem.
     * @param rDofSet Reference to the container of the problem's degrees of freedom (stored by the BuilderAndSolver)
     * @param A System matrix (unused)
     * @param Dx Vector of results (variations on nodal variables)
     * @param b RHS vector (residual)
     * @return true if convergence is achieved, false otherwise
     */
    bool PostCriteria(ModelPart& rModelPart,
                      DofsArrayType& rDofSet,
                      const TSystemMatrixType& A,
                      const TSystemVectorType& Dx,
                      const TSystemVectorType& b) override
    {
        if (SparseSpaceType::Size(Dx) != 0) // if we are solving for something
        {
            int NumDofs = rDofSet.size();

            // Initialize
            double solution_norm = 0.0;
            double increase_norm = 0.0;
            unsigned int dof_num = 0;

            std::string dof_name = rDofSet.begin()->GetVariable().Name();

            // Set a partition for OpenMP
            PartitionVector DofPartition;
            int NumThreads = OpenMPUtils::GetNumThreads();
            OpenMPUtils::DivideInPartitions(NumDofs, NumThreads, DofPartition);

            // Loop over Dofs
#pragma omp parallel reduction(+ : solution_norm, increase_norm, dof_num)
            {
                int k = OpenMPUtils::ThisThread();
                typename DofsArrayType::iterator DofBegin =
                    rDofSet.begin() + DofPartition[k];
                typename DofsArrayType::iterator DofEnd =
                    rDofSet.begin() + DofPartition[k + 1];

                std::size_t DofId;
                TDataType DofValue;
                TDataType DofIncr;

                for (typename DofsArrayType::iterator itDof = DofBegin;
                     itDof != DofEnd; ++itDof)
                {
                    if (itDof->IsFree())
                    {
                        DofId = itDof->EquationId();
                        DofValue = itDof->GetSolutionStepValue(0);
                        DofIncr = Dx[DofId];

                        solution_norm += DofValue * DofValue;
                        increase_norm += DofIncr * DofIncr;
                        dof_num += 1;
                    }
                }
            }

            if (solution_norm == 0.0)
                solution_norm = 1.0;

            const double ratio = increase_norm / solution_norm;
            const double ratio_abs = sqrt(increase_norm) / static_cast<int>(dof_num);

            const ProcessInfo& r_current_process_info = rModelPart.GetProcessInfo();
            const unsigned int iteration = r_current_process_info[NL_ITERATION_NUMBER];

            if (rModelPart.GetCommunicator().MyPID() == 0 && this->GetEchoLevel() > 0)
            {
                std::cout << "[" << iteration << "] CONVERGENCE CHECK: ";
                std::cout << dof_name;
                std::cout << ": ratio = " << std::scientific << ratio
                          << "; exp.ratio = " << std::scientific << mRatioTolerance;
                std::cout << ": abs = " << std::scientific << ratio_abs
                          << "; exp.abs = " << std::scientific << mAbsTolerance
                          << std::endl;
            }

            if ((std::abs(ratio) > mRatioTolerance) && (std::abs(ratio_abs) > mAbsTolerance))
                return false;

            if (rModelPart.GetCommunicator().MyPID() == 0 && this->GetEchoLevel() > 0)
            {
                std::cout << "CONVERGENCE CHECK: ";
                std::cout << dof_name;
                std::cout << ": *** CONVERGENCE IS ACHIEVED ***" << std::endl;
            }

            return true;
        }
        else // in this case all the displacements are imposed!
        {
            return true;
        }
    }

    /// Initialize this class before using it
    /**
     * @param rModelPart Reference to the ModelPart containing the fluid problem. (unused)
     */
    void Initialize(ModelPart& rModelPart) override
    {
        BaseType::mConvergenceCriteriaIsInitialized = true;
    }

    void InitializeSolutionStep(ModelPart& rModelPart,
                                DofsArrayType& rDofSet,
                                const TSystemMatrixType& A,
                                const TSystemVectorType& Dx,
                                const TSystemVectorType& b) override
    {
    }

    void FinalizeSolutionStep(ModelPart& rModelPart,
                              DofsArrayType& rDofSet,
                              const TSystemMatrixType& A,
                              const TSystemVectorType& Dx,
                              const TSystemVectorType& b) override
    {
    }

    ///@} // Operations

private:
    double mRatioTolerance;
    double mAbsTolerance;
}; // namespace Kratos

///@} // Kratos classes

///@} // Application group
} // namespace Kratos

#endif /* _VEL_PR_CRITERIA_H */
