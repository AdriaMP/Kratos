//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Pooyan Dadvand
//                   Riccardo Rossi
//

// System includes

// External includes

// Project includes
#include "includes/define_python.h"
#include "includes/dem_variables.h"
#include "python/add_dem_variables_to_python.h"

namespace Kratos
{

namespace Python
{

void  AddDEMVariablesToPython(pybind11::module& m)
{
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, TOTAL_FORCES )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, DAMP_FORCES )
    //variables in the general Discrete_element class & MPI & gidio

    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, FRAME_OF_REFERENCE_TYPE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, PARTICLE_MASS )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, RADIUS )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, PARTICLE_MATERIAL )

    //variables for the neighbour search
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SEARCH_RADIUS_INCREMENT )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SEARCH_RADIUS_INCREMENT_FOR_WALLS )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, OLD_NEIGHBOURS_IDS)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, INI_NEIGHBOURS_IDS)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, CONTINUUM_INI_NEIGHBOURS_IDS)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, NEIGHBOURS_IDS )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, NEIGHBOURS_IDS_DOUBLE )

    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, ELEMENT_TYPE )

    // Swimming DEM Application BEGINNING
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, NUMBER_OF_INIT_BASSET_STEPS )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, COUPLING_TYPE)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, NON_NEWTONIAN_OPTION )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, MANUALLY_IMPOSED_DRAG_LAW_OPTION )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, DRAG_MODIFIER_TYPE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, BUOYANCY_FORCE_TYPE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, DRAG_FORCE_TYPE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, VIRTUAL_MASS_FORCE_TYPE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, BASSET_FORCE_TYPE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, LIFT_FORCE_TYPE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, MAGNUS_FORCE_TYPE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, HYDRO_TORQUE_TYPE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, FLUID_MODEL_TYPE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, DRAG_POROSITY_CORRECTION_TYPE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, TIME_STEPS_PER_QUADRATURE_STEP )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, QUADRATURE_ORDER )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, CURRENT_COMPONENT )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, POWER_LAW_TOLERANCE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, PARTICLE_SPHERICITY )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, INIT_DRAG_FORCE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, DRAG_LAW_SLOPE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, DISPERSE_FRACTION )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, DISPERSE_FRACTION_RATE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, FLUID_FRACTION )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, FLUID_FRACTION_OLD )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, FLUID_FRACTION_RATE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, FLUID_FRACTION_FILTERED )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, PHASE_FRACTION )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, PHASE_FRACTION_RATE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, DISPERSE_FRACTION_PROJECTED )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, FLUID_FRACTION_PROJECTED )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, FLUID_DENSITY_PROJECTED )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, FLUID_VISCOSITY_PROJECTED )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, REYNOLDS_NUMBER )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, DRAG_COEFFICIENT )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SHEAR_RATE_PROJECTED )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, DELTA_TIME_QUADRATURE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, LAST_TIME_APPENDING )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, TIME_AVERAGED_DOUBLE )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, VELOCITY_OLD )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, VELOCITY_OLD_OLD )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, ACCELERATION_MOVING_FRAME_ORIGIN )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, ANGULAR_VELOCITY_MOVING_FRAME )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, ANGULAR_VELOCITY_MOVING_FRAME_OLD )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, ANGULAR_ACCELERATION_MOVING_FRAME )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, SLIP_VELOCITY )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, ADDITIONAL_FORCE )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, ADDITIONAL_FORCE_OLD )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, ADDITIONAL_FORCE_OLD_OLD )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, HYDRODYNAMIC_FORCE )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, HYDRODYNAMIC_MOMENT )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, FLUID_VEL_PROJECTED )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, FLUID_VEL_PROJECTED_RATE )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, FLUID_VEL_LAPL_PROJECTED )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, FLUID_VEL_LAPL_RATE_PROJECTED )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, MATERIAL_FLUID_ACCEL_PROJECTED )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, FLUID_ACCEL_PROJECTED )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, FLUID_ACCEL_FOLLOWING_PARTICLE_PROJECTED )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, FLUID_VORTICITY_PROJECTED )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, PARTICLE_VEL_FILTERED )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, HYDRODYNAMIC_REACTION )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, MEAN_HYDRODYNAMIC_REACTION )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, DRAG_REACTION )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, LIFT_FORCE )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, VIRTUAL_MASS_FORCE )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, BASSET_FORCE )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, EXTERNAL_APPLIED_FORCE )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, EXTERNAL_APPLIED_MOMENT )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, BUOYANCY )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, PRESSURE_GRADIENT )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, PRESSURE_GRAD_PROJECTED )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, DISPERSE_FRACTION_GRADIENT )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, DISPERSE_FRACTION_GRADIENT_PROJECTED )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, FLUID_FRACTION_GRADIENT )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, FLUID_FRACTION_GRADIENT_PROJECTED )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, PHASE_FRACTION_GRADIENT )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, TIME_AVERAGED_ARRAY_3 )
    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(m, AUX_VEL )
    // Swimming DEM Application END

    // Nano-particle only BEGIN
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, CATION_CONCENTRATION )
    // Nano-particle only END

    // for DEM and DEM-FEM_Application

    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,PARTICLE_ROTATE_SPRING_FAILURE_TYPE)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,DEM_DELTA_TIME)
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m,FIXED_MESH_OPTION)

    // for DEM Sintering

    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, ATOMIC_VOLUME )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SURFACE_ENERGY )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, DIHEDRAL_ANGLE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, SINTERING_START_TEMPERATURE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, RELAXATION_TIME )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, LARGE_VISCOSITY_COEFFICIENT )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, PRE_EXP_DIFFUSION_COEFFICIENT )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, GB_WIDTH )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(m, ENTHAPLY_ACTIVATION )

}
} // namespace Python.
} // Namespace Kratos
