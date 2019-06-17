from __future__ import print_function, absolute_import, division  # makes these scripts backward compatible with python 2.6 and 2.7

# CoSimulation imports
import KratosMultiphysics.CoSimulationApplication.co_simulation_tools as cs_tools
import KratosMultiphysics.CoSimulationApplication.colors as colors

class CoSimulationConvergenceCriteria(object):
    def __init__(self, settings, solver_wrapper):
        self.settings = settings
        self.settings.RecursivelyValidateAndAssignDefaults(self._GetDefaultSettings())

        self.interface_data = solver_wrapper.GetInterfaceData(self.settings["data_name"].GetString())

        self.echo_level = self.settings["echo_level"].GetInt()
        self.abs_tolerance = self.settings["abs_tolerance"].GetDouble()
        self.rel_tolerance = self.settings["rel_tolerance"].GetDouble()


    def Initialize(self):
        pass

    def Finalize(self):
        pass

    def InitializeSolutionStep(self):
        pass

    def FinalizeSolutionStep(self):
        pass

    def InitializeCouplingIteration(self):
        pass

    def FinalizeCouplingIteration(self):
        pass

    def IsConverged(self):
        raise NotImplementedError('"IsConverged" has to be implemented in the derived class!')

    def PrintInfo(self):
        cs_tools.cs_print_info("Convergence Criteria", colors.bold(self._Name()))

    def Check(self):
        print("ConvCrit does not implement Check yet!")

    def _Name(self):
        return self.__class__.__name__

    @classmethod
    def _GetDefaultSettings(cls):
        return cs_tools.cs_data_structure.Parameters("""{
            "type"       : "UNSPECIFIED",
            "solver"     : "UNSPECIFIED",
            "data_name"  : "UNSPECIFIED",
            "abs_tolerance" : 1e-5,
            "rel_tolerance" : 1e-5,
            "echo_level" : 0
        }""")
