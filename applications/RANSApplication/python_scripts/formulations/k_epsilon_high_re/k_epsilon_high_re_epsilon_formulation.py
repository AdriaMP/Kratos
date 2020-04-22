from __future__ import print_function, absolute_import, division

# import kratos
import KratosMultiphysics as Kratos

# import required applications
import KratosMultiphysics.RANSApplication as KratosRANS

# import formulation interface
from KratosMultiphysics.RANSApplication.formulations.formulation import Formulation

# import utilities
from KratosMultiphysics import VariableUtils
from KratosMultiphysics.RANSApplication.formulations.utilities import CreateLinearSolver
from KratosMultiphysics.RANSApplication.formulations.utilities import CreateFormulationModelPart
from KratosMultiphysics.RANSApplication.formulations.utilities import CalculateNormalsOnConditions
from KratosMultiphysics.RANSApplication.formulations.utilities import CreateResidualBasedBlockBuilderAndSolver
from KratosMultiphysics.RANSApplication.formulations.utilities import CreateResidualCriteria
from KratosMultiphysics.RANSApplication.formulations.utilities import CreateResidualBasedNewtonRaphsonStrategy
from KratosMultiphysics.RANSApplication.formulations.utilities import CreateSteadyAlgeraicFluxCorrectedTransportScheme
from KratosMultiphysics.RANSApplication.formulations.utilities import CreateBossakScalarScheme
from KratosMultiphysics.RANSApplication.formulations.utilities import CreateSteadyScalarScheme
from KratosMultiphysics.RANSApplication.formulations.utilities import IsBufferInitialized
from KratosMultiphysics.RANSApplication.formulations.utilities import InitializePeriodicConditions

class KEpsilonHighReEpsilonFormulation(Formulation):
    def __init__(self, model_part, settings):
        super(KEpsilonHighReEpsilonFormulation,
              self).__init__(model_part, settings)

        defaults = Kratos.Parameters(r"""{
            "wall_function_type"    : "turbulent_kinetic_energy_based",
            "relative_tolerance"    : 1e-3,
            "absolute_tolerance"    : 1e-5,
            "max_iterations"        : 200,
            "relaxation_factor"     : 0.5,
            "echo_level"            : 2,
            "linear_solver_settings": {
                "solver_type"  : "amgcl"
            }
        }""")

        self.settings.ValidateAndAssignDefaults(defaults)
        self.echo_level = self.settings["echo_level"].GetInt()

    def PrepareModelPart(self):
        wall_function_type = self.settings["wall_function_type"].GetString()
        if (wall_function_type == "turbulent_kinetic_energy_based_lhs"):
            # this wall condition is most preferred, but since it is changing the LHS, positivity preserving quality of
            # stabilization methods are not guranteed.
            condition_name = "RansEvmKEpsilonEpsilonWall"
        elif (wall_function_type == "turbulent_kinetic_energy_based_rhs"):
            # this wall condition is more preferred, but fails if turbulent kinetic energy singularities is found,
            # since this only changes RHS, stabilization gurantees positivity preservation
            condition_name = "RansEvmKEpsilonEpsilonKBasedWallCondition"
        elif wall_function_type == "velocity_based_rhs":
            # this one works even with turbulent kinetic energy singularities, since this also modifies only the RHS
            # positivity preserving quality is guranteed
            condition_name = "RansEvmKEpsilonEpsilonVelocityBasedWallCondition"
        else:
            raise Exception(
                "Only \"turbulent_kinetic_energy_based_lhs\", \"turbulent_kinetic_energy_based_rhs\" and \"velocity_based_rhs\" wall function types supported. [ wall_function_type = \""
                + wall_function_type + "\" ].")

        self.epsilon_model_part = CreateFormulationModelPart(self, self.element_name, condition_name)

        Kratos.Logger.PrintInfo(self.GetName(),
                                "Created formulation model part.")

    def Initialize(self):
        VariableUtils().SetNonHistoricalVariableToZero(KratosRANS.RANS_Y_PLUS, self.epsilon_model_part.Conditions)
        VariableUtils().SetNonHistoricalVariableToZero(KratosRANS.FRICTION_VELOCITY, self.epsilon_model_part.Conditions)

        if (self.IsPeriodic()):
            InitializePeriodicConditions(self.GetBaseModelPart(), self.epsilon_model_part, [KratosRANS.TURBULENT_ENERGY_DISSIPATION_RATE])

        solver_settings = self.settings
        linear_solver = CreateLinearSolver(
            solver_settings["linear_solver_settings"])
        builder_and_solver = CreateResidualBasedBlockBuilderAndSolver(
            linear_solver, self.IsPeriodic(), self.GetCommunicator())
        convergence_criteria = CreateResidualCriteria(
                                self.settings["relative_tolerance"].GetDouble(),
                                self.settings["absolute_tolerance"].GetDouble())

        if (self.is_steady_simulation):
            scheme = self.scheme_type(self.settings["relaxation_factor"].GetDouble())
        else:
            scheme = CreateBossakScalarScheme(
                self.epsilon_model_part.ProcessInfo[Kratos.BOSSAK_ALPHA],
                self.settings["relaxation_factor"].GetDouble(),
                KratosRANS.TURBULENT_ENERGY_DISSIPATION_RATE,
                KratosRANS.TURBULENT_ENERGY_DISSIPATION_RATE_2,
                KratosRANS.RANS_AUXILIARY_VARIABLE_2)

        self.solver = CreateResidualBasedNewtonRaphsonStrategy(
            self.epsilon_model_part, scheme,
            linear_solver, convergence_criteria, builder_and_solver, self.settings["max_iterations"].GetInt(), False,
            False, False)

        builder_and_solver.SetEchoLevel(
            solver_settings["echo_level"].GetInt() - 3)
        self.solver.SetEchoLevel(
            solver_settings["echo_level"].GetInt() - 2)
        convergence_criteria.SetEchoLevel(
            solver_settings["echo_level"].GetInt() - 1)

        self.solver.Initialize()
        Kratos.Logger.PrintInfo(self.GetName(), "Initialized formulation")

    def InitializeSolutionStep(self):
        self.solver.InitializeSolutionStep()

    def IsConverged(self):
        if (hasattr(self, "is_solved_once")):
            return self.GetStrategy().IsConverged()
        else:
            return False

    def SolveCouplingStep(self):
        if (IsBufferInitialized(self)):
            self.is_solved_once = True
            self.solver.Predict()
            self.solver.SolveSolutionStep()
            Kratos.Logger.PrintInfo(self.GetName(), "Solved  formulation.")
            return True

        return False

    def FinializeSolutionStep(self):
        self.solver.FinializeSolutionStep()

    def Check(self):
        self.solver.Check()

    def Clear(self):
        self.solver.Clear()

    def GetStrategy(self):
        return self.solver

    def SetTimeSchemeSettings(self, settings):
        if (settings.Has("scheme_type")):
            scheme_type = settings["scheme_type"].GetString()
            if (scheme_type == "steady"):
                self.is_steady_simulation = True
            elif (scheme_type == "transient"):
                self.is_steady_simulation = False
            else:
                raise Exception("Only \"steady\" and \"transient\" scheme types supported. [ scheme_type = \"" + scheme_type  + "\" ]")
        else:
            raise Exception("\"scheme_type\" is missing in time scheme settings")

    def GetMaxCouplingIterations(self):
        return "N/A"

    def GetModelPart(self):
        return self.epsilon_model_part

    def SetStabilizationMethod(self, stabilization_method):
        if (stabilization_method == "algebraic_flux_corrected"):
            self.element_name = "RansEvmKEpsilonEpsilonAFC"
            self.scheme_type = CreateSteadyAlgeraicFluxCorrectedTransportScheme
        elif (stabilization_method == "residual_based_flux_corrected"):
            self.element_name = "RansEvmKEpsilonEpsilonResidualBasedFC"
            self.scheme_type = CreateSteadyScalarScheme
        elif (stabilization_method == "non_linear_cross_wind_dissipation"):
            self.element_name = "RansEvmKEpsilonEpsilonCrossWindStabilized"
            self.scheme_type = CreateSteadyScalarScheme
        else:
            raise Exception("Unsupported stabilization method")
