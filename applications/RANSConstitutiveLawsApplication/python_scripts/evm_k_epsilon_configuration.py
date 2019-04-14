from __future__ import print_function, absolute_import, division

import KratosMultiphysics as Kratos
import KratosMultiphysics.RANSConstitutiveLawsApplication as KratosRANS
from turbulence_eddy_viscosity_model_configuration import TurbulenceEddyViscosityModelConfiguration

from KratosMultiphysics.kratos_utilities import IsApplicationAvailable
if IsApplicationAvailable("FluidDynamicsApplication"):
    import KratosMultiphysics.FluidDynamicsApplication
else:
    msg = "k-epsilon turbulence model depends on the FluidDynamicsApplication which is not found."
    msg += " Please re-install/compile with FluidDynamicsApplication"
    raise Exception(msg)

class TurbulenceKEpsilonConfiguration(
        TurbulenceEddyViscosityModelConfiguration):
    def __init__(self, model, parameters, linear_solver):
        super(TurbulenceKEpsilonConfiguration, self).__init__(model, parameters)

        self.linear_solver = linear_solver
        default_settings = Kratos.Parameters(r'''{
                "scheme_settings": {
                    "scheme_type": "bossak",
                    "alpha_bossak": -0.3
                },
                "echo_level"        :0,
                "turbulent_kinetic_energy_settings":{
                    "relative_tolerance"    : 1e-3,
                    "absolute_tolerance"    : 1e-5,
                    "max_iterations"        : 200,
                    "echo_level"            : 0,
                    "linear_solver_settings": {
                        "solver_type"  : "amgcl"
                    }
                },
                "turbulent_energy_dissipation_rate_settings":{
                    "relative_tolerance"    : 1e-3,
                    "absolute_tolerance"    : 1e-5,
                    "max_iterations"        : 200,
                    "echo_level"            : 0,
                    "linear_solver_settings": {
                        "solver_type"  : "amgcl"
                    }
                },
                "constants":
                {
                    "wall_smoothness_beta"    : 5.2,
                    "von_karman"              : 0.41,
                    "c_mu"                    : 0.09,
                    "c1"                      : 1.44,
                    "c2"                      : 1.92,
                    "sigma_k"                 : 1.0,
                    "sigma_epsilon"           : 1.3
                },
                "flow_parameters":
                {
                    "ramp_up_time"                        : 0.5
                }
        }''')

    #     parameters["model_settings"].ValidateAndAssignDefaults(default_settings)

    #     self.model_elements = ["RANSEVMK", "RANSEVMEPSILON"]
    #     self.model_conditions = ["Condition", "Condition"]
    #     self.rans_solver_configurations = []
    #     self.is_initial_values_assigned = False

    #     self.ramp_up_time = self.settings["model_settings"]["flow_parameters"]["ramp_up_time"].GetDouble()

    # def PrepareSolvingStrategy(self):
    #     # reading constants
    #     constants = self.settings["model_settings"]["constants"]
    #     self.fluid_model_part.ProcessInfo[KratosRANS.WALL_SMOOTHNESS_BETA] = constants["wall_smoothness_beta"].GetDouble()
    #     self.fluid_model_part.ProcessInfo[KratosRANS.WALL_VON_KARMAN] = constants["von_karman"].GetDouble()
    #     self.fluid_model_part.ProcessInfo[KratosRANS.TURBULENCE_RANS_C_MU] = constants["c_mu"].GetDouble()
    #     self.fluid_model_part.ProcessInfo[KratosRANS.TURBULENCE_RANS_C1] = constants["c1"].GetDouble()
    #     self.fluid_model_part.ProcessInfo[KratosRANS.TURBULENCE_RANS_C2] = constants["c2"].GetDouble()
    #     self.fluid_model_part.ProcessInfo[KratosRANS.TURBULENT_KINETIC_ENERGY_SIGMA] = constants["sigma_k"].GetDouble()
    #     self.fluid_model_part.ProcessInfo[KratosRANS.TURBULENT_ENERGY_DISSIPATION_RATE_SIGMA] = constants["sigma_epsilon"].GetDouble()
    #     self.fluid_model_part.ProcessInfo[KratosRANS.TURBULENT_VISCOSITY_MIN] = self.nu_t_min
    #     self.fluid_model_part.ProcessInfo[KratosRANS.TURBULENT_VISCOSITY_MAX] = self.nu_t_max

    #     scheme_settings = self.settings["model_settings"]["scheme_settings"]

    #     # create turbulent kinetic energy strategy
    #     model_part = self.turbulence_model_parts_list[0]
    #     solver_settings = self.settings["model_settings"]["turbulent_kinetic_energy_settings"]
    #     scalar_variable = KratosRANS.TURBULENT_KINETIC_ENERGY
    #     scalar_variable_rate = KratosRANS.TURBULENT_KINETIC_ENERGY_RATE
    #     relaxed_scalar_variable_rate = KratosRANS.RANS_AUXILIARY_VARIABLE_1
    #     self.rans_solver_configurations.append(
    #         self.CreateStrategy(solver_settings, scheme_settings, model_part,
    #                             scalar_variable, scalar_variable_rate,
    #                             relaxed_scalar_variable_rate))

    #     current_strategy = self.rans_solver_configurations[-1][0]
    #     self.strategies_list.append(current_strategy)
    #     self.GetTurbulenceSolvingProcess().AddStrategy(current_strategy)

    #     # create turbulent energy dissipation rate strategy
    #     model_part = self.turbulence_model_parts_list[1]
    #     solver_settings = self.settings["model_settings"]["turbulent_energy_dissipation_rate_settings"]
    #     scalar_variable = KratosRANS.TURBULENT_ENERGY_DISSIPATION_RATE
    #     scalar_variable_rate = KratosRANS.TURBULENT_ENERGY_DISSIPATION_RATE_2
    #     relaxed_scalar_variable_rate = KratosRANS.RANS_AUXILIARY_VARIABLE_2
    #     self.rans_solver_configurations.append(
    #         self.CreateStrategy(solver_settings, scheme_settings, model_part,
    #                             scalar_variable, scalar_variable_rate,
    #                             relaxed_scalar_variable_rate))

    #     current_strategy = self.rans_solver_configurations[-1][0]
    #     self.strategies_list.append(current_strategy)
    #     self.GetTurbulenceSolvingProcess().AddStrategy(current_strategy)

    #     Kratos.Logger.PrintInfo(self.__class__.__name__, "All turbulence solution strategies are created.")

    def AddVariables(self):
        # adding k-epsilon specific variables
        self.fluid_model_part.AddNodalSolutionStepVariable(KratosRANS.TURBULENT_KINETIC_ENERGY)
        self.fluid_model_part.AddNodalSolutionStepVariable(KratosRANS.TURBULENT_KINETIC_ENERGY_RATE)
        self.fluid_model_part.AddNodalSolutionStepVariable(KratosRANS.TURBULENT_ENERGY_DISSIPATION_RATE)
        self.fluid_model_part.AddNodalSolutionStepVariable(KratosRANS.TURBULENT_ENERGY_DISSIPATION_RATE_2)
        self.fluid_model_part.AddNodalSolutionStepVariable(KratosRANS.RANS_AUXILIARY_VARIABLE_1)
        self.fluid_model_part.AddNodalSolutionStepVariable(KratosRANS.RANS_AUXILIARY_VARIABLE_2)

        super(TurbulenceKEpsilonConfiguration, self).AddVariables()

    def AddDofs(self):
        Kratos.VariableUtils().AddDof(KratosRANS.TURBULENT_KINETIC_ENERGY, self.fluid_model_part)
        Kratos.VariableUtils().AddDof(KratosRANS.TURBULENT_ENERGY_DISSIPATION_RATE, self.fluid_model_part)

        Kratos.Logger.PrintInfo(self.__class__.__name__, "DOFs added successfully.")

    # def InitializeSolutionStep(self):
    #     time = self.fluid_model_part.ProcessInfo[Kratos.TIME]
    #     if (time >= self.ramp_up_time):
    #         self.is_computing_solution = True
    #         self.GetTurbulenceSolvingProcess().SetIsCoSolvingProcessActive(self.is_computing_solution)
    #     else:
    #         return

    #     super(TurbulenceKEpsilonConfiguration, self).InitializeSolutionStep()
    #     self.UpdateBoundaryConditions()

    # def UpdateBoundaryConditions(self):
    #     evm_k_epsilon_utilities = KratosRANS.EvmKepsilonModelUtilities()

    #     self.CalculateYPlus()

    #     if not self.is_initial_values_assigned:
    #         evm_k_epsilon_utilities.AssignInitialValues(self.fluid_model_part)
    #         self.is_initial_values_assigned = True
    #         Kratos.Logger.PrintInfo(self.__class__.__name__, "Assigned initial values for turbulence model.")

    #     evm_k_epsilon_utilities.UpdateBoundaryConditions(self.fluid_model_part)

    # def InitializeBoundaryNodes(self):
    #     rans_variable_utils = KratosRANS.RansVariableUtils()

    #     rans_variable_utils.FixScalarVariableDofs(Kratos.INLET, KratosRANS.TURBULENT_KINETIC_ENERGY, self.fluid_model_part.Nodes)
    #     rans_variable_utils.FixScalarVariableDofs(Kratos.STRUCTURE, KratosRANS.TURBULENT_KINETIC_ENERGY, self.fluid_model_part.Nodes)

    #     rans_variable_utils.FixScalarVariableDofs(Kratos.INLET, KratosRANS.TURBULENT_ENERGY_DISSIPATION_RATE, self.fluid_model_part.Nodes)
    #     rans_variable_utils.FixScalarVariableDofs(Kratos.STRUCTURE, KratosRANS.TURBULENT_ENERGY_DISSIPATION_RATE, self.fluid_model_part.Nodes)

    def GetTurbulenceSolvingProcess(self):
        if self.turbulence_model_process is None:
            self.turbulence_model_process = KratosRANS.TurbulenceEvmKEpsilon2DProcess(
                                                self.fluid_model_part, self.settings, self.linear_solver, self.linear_solver, self.linear_solver)
            Kratos.Logger.PrintInfo(self.__class__.__name__, "Created turbulence solving process.")

        return self.turbulence_model_process
