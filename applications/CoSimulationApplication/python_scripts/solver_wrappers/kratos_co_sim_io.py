# Importing the Kratos Library
import KratosMultiphysics as KM

# Importing the base class
from KratosMultiphysics.CoSimulationApplication.base_classes.co_simulation_io import CoSimulationIO

# CoSimulation imports
from KratosMultiphysics.CoSimulationApplication import CoSimIO


def Create(model, settings, solver_name):
    return KratosCoSimIO(model, settings, solver_name)

class KratosCoSimIO(CoSimulationIO):
    """Wrapper for the CoSimIO to be used with Kratos
    """
    def __init__(self, settings, model, solver_name):
        super(KratosCoSimIO, self).__init__(settings, model, solver_name)

        CoSimIO.Connect(self.solver_name, CoSimIO.InfoFromParameters(self.settings))

    def Finalize(self):
        CoSimIO.Disconnect(self.solver_name)

    def ImportCouplingInterface(self, interface_config):
        model_part_name = interface_config["model_part_name"]
        CoSimIO.ImportMesh(self.solver_name, model_part_name, self.model[model_part_name]) # TODO this can also be geometry at some point

    def ExportCouplingInterface(self, interface_config):
        model_part_name = interface_config["model_part_name"]
        CoSimIO.ExportMesh(self.solver_name, model_part_name, self.model[model_part_name]) # TODO this can also be geometry at some point

    def ImportData(self, data_config):
        data_type = data_config["type"]
        if data_type == "coupling_interface_data":
            interface_data = data_config["interface_data"]
            CoSimIO.ImportData(self.solver_name, interface_data.name, interface_data.GetModelPart(), interface_data.variable, GetDataLocation(interface_data.location))

        elif data_type == "time":
            time_list = CoSimIO.ImportData(self.solver_name, "time_to_co_sim")
            if len(time_list) != 1:
                raise Exception("Wrong size received!")
            data_config["time"] = time_list[0]
        else:
            raise NotImplementedError('Exporting interface data of type "{}" is not implemented for this IO: "{}"'.format(data_type, self._ClassName()))

    def ExportData(self, data_config):
        data_type = data_config["type"]
        if data_type == "coupling_interface_data":
            interface_data = data_config["interface_data"]
            CoSimIO.ExportData(self.solver_name, interface_data.name, interface_data.GetModelPart(), interface_data.variable, GetDataLocation(interface_data.location))

        elif data_type == "control_signal":
            control_signal_key = data_config["signal"]
            CoSimIO.SendControlSignal(self.solver_name, data_config["identifier"], control_signal_key)

        elif data_type == "time":
            current_time = data_config["time"]
            CoSimIO.ExportData(self.solver_name, "time_from_co_sim", [current_time])

        elif data_type == "convergence_signal":
            if data_config["is_converged"]:
                control_signal_key = CoSimIO.ControlSignal.ConvergenceAchieved
            else:
                control_signal_key = CoSimIO.ControlSignal.Dummy
            CoSimIO.SendControlSignal(self.solver_name, "", control_signal_key)
        else:
            raise NotImplementedError('Exporting interface data of type "{}" is not implemented for this IO: "{}"'.format(data_type, self._ClassName()))

    def PrintInfo(self):
        print("This is the KratosCoSimIO")

    def Check(self):
        pass

    @classmethod
    def _GetDefaultSettings(cls):
        this_defaults = KM.Parameters("""{
            "is_connection_master" : true,
            "communication_format" : "file",
            "print_timing"         : false
        }""")
        this_defaults.AddMissingParameters(super(KratosCoSimIO, cls)._GetDefaultSettings())
        return this_defaults

def GetDataLocation(location_str):
    location_map = {
        "node_historical"     : CoSimIO.DataLocation.NodeHistorical,
        "node_non_historical" : CoSimIO.DataLocation.NodeNonHistorical,
        "element"             : CoSimIO.DataLocation.Element,
        "condition"           : CoSimIO.DataLocation.Condition,
        "model_part"          : CoSimIO.DataLocation.ModelPart
    }
    return location_map[location_str]
