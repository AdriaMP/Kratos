from __future__ import print_function, absolute_import, division

import KratosMultiphysics as Kratos
from python_solver import PythonSolver
from kratos_utilities import CheckIfApplicationsAvailable

if CheckIfApplicationsAvailable("RANSConstitutiveLawsApplication"):
    import KratosMultiphysics.RANSConstitutiveLawsApplication as KratosRANS


def CreateTurbulenceModel(model, settings):
    if not CheckIfApplicationsAvailable("RANSConstitutiveLawsApplication"):
        msg = "Using a turbulence model requires the RANSConstitutiveLawsApplication. "
        msg += "Please re-install/re-compile with RANSConstitutiveLawsApplication."
        raise Exception(msg)

    from turbulence_model_factory import Factory
    return Factory(model, settings)

class TurbulenceModelConfiguration(PythonSolver):
    def AddVariables(self):
        msg = "Calling the base TurbulenceModelConfiguration class AddVariables method."
        msg += " Please override it in the derrived class."
        raise Exception(msg)

    def AddDofs(self):
        msg = "Calling the base TurbulenceModelConfiguration class AddDofs method."
        msg += " Please override it in the derrived class."
        raise Exception(msg)

    def PrepareModelPart(self):
        msg = "Calling the base TurbulenceModelConfiguration class PrepareModelPart method."
        msg += " Please override it in the derrived class."
        raise Exception(msg)

    def GetTurbulenceSolvingProcess(self):
        msg = "Calling the base TurbulenceModelConfiguration class GetTurbulenceSolvingProcess method."
        msg += " Please override it in the derrived class to return a KratosMultiphysics.Process."
        raise Exception(msg)

    def Initialize(self):
        self.GetTurbulenceSolvingProcess().ExecuteInitialize()

    def Check(self):
        self.GetTurbulenceSolvingProcess().Check()

    def InitializeSolutionStep(self):
        self.GetTurbulenceSolvingProcess().ExecuteInitializeSolutionStep()

    def FinalizeSolutionStep(self):
        self.GetTurbulenceSolvingProcess().ExecuteFinalizeSolutionStep()