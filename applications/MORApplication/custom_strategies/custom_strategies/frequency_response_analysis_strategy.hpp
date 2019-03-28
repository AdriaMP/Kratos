//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ \.
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Riccardo Rossi
//

#if !defined(FREQUENCY_RESPONSE_ANALYSIS_STRATEGY)
#define FREQUENCY_RESPONSE_ANALYSIS_STRATEGY

// System includes

// External includes

// Project includes
#include "includes/define.h"
#include "solving_strategies/strategies/solving_strategy.h"
#include "solving_strategies/convergencecriterias/convergence_criteria.h"
#include "utilities/builtin_timer.h"
#include "utilities/qr_utility.h"

//default builder and solver
#include "custom_strategies/custom_builder_and_solvers/system_matrix_builder_and_solver.hpp"

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
 * @class FrequencyResponseAnalysisStrategy
 * @ingroup KratosCore
 * @brief This is the linear MOR matrix output strategy
 * @details This strategy builds the K and M matrices and outputs them
 * @author Srikkanth Varadharajan
 */
template <class TSparseSpace,
          class TDenseSpace,  // = DenseSpace<double>,
          class TLinearSolver //= LinearSolver<TSparseSpace,TDenseSpace>
          >
class FrequencyResponseAnalysisStrategy
    : public SolvingStrategy<TSparseSpace, TDenseSpace, TLinearSolver>
{
  public:
    ///@name Type Definitions
    ///@
    // Counted pointer of ClassName
    KRATOS_CLASS_POINTER_DEFINITION(FrequencyResponseAnalysisStrategy);

    typedef SolvingStrategy<TSparseSpace, TDenseSpace, TLinearSolver> BaseType;

    typedef SystemMatrixBuilderAndSolver< TSparseSpace, TDenseSpace, TLinearSolver > TBuilderAndSolverType;

    typedef typename BaseType::TDataType TDataType;

    typedef TSparseSpace SparseSpaceType;

    typedef TDenseSpace DenseSpaceType;

    typedef typename TDenseSpace::MatrixType TDenseMatrixType;

    typedef typename TDenseSpace::MatrixPointerType TDenseMatrixPointerType;

    typedef typename BaseType::TSchemeType TSchemeType;

    typedef typename BaseType::DofsArrayType DofsArrayType;

    typedef typename BaseType::TSystemMatrixType TSystemMatrixType;

    typedef typename BaseType::TSystemVectorType TSystemVectorType;

    typedef typename BaseType::LocalSystemVectorType LocalSystemVectorType;

    typedef typename BaseType::LocalSystemMatrixType LocalSystemMatrixType;

    typedef typename BaseType::TSystemMatrixPointerType TSystemMatrixPointerType;

    typedef typename BaseType::TSystemVectorPointerType TSystemVectorPointerType;

    typedef std::complex<double> ComplexType;

    typedef DenseVector<ComplexType> ComplexVectorType;

    typedef DenseMatrix<ComplexType> ComplexMatrixType;

    ///@}
    ///@name Life Cycle

    ///@{

    /**
     * Default constructor
     * @param rModelPart The model part of the problem
     * @param pScheme The integration schemed
     * @param MoveMeshFlag The flag that allows to move the mesh
     */
    FrequencyResponseAnalysisStrategy(
        ModelPart& rModelPart,
        typename TSchemeType::Pointer pScheme,
        typename TLinearSolver::Pointer pNewLinearSolver,
        bool MoveMeshFlag = false)
        : SolvingStrategy<TSparseSpace, TDenseSpace, TLinearSolver>(rModelPart, MoveMeshFlag)
    {
        KRATOS_TRY;

        // Saving the scheme
        mpScheme = pScheme;

        // Setting up the default builder and solver
         mpBuilderAndSolver = typename TBuilderAndSolverType::Pointer(
            new TBuilderAndSolverType(pNewLinearSolver)); 

        // Saving the linear solver
        mpLinearSolver = pNewLinearSolver;            

        // Set flags to start correcty the calculations
        mSolutionStepIsInitialized = false;
        mInitializeWasPerformed = false;

        // Tells to the builder and solver if the reactions have to be Calculated or not
        GetBuilderAndSolver()->SetCalculateReactionsFlag(false);

        // Tells to the Builder And Solver if the system matrix and vectors need to
        // be reshaped at each step or not
        // GetBuilderAndSolver()->SetReshapeMatrixFlag(mReformDofSetAtEachStep);

        // Set EchoLevel to the default value (only time is displayed)
        SetEchoLevel(1);

        // By default the matrices are rebuilt at each iteration
        this->SetRebuildLevel(0);

        mpA = TSparseSpace::CreateEmptyMatrixPointer();
        mpS = TSparseSpace::CreateEmptyMatrixPointer();
        mpM = TSparseSpace::CreateEmptyMatrixPointer();
        mpRHS = TSparseSpace::CreateEmptyVectorPointer();

        KRATOS_CATCH("");
    }

    /**
     * @brief Destructor.
     * @details In trilinos third party library, the linear solver's preconditioner should be freed before the system matrix. We control the deallocation order with Clear().
     */
    ~FrequencyResponseAnalysisStrategy() override
    {
        Clear();
    }

    /**
     * @brief Set method for the time scheme
     * @param pScheme The pointer to the time scheme considered
     */
    void SetScheme(typename TSchemeType::Pointer pScheme)
    {
        mpScheme = pScheme;
    };

    /**
     * @brief Get method for the time scheme
     * @return mpScheme: The pointer to the time scheme considered
     */
    typename TSchemeType::Pointer GetScheme()
    {
        return mpScheme;
    };

    /**
     * @brief Set method for the builder and solver
     * @param pNewBuilderAndSolver The pointer to the builder and solver considered
     */
    void SetBuilderAndSolver(typename TBuilderAndSolverType::Pointer pNewBuilderAndSolver)
    {
        mpBuilderAndSolver = pNewBuilderAndSolver;
    };

    /**
     * @brief Get method for the builder and solver
     * @return mpBuilderAndSolver: The pointer to the builder and solver considered
     */
    typename TBuilderAndSolverType::Pointer GetBuilderAndSolver()
    {
        return mpBuilderAndSolver;
    };

    /**
     * @brief This method sets the flag mInitializeWasPerformed
     * @param InitializePerformedFlag The flag that tells if the initialize has been computed
     */
    void SetInitializePerformedFlag(bool InitializePerformedFlag = true)
    {
        mInitializeWasPerformed = InitializePerformedFlag;
    }

    /**
     * @brief This method gets the flag mInitializeWasPerformed
     * @return mInitializeWasPerformed: The flag that tells if the initialize has been computed
     */
    bool GetInitializePerformedFlag()
    {
        return mInitializeWasPerformed;
    }

    /**
     * @brief This method sets the flag mReformDofSetAtEachStep
     * @param Flag The flag that tells if each time step the system is rebuilt
     */
    void SetReformDofSetAtEachStepFlag(bool Flag)
    {
        mReformDofSetAtEachStep = Flag;
        GetBuilderAndSolver()->SetReshapeMatrixFlag(mReformDofSetAtEachStep);
    }

    /**
     * @brief This method returns the flag mReformDofSetAtEachStep
     * @return The flag that tells if each time step the system is rebuilt
     */
    bool GetReformDofSetAtEachStepFlag()
    {
        return mReformDofSetAtEachStep;
    }

    /**
     * @brief It sets the level of echo for the solving strategy
     * @param Level The level to set
     * @details The different levels of echo are:
     * - 0: Mute... no echo at all
     * - 1: Printing time and basic informations
     * - 2: Printing linear solver data
     * - 3: Print of debug informations: Echo of stiffness matrix, Dx, b...
     */

    void SetEchoLevel(int Level) override
    {
        BaseType::mEchoLevel = Level;
        GetBuilderAndSolver()->SetEchoLevel(Level);
    }

    /**
     * @brief Initialization of member variables and prior operations
     */
    void Initialize() override
    {
        KRATOS_TRY;

        if (mInitializeWasPerformed == false)
        {

            std::cout << "This is the frequency response strategy !!" << std::endl;
            //pointers needed in the solution
            typename TSchemeType::Pointer p_scheme = GetScheme();

            //Initialize The Scheme - OPERATIONS TO BE DONE ONCE
            if (p_scheme->SchemeIsInitialized() == false)
                p_scheme->Initialize(BaseType::GetModelPart());

            //Initialize The Elements - OPERATIONS TO BE DONE ONCE
            if (p_scheme->ElementsAreInitialized() == false)
                p_scheme->InitializeElements(BaseType::GetModelPart());

            //Initialize The Conditions - OPERATIONS TO BE DONE ONCE
            if (p_scheme->ConditionsAreInitialized() == false)
                p_scheme->InitializeConditions(BaseType::GetModelPart());

            mInitializeWasPerformed = true;
        }

        

        KRATOS_CATCH("");
    }

    /**
     * @brief Clears the internal storage
     */
    void Clear() override
    {
        KRATOS_TRY;

        // if the preconditioner is saved between solves, it
        // should be cleared here.
        GetBuilderAndSolver()->GetLinearSystemSolver()->Clear();

        if (mpA != nullptr)
            SparseSpaceType::Clear(mpA);
        if (mpM != nullptr)
            SparseSpaceType::Clear(mpM);
        if (mpRHS != nullptr)
            SparseSpaceType::Clear(mpRHS);
        if (mpS != nullptr)
            SparseSpaceType::Clear(mpRHS);

        //setting to zero the internal flag to ensure that the dof sets are recalculated
        GetBuilderAndSolver()->SetDofSetIsInitializedFlag(false);
        GetBuilderAndSolver()->Clear();
        GetScheme()->Clear();

        mInitializeWasPerformed = false;

        KRATOS_CATCH("");
    }


    /**
     * @brief Performs all the required operations that should be done (for each step) before solving the solution step.
     * @details A member variable should be used as a flag to make sure this function is called only once per step.
     */
    void InitializeSolutionStep() override
    {
        KRATOS_TRY;

        if (mSolutionStepIsInitialized == false)
        {
            
            

            typename TSchemeType::Pointer p_scheme = GetScheme();
            typename TBuilderAndSolverType::Pointer p_builder_and_solver = GetBuilderAndSolver();

            const int rank = BaseType::GetModelPart().GetCommunicator().MyPID();
            //set up the system, operation performed just once unless it is required
            //to reform the dof set at each iteration
            BuiltinTimer system_construction_time;
            if (p_builder_and_solver->GetDofSetIsInitializedFlag() == false ||
                mReformDofSetAtEachStep == true)
            {
                //setting up the list of the DOFs to be solved
                BuiltinTimer setup_dofs_time;
                p_builder_and_solver->SetUpDofSet(p_scheme, BaseType::GetModelPart());
                KRATOS_INFO_IF("Setup Dofs Time", BaseType::GetEchoLevel() > 0 && rank == 0)
                    << setup_dofs_time.ElapsedSeconds() << std::endl;

                //shaping correctly the system
                BuiltinTimer setup_system_time;
                p_builder_and_solver->SetUpSystem(BaseType::GetModelPart());
                KRATOS_INFO_IF("Setup System Time", BaseType::GetEchoLevel() > 0 && rank == 0)
                    << setup_system_time.ElapsedSeconds() << std::endl;

                //setting up the Vectors involved to the correct size
                BuiltinTimer system_matrix_resize_time;
                p_builder_and_solver->ResizeAndInitializeVectors(p_scheme, mpA, mpRHS, mpRHS,
                                                                 BaseType::GetModelPart());
                p_builder_and_solver->ResizeAndInitializeVectors(p_scheme, mpM, mpRHS, mpRHS,
                                                                 BaseType::GetModelPart()); 
                p_builder_and_solver->ResizeAndInitializeVectors(p_scheme, mpS, mpRHS, mpRHS,
                                                                 BaseType::GetModelPart()); 

                KRATOS_INFO_IF("System Matrix Resize Time", BaseType::GetEchoLevel() > 0 && rank == 0)
                    << system_matrix_resize_time.ElapsedSeconds() << std::endl;
            }

            KRATOS_INFO_IF("System Construction Time", BaseType::GetEchoLevel() > 0 && rank == 0)
                << system_construction_time.ElapsedSeconds() << std::endl;

            TSystemMatrixType& rA  = *mpA;
            TSystemMatrixType& rM = *mpM;
            TSystemMatrixType& rS  = *mpS;
            TSystemVectorType& rRHS  = *mpRHS;

            //initial operations ... things that are constant over the Solution Step
            p_builder_and_solver->InitializeSolutionStep(BaseType::GetModelPart(), rA, rRHS, rRHS);
            p_builder_and_solver->InitializeSolutionStep(BaseType::GetModelPart(), rM, rRHS, rRHS);
            p_builder_and_solver->InitializeSolutionStep(BaseType::GetModelPart(), rS, rRHS, rRHS);

            //initial operations ... things that are constant over the Solution Step
            p_scheme->InitializeSolutionStep(BaseType::GetModelPart(), rA, rRHS, rRHS);
            p_scheme->InitializeSolutionStep(BaseType::GetModelPart(), rM, rRHS, rRHS);
            p_scheme->InitializeSolutionStep(BaseType::GetModelPart(), rS, rRHS, rRHS);

            mSolutionStepIsInitialized = true;
        }

        KRATOS_CATCH("");
    }

    /**
     * @brief Performs all the required operations that should be done (for each step) after solving the solution step.
     * @details A member variable should be used as a flag to make sure this function is called only once per step.
     */
    void FinalizeSolutionStep() override
    {
        KRATOS_TRY;

        typename TSchemeType::Pointer p_scheme = GetScheme();
        typename TBuilderAndSolverType::Pointer p_builder_and_solver = GetBuilderAndSolver();

        TSystemMatrixType& rA  = *mpA;
        TSystemMatrixType& rM = *mpM;
        TSystemVectorType& rRHS  = *mpRHS;
        TSystemMatrixType& rS  = *mpS;

        //Finalisation of the solution step,
        //operations to be done after achieving convergence, for example the
        //Final Residual Vector (mb) has to be saved in there
        //to avoid error accumulation

        p_scheme->FinalizeSolutionStep(BaseType::GetModelPart(), rA, rRHS, rRHS);
        p_scheme->FinalizeSolutionStep(BaseType::GetModelPart(), rM, rRHS, rRHS);
        p_builder_and_solver->FinalizeSolutionStep(BaseType::GetModelPart(), rA, rRHS, rRHS);
        p_builder_and_solver->FinalizeSolutionStep(BaseType::GetModelPart(), rM, rRHS, rRHS);

        p_scheme->FinalizeSolutionStep(BaseType::GetModelPart(), rS, rRHS, rRHS);
        p_builder_and_solver->FinalizeSolutionStep(BaseType::GetModelPart(), rS, rRHS, rRHS);

        //Cleaning memory after the solution
        p_scheme->Clean();

        //reset flags for next step
        mSolutionStepIsInitialized = false;

        if (mReformDofSetAtEachStep == true) //deallocate the systemvectors
        {
            SparseSpaceType::Clear(mpA);
            SparseSpaceType::Clear(mpRHS);
            SparseSpaceType::Clear(mpM);
            SparseSpaceType::Clear(mpS);

            this->Clear();
        }

        KRATOS_CATCH("");
    }

    void AssignVariables(TSystemVectorType& rDisplacement, int step=0)
    {
        auto& r_model_part = BaseType::GetModelPart();
        for( auto& node : r_model_part.Nodes() )
        {
            ModelPart::NodeType::DofsContainerType& rNodeDofs = node.GetDofs();
            
            for( auto it_dof = std::begin(rNodeDofs); it_dof != std::end(rNodeDofs); it_dof++ )
            {
                if( !it_dof->IsFixed() )
                {
                    it_dof->GetSolutionStepValue(step) = rDisplacement(it_dof->EquationId());
                }
                else
                {
                    it_dof->GetSolutionStepValue(step) = 0.0;
                }
            }
        }
    }

    /**
     * @brief Solves the current step. This function returns true if a solution has been found, false otherwise.
     */
    bool SolveSolutionStep() override
    {
        KRATOS_TRY;
        typename TSchemeType::Pointer p_scheme = GetScheme();
        typename TBuilderAndSolverType::Pointer p_builder_and_solver = GetBuilderAndSolver();
        TSystemMatrixType& rA  = *mpA;
        TSystemMatrixType& rM = *mpM;
        TSystemMatrixType& rS  = *mpS;
        TSystemVectorType& rRHS  = *mpRHS;
        SparseSpaceType::Set(rRHS,0.0); //why??

        TSystemVectorType tmp(rA.size1(), 0.0);

        p_builder_and_solver->BuildStiffnessMatrix(p_scheme, BaseType::GetModelPart(), rA, tmp);

        // Applying the Dirichlet Boundary conditions
        p_builder_and_solver->ApplyDirichletConditions(p_scheme, BaseType::GetModelPart(), rA, tmp, rRHS);  

        p_builder_and_solver->BuildMassMatrix(p_scheme, BaseType::GetModelPart(), rM, tmp);

        p_builder_and_solver->ApplyDirichletConditionsForMassMatrix(p_scheme, BaseType::GetModelPart(), rM);

        p_builder_and_solver->BuildRHS(p_scheme, BaseType::GetModelPart(), rRHS);

        p_builder_and_solver->BuildDampingMatrix(p_scheme, BaseType::GetModelPart(), rS, tmp);

        p_builder_and_solver->ApplyDirichletConditionsForDampingMatrix(p_scheme, BaseType::GetModelPart(), rS);
        


        const unsigned int system_size = p_builder_and_solver->GetEquationSystemSize();

        //initialize sb, As, AAs vectors
        auto s = SparseSpaceType::CreateEmptyVectorPointer();
        auto& rs = *s;
        SparseSpaceType::Resize(rs,system_size);
        SparseSpaceType::Set(rs,0.0);

        
        /*ComplexMatrixType Damping_matrix;
        Damping_matrix.resize(system_size, system_size,false);

        ComplexMatrixType System_matrix;
        System_matrix.resize(system_size, system_size,false);

        
        //test = ZeroMatrix(system_size,system_size);

        Damping_matrix = rS;*/

        auto kdyn = SparseSpaceType::CreateEmptyMatrixPointer();
        auto& r_kdyn = *kdyn;
        SparseSpaceType::Resize(r_kdyn, system_size, system_size);

        auto& r_process_info = BaseType::GetModelPart().GetProcessInfo();
        double excitation_frequency = r_process_info[TIME];

        //Building the system matrix
        r_kdyn = rA - ( std::pow(excitation_frequency, 2.0 ) * rM );  
        p_builder_and_solver->GetLinearSystemSolver()->Solve( r_kdyn, rs, rRHS );

        TSystemVectorType displacement;
        displacement.resize( system_size, false );
        displacement = ZeroVector( system_size );

        //assigning the computed displacement
        displacement = rs;
        AssignVariables(displacement);

		return true;
        KRATOS_CATCH("");
        
    }

    /**
     * @brief Function to perform expensive checks.
     * @details It is designed to be called ONCE to verify that the input is correct.
     */
    int Check() override
    {
        KRATOS_TRY

        BaseType::Check();

        GetBuilderAndSolver()->Check(BaseType::GetModelPart());

        GetScheme()->Check(BaseType::GetModelPart());

        return 0;

        KRATOS_CATCH("")
    }

    ///@}
    ///@name Operators

    ///@{

    ///@}
    ///@name Operations
    ///@{

    ///@}
    ///@name Access

    ///@{

    /**
     * @brief This method returns the LHS matrix
     * @return The LHS matrix
     */
    TSystemMatrixType &GetSystemMatrix()
    {
        TSystemMatrixType &mA = *mpA;

        return mA;
    }

    TSystemMatrixType &GetMassMatrix()
    {
        TSystemMatrixType &mM = *mpM;

        return mM;
    }

    TSystemMatrixType &GetDampingMatrix()
    {
        TSystemMatrixType &mS = *mpS;

        return mS;
    }

    /**
     * @brief This method returns the RHS vector
     * @return The RHS vector
     */
    TSystemVectorType& GetSystemVector()
    {
        TSystemVectorType& mb = *mpRHS;

        return mb;
    }

    TSystemMatrixType &GetMr()
    {
        TSystemMatrixType &mMr = *mpMr;

        return mMr;
    };

    TSystemMatrixType &GetAr()
    {
        TSystemMatrixType &mAr = *mpAr;

        return mAr;
    };

    TSystemMatrixType &GetSr()
    {
        TSystemMatrixType &mSr = *mpSr;

        return mSr;
    };

    TSystemVectorType& GetRHSr()
    {
        
        TSystemVectorType& mb = *mpRHSr;

        return mb;
    };

    TSystemMatrixType &GetBasis()
    {
        TSystemMatrixType &mBasis = *mpBasis;

        return mBasis;
    };

    ///@}
    ///@name Inquiry
    ///@{

    ///@}
    ///@name Friends
    ///@{

    ///@}

  private:
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

  protected:
    ///@name Static Member Variables
    ///@{

    ///@}
    ///@name Member Variables
    ///@{
    typename TLinearSolver::Pointer mpLinearSolver; /// The pointer to the linear solver considered
    typename TSchemeType::Pointer mpScheme; /// The pointer to the time scheme employed
    typename TBuilderAndSolverType::Pointer mpBuilderAndSolver; /// The pointer to the builder and solver employe

    TSystemVectorPointerType mpRHS; /// The RHS vector of the system of equations
    TSystemMatrixPointerType mpA; /// The Stiffness matrix of the system of equations
    TSystemMatrixPointerType mpM; /// The Mass matrix of the system of equations
    TSystemMatrixPointerType mpS; /// The Damping matrix of the system of equations

    double omega;  //excitation frequency

    bool mReformDofSetAtEachStep;

    bool mSolutionStepIsInitialized; /// Flag to set as initialized the solution step

    bool mInitializeWasPerformed; /// Flag to set as initialized the strategy

    ///@}
    ///@name Private Operators
    ///@{

    /**
     * @brief This method returns the components of the system of equations depending of the echo level
     * @param IterationNumber The non linear iteration in the solution loop
     */
    virtual void EchoInfo(const unsigned int IterationNumber)
    {
        TSystemMatrixType& rA  = *mpA;
        TSystemMatrixType& rM = *mpM;
        TSystemVectorType& rRHS  = *mpRHS;
        TSystemMatrixType& rS  = *mpS;

        if (this->GetEchoLevel() == 2) //if it is needed to print the debug info
        {
            KRATOS_INFO("RHS") << "RHS  = " << rRHS << std::endl;
        }
        else if (this->GetEchoLevel() == 3) //if it is needed to print the debug info
        {
            KRATOS_INFO("LHS") << "SystemMatrix = " << rA << std::endl;
            KRATOS_INFO("Dx")  << "Mass Matrix = " << mpM << std::endl;
            KRATOS_INFO("Sx")  << "Damping Matrix = " << mpS << std::endl;
            KRATOS_INFO("RHS") << "RHS  = " << rRHS << std::endl;
        }
        std::stringstream matrix_market_name;
        matrix_market_name << "A_" << BaseType::GetModelPart().GetProcessInfo()[TIME] << "_" << IterationNumber << ".mm";
        TSparseSpace::WriteMatrixMarketMatrix((char *)(matrix_market_name.str()).c_str(), rA, false);

        std::stringstream matrix_market_mass_name;
        matrix_market_mass_name << "M_" << BaseType::GetModelPart().GetProcessInfo()[TIME] << "_" << IterationNumber << ".mm";
        TSparseSpace::WriteMatrixMarketMatrix((char *)(matrix_market_mass_name.str()).c_str(), rM, false);   

        std::stringstream matrix_market_damping_name;
        matrix_market_name << "S_" << BaseType::GetModelPart().GetProcessInfo()[TIME] << "_" << IterationNumber << ".mm";
        TSparseSpace::WriteMatrixMarketMatrix((char *)(matrix_market_damping_name.str()).c_str(), rS, false);        

        std::stringstream matrix_market_vectname;
        matrix_market_vectname << "RHS_" << BaseType::GetModelPart().GetProcessInfo()[TIME] << "_" << IterationNumber << ".mm.rhs";
        TSparseSpace::WriteMatrixMarketVector((char *)(matrix_market_vectname.str()).c_str(), rRHS);
    }

    /**
     * @brief This method prints information after reach the max number of iterations
     * @todo Replace by logger
     */

    virtual void MaxIterationsExceeded()
    {
        if (this->GetEchoLevel() != 0 && BaseType::GetModelPart().GetCommunicator().MyPID() == 0)
        {
            std::cout << "***************************************************" << std::endl;
            std::cout << "******* ATTENTION: max iterations exceeded ********" << std::endl;
            std::cout << "***************************************************" << std::endl;
        }
    }

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

    /**
     * Copy constructor.
     */

    FrequencyResponseAnalysisStrategy(const FrequencyResponseAnalysisStrategy &Other){};

    ///@}

}; /* Class MorOfflineStrategy */

///@}

///@name Type Definitions
///@{

///@}

} /* namespace Kratos. */

#endif /* MOR_OFFLINE_STRATEGY  defined */