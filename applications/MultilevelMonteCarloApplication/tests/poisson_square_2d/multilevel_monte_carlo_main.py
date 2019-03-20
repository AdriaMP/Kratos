from __future__ import absolute_import, division #makes KratosMultiphysics backward compatible with python 2.6 and 2.7

# Importing the Kratos Library
import KratosMultiphysics

# Importing the analysis stage classes of the different problems
from simulation_definition import SimulationScenario

# Import Monte Carlo library
import KratosMultiphysics.MultilevelMonteCarloApplication.mc_utilities as mc_utilities
import KratosMultiphysics.MultilevelMonteCarloApplication.cmlmc_utilities as cmlmc_utilities

# TODO: use json file instead of defining in the main file all the parameters

if __name__ == '__main__':

    """  __ __  __
        |  V  |/ _|
        | \_/ | (_
        |_| |_|\__|
    """

    # set the ProjectParameters.json path
    project_parameters_path = "problem_settings/parameters_poisson_square_2d_coarse.json"
    # customize setting parameters of the MC simulation"""
    settings_MC_simulation = KratosMultiphysics.Parameters("""
    {
        "tolerance"  : 0.001,
        "confidence" : 0.9,
        "batch_size" : 5,
        "convergence_criteria" : "MC_sample_variance_sequential_stopping_rule"
    }
    """)
    # contruct MonteCarlo or MultilevelMonteCarlo class
    mc_manager = mc_utilities.MonteCarlo(settings_MC_simulation,project_parameters_path,SimulationScenario)
    # execute algorithm
    mc_manager.Run()


    """  __ __ __ _   __ __  __
        / _|  V  | | |  V  |/ _|
       | (_| \_/ | |_| \_/ | (_
        \__|_| |_|___|_| |_|\__|
    """

    # set the ProjectParameters.json path
    project_parameters_path = "problem_settings/parameters_poisson_square_2d_coarse.json"
    # customize setting parameters of the MLMC simulation
    settings_MLMC_simulation = KratosMultiphysics.Parameters("""
    {
        "initial_tolerance"             : 0.25,
        "tolerance"                     : 0.1,
        "confidence"                    : 0.9,
        "number_samples_screening"      : 25,
        "levels_screening"              : 2,
        "initial_mesh_size"             : 0.5
    }
    """)
    # customize setting parameters of the metric of the adaptive refinement utility
    custom_metric_refinement_parameters = KratosMultiphysics.Parameters("""
        {
            "hessian_strategy_parameters"           :{
                    "metric_variable"               : ["TEMPERATURE"],
                    "estimate_interpolation_error"  : false,
                    "interpolation_error"           : 0.004
            },
            "anisotropy_remeshing"                  : true,
            "anisotropy_parameters":{
                "reference_variable_name"           : "TEMPERATURE",
                "hmin_over_hmax_anisotropic_ratio"  : 0.15,
                "boundary_layer_max_distance"       : 1.0,
                "interpolation"                     : "Linear"
            },
            "local_gradient_variable"               : "TEMPERATURE"
        }
    """)
    # customize setting parameters of the remesh of the adaptive refinement utility
    custom_remesh_refinement_settings = KratosMultiphysics.Parameters("""
        {
            "echo_level"                            : 0
        }
    """)
    # contruct MultilevelMonteCarlo class
    mlmc_manager = cmlmc_utilities.MultilevelMonteCarlo(settings_MLMC_simulation,project_parameters_path,custom_metric_refinement_parameters,custom_remesh_refinement_settings,SimulationScenario)
    mlmc_manager.Run()
