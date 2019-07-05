from __future__ import print_function, absolute_import, division  # makes these scripts backward compatible with python 2.6 and 2.7

# Importing the Kratos Library
import KratosMultiphysics as KM

# CoSimulation imports
import KratosMultiphysics.CoSimulationApplication.factories.io_factory as io_factory
from KratosMultiphysics.CoSimulationApplication.coupling_interface_data import CouplingInterfaceData

def Create(settings, name):
    return CoSimulationSolverWrapper(settings, name)

class CoSimulationSolverWrapper(object):
    """Baseclass for the solver wrappers used for CoSimulation
    It wraps solvers used in the CoSimulation
    """
    def __init__(self, settings, name):
        """Constructor of the Base Solver Wrapper
        Deriving classes should call it in their constructors
        """

        # Every SolverWrapper has its own model, because:
        # - the names can be easily overlapping (e.g. "Structure.Interface")
        # - Solvers should not be able to access the data of other solvers directly!
        self.model = KM.Model()

        self.settings = settings
        self.settings.ValidateAndAssignDefaults(self._GetDefaultSettings())

        self.name = name
        self.echo_level = self.settings["echo_level"].GetInt()
        self.data_dict = self.__CreateInterfaceDataDict()
        # The IO is only used if the corresponding solver is used in coupling and it initialized from the "higher instance, i.e. the coupling-solver
        self.io = None


    def Initialize(self):
        for data in self.data_dict.values():
            data.Initialize()

    def Finalize(self):
        pass

    def AdvanceInTime(self, current_time):
        return current_time + self.settings["time_step"] # needed if this solver is used as dummy

    def Predict(self):
        pass

    def InitializeSolutionStep(self):
        pass

    def FinalizeSolutionStep(self):
        pass

    def OutputSolutionStep(self):
        pass

    def SolveSolutionStep(self):
        pass


    def InitializeIO(self, solvers, io_echo_level):
        if self.__IOIsInitialized():
            raise Exception('IO for "' + self.name + '" is already initialized!')

        io_settings = self.settings["io_settings"]

        if not io_settings.Has("echo_level"):
            io_settings.AddEmptyValue("echo_level").SetInt(self.echo_level)

        self.io = io_factory.CreateIO(self.settings["io_settings"],
                                      self.model,
                                      self._GetIOName())

    def ImportCouplingInterfaceData(self, data_name, from_client=None):
        if not self.__IOIsInitialized():
            raise Exception('IO for "' + self.name + '" is not initialized!')
        self.io.ImportCouplingInterfaceData(data_name, from_client)
    def ImportCouplingInterface(self, geometry_name, from_client=None):
        if not self.__IOIsInitialized():
            raise Exception('IO for "' + self.name + '" is not initialized!')
        self.io.ImportCouplingInterface(geometry_name, from_client)

    def ExportCouplingInterfaceData(self, data_name, to_client=None):
        if not self.__IOIsInitialized():
            raise Exception('IO for "' + self.name + '" is not initialized!')
        self.io.ExportCouplingInterfaceData(data_name, to_client)
    def ExportCouplingInterface(self, geometry_name, to_client=None):
        if not self.__IOIsInitialized():
            raise Exception('IO for "' + self.name + '" is not initialized!')
        self.io.ExportCouplingInterface(geometry_name, to_client)


    def GetInterfaceData(self, data_name):
        try:
            return self.data_dict[data_name]
        except KeyError:
            raise Exception('Requested data field "{}" does not exist for solver "{}"'.format(data_name, self.name))

    def PrintInfo(self):
        '''This function can be filled if desired, e.g. to print settings at higher echo-levels
        '''
        pass

    def _Name(self):
        return self.__class__.__name__

    def Check(self):
        print("!!!WARNING!!! your solver does not implement Check!!!")

    def IsDistributed(self):
        '''Returns whether this solver is executed distributed Aka MPI-parallel
        '''
        # TODO check if this method is necessary!
        return False

    @classmethod
    def _GetIOName(cls):
        # only external solvers have to specify sth here / override this
        return "dummy_io"

    ## __CreateInterfaceDataDict : Private Function to obtain the map of data objects
    #
    #  @param self            The object pointer.
    def __CreateInterfaceDataDict(self):
        data_dict = dict()
        for data_name, data_config in self.settings["data"].items():
            data_dict[data_name] = CouplingInterfaceData(data_config, self.model)

        return data_dict

    def __IOIsInitialized(self):
        return self.io is not None

    @classmethod
    def _GetDefaultSettings(cls):
        return KM.Parameters("""{
            "type"        : "",
            "io_settings" : {},
            "settings"    : {},
            "data"        : {},
            "echo_level"  : 0
        }""")
