# Importing the Kratos Library
import KratosMultiphysics

# Import applications
import KratosMultiphysics.StructuralMechanicsApplication as StructuralMechanicsApplication
import KratosMultiphysics.ContactStructuralMechanicsApplication as ContactStructuralMechanicsApplication

# Import the implicit solver (the explicit one is derived from it)
from KratosMultiphysics.StructuralMechanicsApplication import structural_mechanics_implicit_dynamic_solver

# Import auxiliar methods
from KratosMultiphysics.ContactStructuralMechanicsApplication import auxiliar_methods_solvers

# Import convergence_criteria_factory
from KratosMultiphysics.StructuralMechanicsApplication import convergence_criteria_factory

def GetDefaults():
    return auxiliar_methods_solvers.AuxiliarMPCContactSettings()

def CreateSolver(model, custom_settings):
    return MPCContactImplicitMechanicalSolver(model, custom_settings)

class MPCContactImplicitMechanicalSolver(structural_mechanics_implicit_dynamic_solver.ImplicitMechanicalSolver):
    """The MPC contact implicit dynamic solver.

    This class creates the mechanical solvers for contact implicit dynamic analysis. It currently
    supports linear and Newton-Raphson strategies.

    Public member variables:

    See structural_mechanics_solver.py for more information.
    """

    def __init__(self, model, custom_settings):

        self._validate_settings_in_baseclass=True # To be removed eventually

        # Construct the base solver.
        super().__init__(model, custom_settings)

        self.mpc_contact_settings = self.settings["mpc_contact_settings"]
        self.mpc_contact_settings.RecursivelyAddMissingParameters(GetDefaults()["mpc_contact_settings"])

        # Setting the parameters
        auxiliar_methods_solvers.AuxiliarMPCSetSettings(self.settings, self.mpc_contact_settings)

        # Logger
        KratosMultiphysics.Logger.PrintInfo("::[MPCContactImplicitMechanicalSolver]:: ", "Construction finished")

    def ValidateSettings(self):
        """This function validates the settings of the solver
        """
        auxiliar_methods_solvers.AuxiliarValidateSettings(self)

    def AddVariables(self):

        super().AddVariables()

        # We add the contact related variables
        contact_type = self.mpc_contact_settings["contact_type"].GetString()
        auxiliar_methods_solvers.AuxiliarMPCAddVariables(self.main_model_part, contact_type)

    def Initialize(self):
        KratosMultiphysics.Logger.PrintInfo("::[MPCContactImplicitMechanicalSolver]:: ", "Initializing ...")

        super().Initialize() # The mechanical solver is created here.

        # We set the flag INTERACTION
        if self.mpc_contact_settings["simplified_semi_smooth_newton"].GetBool():
            computing_model_part = self.GetComputingModelPart()
            computing_model_part.ProcessInfo.Set(KratosMultiphysics.INTERACTION, True)

        KratosMultiphysics.Logger.PrintInfo("::[MPCContactImplicitMechanicalSolver]:: ", "Finished initialization.")

    def ComputeDeltaTime(self):
        return auxiliar_methods_solvers.AuxiliarComputeDeltaTime(self.main_model_part, self.GetComputingModelPart(), self.settings, self.mpc_contact_settings)

    #### Private functions ####

    def _create_convergence_criterion(self):
        convergence_criterion = convergence_criteria_factory.convergence_criterion(self._get_convergence_criterion_settings())
        conv_criteria = convergence_criterion.mechanical_convergence_criterion
        contact_criteria = ContactStructuralMechanicsApplication.MPCContactCriteria()
        return KratosMultiphysics.AndCriteria(conv_criteria, contact_criteria)

    def _create_mechanical_solution_strategy(self):
        mechanical_solution_strategy = self._create_contact_newton_raphson_strategy()
        return mechanical_solution_strategy

    def _create_contact_newton_raphson_strategy(self):
        computing_model_part = self.GetComputingModelPart()
        self.mechanical_scheme = self.get_solution_scheme()
        self.linear_solver = self.get_linear_solver()
        self.mechanical_convergence_criterion = self.get_convergence_criterion()
        self.builder_and_solver = self.get_builder_and_solver()
        return auxiliar_methods_solvers.AuxiliarMPCNewton(computing_model_part, self.mechanical_scheme, self.linear_solver, self.mechanical_convergence_criterion, self.builder_and_solver, self.settings, self.mpc_contact_settings)

    @classmethod
    def GetDefaultParameters(cls):
        this_defaults = GetDefaults()
        this_defaults.RecursivelyAddMissingParameters(super(MPCContactImplicitMechanicalSolver, cls).GetDefaultParameters())
        return this_defaults
