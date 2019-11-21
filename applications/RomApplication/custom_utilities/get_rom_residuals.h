//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ \.
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:         BSD License
//  Kratos default license: kratos/license.txt
//
//  Main authors:    RAUL BRAVO
//

#if !defined( GET_ROM_RESIDUALS_H_INCLUDED )
#define  GET_ROM_RESIDUALS_H_INCLUDED

/* Project includes */
#include "includes/define.h"
#include "includes/model_part.h"
#include "solving_strategies/schemes/scheme.h"
#include "solving_strategies/builder_and_solvers/builder_and_solver.h"

/* Application includes */
#include "rom_application_variables.h"
 
// This utility returns the converged residuals projected onto the ROM basis Phi.
namespace Kratos
{
    
template<class TSparseSpace,
         class TDenseSpace, // = DenseSpace<double>,
         class TLinearSolver //= LinearSolver<TSparseSpace,TDenseSpace>
         >
class GetRomResiduals: public BuilderAndSolver<TSparseSpace,TDenseSpace,TLinearSolver>
    {
        public:
        
        KRATOS_CLASS_POINTER_DEFINITION(GetRomResiduals);

        typedef BuilderAndSolver<TSparseSpace,TDenseSpace,TLinearSolver> BaseType;
        typedef typename BaseType::TSchemeType TSchemeType;
        typedef typename BaseType::LocalSystemVectorType LocalSystemVectorType;
        typedef typename BaseType::LocalSystemMatrixType LocalSystemMatrixType;

        
        GetRomResiduals(
        ModelPart& rModelPart,
        Parameters ThisParameters,
        typename TSchemeType::Pointer pScheme        
        ): mpScheme(pScheme), mpModelPart(rModelPart)
    {
        mNodalVariablesNames = ThisParameters["nodal_unknowns"].GetStringArray();        
        //Need to read the type of the variable and optain its size, incorrectly done here
        //Privisional way of retrieving the number of DOFs. Better alternative to be implemented.
        if (mNodalVariablesNames[0] == "TEMPERATURE")
            mNodalDofs = 1;
        if (mNodalVariablesNames[0] == "DISPLACEMENT")
            mNodalDofs = std::stoi(mNodalVariablesNames[1]);
        mRomDofs = ThisParameters["number_of_rom_dofs"].GetInt();   
        mpScheme = pScheme;
    }

        ~GetRomResiduals();


        Matrix Calculate() 
        {
            // Getting the elements from the model
            const int nelements = static_cast<int>(mpModelPart.Elements().size());

            // Getting the array of the conditions
            const int nconditions = static_cast<int>(mpModelPart.Conditions().size());
            auto& CurrentProcessInfo = mpModelPart.GetProcessInfo();
            auto el_begin = mpModelPart.ElementsBegin();
            auto cond_begin = mpModelPart.ConditionsBegin();

            //contributions to the system
            LocalSystemMatrixType LHS_Contribution = LocalSystemMatrixType(0, 0);
            LocalSystemVectorType RHS_Contribution = LocalSystemVectorType(0);

            //vector containing the localization in the system of the different
            //terms
            Element::EquationIdVectorType EquationId;
            Matrix MatrixResiduals( (nelements + nconditions), mRomDofs);   // Matrix of reduced residuals.
            
            for (int k = 0; k < nelements; k++)
            {
                auto it_el = el_begin + k;
                //detect if the element is active or not. If the user did not make any choice the element
                //is active by default
                bool element_is_active = true;
                if ((it_el)->IsDefined(ACTIVE))
                    element_is_active = (it_el)->Is(ACTIVE);              
                
                if (element_is_active)
                {   //calculate elemental contribution
                    mpScheme->CalculateSystemContributions(*(it_el.base()), LHS_Contribution, RHS_Contribution, EquationId, CurrentProcessInfo);                    
                    Element::DofsVectorType dofs;
                    it_el->GetDofList(dofs, CurrentProcessInfo);
                    //assemble the elemental contribution - here is where the ROM acts
                    //compute the elemental reduction matrix PhiElemental
                    const auto& geom = it_el->GetGeometry();
                    Matrix PhiElemental(geom.size()*mNodalDofs, mRomDofs);
                    Vector ResidualReduced(mRomDofs); // The size of the residual will vary only when using more ROM modes, one row per element

                    for(unsigned int i=0; i<geom.size(); ++i)
                    {
                        const Matrix& rom_nodal_basis = geom[i].GetValue(ROM_BASIS);
                        for(unsigned int k=0; k<rom_nodal_basis.size1(); ++k)
                        {
                            if (dofs[i*mNodalDofs + k]->IsFixed())
                                row(PhiElemental, i*mNodalDofs + k) = ZeroVector(PhiElemental.size2());
                            else
                                row(PhiElemental, i*mNodalDofs+k) = row(rom_nodal_basis,k);
                        }
                    }
                    ResidualReduced = prod(trans(PhiElemental), RHS_Contribution);
                    row(MatrixResiduals, k) = ResidualReduced;
                    KRATOS_WATCH(ResidualReduced)

                    // clean local elemental me overridemory
                    mpScheme->CleanMemory(*(it_el.base()));                
                }
                
            }

            // #pragma omp for  schedule(guided , 512)
            for (int k = 0; k < nconditions;  k++)
            {
                ModelPart::ConditionsContainerType::iterator it = cond_begin + k;
                //detect if the condition is active or not. If the user did not make any choice the condition
                //is active by default
                bool condition_is_active = true;
                if ((it)->IsDefined(ACTIVE))
                    condition_is_active = (it)->Is(ACTIVE);

                if (condition_is_active)
                {
                    Condition::DofsVectorType dofs;
                    it->GetDofList(dofs, CurrentProcessInfo);
                    //calculate elemental contribution
                    mpScheme->Condition_CalculateSystemContributions(*(it.base()), LHS_Contribution, RHS_Contribution, EquationId, CurrentProcessInfo);

                    //assemble the elemental contribution - here is where the ROM acts
                    //compute the elemental reduction matrix PhiElemental
                    const auto& r_geom = it->GetGeometry();
                    Matrix PhiElemental(r_geom.size()*mNodalDofs, mRomDofs);
                    Vector ResidualReduced(mRomDofs); // The size of the residual will vary only when using more ROM modes, one row per condition

                    for(unsigned int i=0; i<r_geom.size(); ++i)
                    {
                        const Matrix& rom_nodal_basis = r_geom[i].GetValue(ROM_BASIS);
                        for(unsigned int k=0; k<rom_nodal_basis.size1(); ++k)
                        {
                            if (dofs[i*mNodalDofs + k]->IsFixed())
                                row(PhiElemental, i*mNodalDofs + k) = ZeroVector(PhiElemental.size2());
                            else
                                row(PhiElemental, i*mNodalDofs+k) = row(rom_nodal_basis,k);
                        }
                    }                                       
                    ResidualReduced = prod(trans(PhiElemental), RHS_Contribution);
                    row(MatrixResiduals, k+nelements) = ResidualReduced;

                    // clean local elemental memory
                    mpScheme->CleanMemory(*(it.base()));
                }
                
                // std::ofstream myfile;
                // myfile.open ("/home/jrbravo/Desktop/PhD/example.txt");
                // myfile << MatrixResiduals;
                // myfile.close();            
                // KRATOS_WATCH(MatrixResiduals)

            }
        return MatrixResiduals;        
        }


        
            std::vector< std::string > mNodalVariablesNames;
            int mNodalDofs;
            int mRomDofs;
            typename TSchemeType::Pointer mpScheme;
            ModelPart& mpModelPart;

        };



} // namespace Kratos



#endif // GET_ROM_RESIDUALS_H_INCLUDED  defined