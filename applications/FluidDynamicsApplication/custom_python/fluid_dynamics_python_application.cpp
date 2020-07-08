//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Jordi Cotela
//

// System includes

#if defined(KRATOS_PYTHON)
// External includes
#include <pybind11/pybind11.h>


// Project includes
#include "includes/define_python.h"
#include "fluid_dynamics_application.h"
#include "custom_python/add_custom_strategies_to_python.h"
#include "custom_python/add_custom_utilities_to_python.h"
#include "custom_python/add_custom_processes_to_python.h"
#include "custom_python/add_custom_constitutive_laws_to_python.h"
#include "custom_python/add_custom_response_functions_to_python.h"


namespace Kratos
{

namespace Python
{



PYBIND11_MODULE(KratosFluidDynamicsApplication,m)
{
    namespace py = pybind11;

    py::class_<KratosFluidDynamicsApplication,
           KratosFluidDynamicsApplication::Pointer,
           KratosApplication >(m,"KratosFluidDynamicsApplication")
           .def(py::init<>())
           ;

    AddCustomConstitutiveLawsToPython(m);
    AddCustomStrategiesToPython(m);
    AddCustomUtilitiesToPython(m);
    AddCustomProcessesToPython(m);
    AddCustomResponseFunctionsToPython(m);

    //registering variables in python
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,PATCH_INDEX);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,TAUONE);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,TAUTWO);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,PRESSURE_MASSMATRIX_COEFFICIENT);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,Y_WALL);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,SUBSCALE_PRESSURE);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,C_DES);
//        KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,C_SMAGORINSKY);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,CHARACTERISTIC_VELOCITY);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,DIVERGENCE);

    // For Non-Newtonian constitutive relations
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,REGULARIZATION_COEFFICIENT);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,BINGHAM_SMOOTHER);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,GEL_STRENGTH);

    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m,SUBSCALE_VELOCITY);
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m,COARSE_VELOCITY);

    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,FIC_BETA);

    // Adjoint variables
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, ADJOINT_FLUID_VECTOR_1 )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, ADJOINT_FLUID_VECTOR_2 )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, ADJOINT_FLUID_VECTOR_3 )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, AUX_ADJOINT_FLUID_VECTOR_1 )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, ADJOINT_FLUID_SCALAR_1 )

    // Embedded fluid variables
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,EMBEDDED_IS_ACTIVE);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,SLIP_LENGTH);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,PENALTY_COEFFICIENT);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,EMBEDDED_WET_PRESSURE);
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m,EMBEDDED_WET_VELOCITY);

    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,Q_VALUE);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,VORTICITY_MAGNITUDE);
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m,RECOVERED_PRESSURE_GRADIENT);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,NODAL_WEIGHTS);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,AUX_DISTANCE);

    // Compressible fluid variables
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,HEAT_CAPACITY_RATIO);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,REACTION_DENSITY);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,REACTION_ENERGY);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,MACH);

    // Level-set convective velocity
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, CONVECTIVE_VELOCITY)

    // Curvature
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,CURVATURE);

    // Smoothed surface to calculate DISTANCE_GRADIENT
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,DISTANCE_AUX);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,DISTANCE_AUX2);
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m,DISTANCE_GRADIENT_AUX);

    // Parallel levelset distance calculator needs an AREA_VARIABLE_AUX
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,AREA_VARIABLE_AUX);

    // A variable to check if node is on cut element (maybe in a layer farther for future!)
    //KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, IS_NEAR_CUT)

    // Contact line calculation
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m,NORMAL_VECTOR);
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m,TANGENT_VECTOR);
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m,CONTACT_VECTOR);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, CONTACT_ANGLE);
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m,CONTACT_VECTOR_MICRO);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, CONTACT_ANGLE_MICRO);
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, CONTACT_VELOCITY);

    // Enriched pressure is an array of NumNodes components defined for elements. Access it using Element.GetValue()
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, ENRICHED_PRESSURE_1)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, ENRICHED_PRESSURE_2)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, ENRICHED_PRESSURE_3)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, ENRICHED_PRESSURE_4)

    // Last known velocity and pressure to recalculate the last increment
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, VELOCITY_STAR)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, PRESSURE_STAR)

    // Pressure gradient to calculate its jump over interface
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, PRESSURE_GRADIENT_AUX)
}


}  // namespace Python.

}  // namespace Kratos.

#endif // KRATOS_PYTHON defined
