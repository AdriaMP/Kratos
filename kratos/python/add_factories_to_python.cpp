//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Riccardo Rossi
//                   Vicente Mataix Ferrandiz
//
//

// System includes

// External includes

// Project includes
#include "includes/define_python.h"
#include "python/add_factories_to_python.h"
#include "factories/linear_solver_factory.h"
#include "factories/preconditioner_factory.h"
#include "factories/convergence_criteria_factory.h"
#include "factories/scheme_factory.h"
#include "factories/builder_and_solver_factory.h"
#include "factories/strategy_factory.h"

namespace Kratos
{

namespace Python
{

void  AddFactoriesToPython(pybind11::module& m)
{
    namespace py = pybind11;

    typedef UblasSpace<double, CompressedMatrix, boost::numeric::ublas::vector<double>> SpaceType;
    typedef UblasSpace<double, Matrix, Vector> LocalSpaceType;
    typedef LinearSolver<SpaceType,  LocalSpaceType> LinearSolverType;
    typedef TUblasSparseSpace<std::complex<double>> ComplexSpaceType;
    typedef TUblasDenseSpace<std::complex<double>> ComplexLocalSpaceType;

    //////////////////////////////////////////////////////////////7
    //HERE THE TOOLS TO REGISTER LINEAR SOLVERS

    typedef LinearSolverFactory< SpaceType, LocalSpaceType > LinearSolverFactoryType;
    typedef LinearSolverFactory< ComplexSpaceType, ComplexLocalSpaceType > ComplexLinearSolverFactoryType;
    typedef PreconditionerFactory< SpaceType, LocalSpaceType > PreconditionerFactoryType;
    typedef ConvergenceCriteriaFactory< SpaceType, LocalSpaceType > ConvergenceCriteriaFactoryType;
    typedef SchemeFactory< SpaceType, LocalSpaceType > SchemeFactoryType;
    typedef BuilderAndSolverFactory< SpaceType, LocalSpaceType, LinearSolverType > BuilderAndSolverFactoryType;
    typedef StrategyFactory< SpaceType, LocalSpaceType, LinearSolverType > StrategyFactoryType;

    py::class_<LinearSolverFactoryType, LinearSolverFactoryType::Pointer>(m, "LinearSolverFactory")
     .def( py::init< >() )
     .def("Create",&LinearSolverFactoryType::Create)
     .def("Has",&LinearSolverFactoryType::Has)
    ;

    py::class_<ComplexLinearSolverFactoryType, ComplexLinearSolverFactoryType::Pointer>(m, "ComplexLinearSolverFactory")
     .def( py::init< >() )
     .def("Create",&ComplexLinearSolverFactoryType::Create)
     .def("Has",&ComplexLinearSolverFactoryType::Has)
    ;

    py::class_<PreconditionerFactoryType, PreconditionerFactoryType::Pointer >(m, "PreconditionerFactory")
     .def( py::init< >() )
     .def("Create",&PreconditionerFactoryType::Create)
     .def("Has",&PreconditionerFactoryType::Has)
    ;

    py::class_<ConvergenceCriteriaFactoryType, ConvergenceCriteriaFactoryType::Pointer >(m, "ConvergenceCriteriaFactory")
     .def( py::init< >() )
     .def("Create",&ConvergenceCriteriaFactoryType::Create)
     .def("Has",&ConvergenceCriteriaFactoryType::Has)
    ;

    py::class_<SchemeFactoryType, SchemeFactoryType::Pointer >(m, "SchemeFactory")
     .def( py::init< >() )
     .def("Create",&SchemeFactoryType::Create)
     .def("Has",&SchemeFactoryType::Has)
    ;

    py::class_<BuilderAndSolverFactoryType, BuilderAndSolverFactoryType::Pointer >(m, "BuilderAndSolverFactory")
     .def( py::init< >() )
     .def("Create",&BuilderAndSolverFactoryType::Create)
     .def("Has",&BuilderAndSolverFactoryType::Has)
    ;

    py::class_<StrategyFactoryType, StrategyFactoryType::Pointer >(m, "StrategyFactory")
     .def( py::init< >() )
     .def("Create",&StrategyFactoryType::Create)
     .def("Has",&StrategyFactoryType::Has)
    ;

}

}  // namespace Python.

} // Namespace Kratos
