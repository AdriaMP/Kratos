//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:         BSD License
//                   Kratos default license: kratos/license.txt
//
//  Main authors:    Vicente Mataix Ferrandiz
//

// System includes

// External includes

// Project includes
#include "includes/define.h"
#include "factories/register_factories.h"
#include "spaces/ublas_space.h"

// Builder And Solver
#include "solving_strategies/builder_and_solvers/explicit_builder.h"

namespace Kratos
{
void RegisterExplicitBuildersFactories()
{
    typedef TUblasSparseSpace<double> SpaceType;
    typedef TUblasDenseSpace<double> LocalSpaceType;

    typedef ExplicitBuilder<SpaceType,  LocalSpaceType> ExplicitBuilderType;

    static std::vector<Internals::RegisteredPrototypeBase<ExplicitBuilderType>> msPrototypesExplicitBuilder{
        Internals::RegisteredPrototype<ExplicitBuilderType,ExplicitBuilderType>(ExplicitBuilderType::Name())
    };
};
} // Namespace Kratos

