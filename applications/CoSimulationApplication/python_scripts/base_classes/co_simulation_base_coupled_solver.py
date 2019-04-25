from __future__ import print_function, absolute_import, division

# Importing the base class
from  . import co_simulation_base_solver

# Other imports
import KratosMultiphysics.CoSimulationApplication.co_simulation_tools as cs_tools
from KratosMultiphysics.CoSimulationApplication.co_simulation_tools import couplingsolverprint, bold

class CoSimulationBaseCouplingSolver(co_simulation_base_solver.CoSimulationBaseSolver):
    def __init__(self, model, cosim_solver_settings, solver_name):
        super(CoSimulationBaseCouplingSolver, self).__init__(model, cosim_solver_settings, solver_name)

        self.participating_solvers = self.__CreateSolvers()
        self.coupling_sequence = self.__GetSolverCoSimulationDetails()

        ### Creating the predictors
        self.predictors_list = cs_tools.CreatePredictors(
            self.cosim_solver_settings["predictors"],
            self.participating_solvers,
            self.echo_level)

    def Initialize(self):
        for solver in self.participating_solvers.values():
            solver.Initialize()
        for solver in self.participating_solvers.values():
            solver.InitializeIO(self.participating_solvers, self.echo_level)
            # we use the Echo_level of the coupling solver, since IO is needed by the coupling
            # and not by the (physics-) solver

        for predictor in self.predictors_list:
            predictor.Initialize()

    def Finalize(self):
        for solver in self.participating_solvers.values():
            solver.Finalize()

        for predictor in self.predictors_list:
            predictor.Finalize()

    def AdvanceInTime(self, current_time):
        self.time = 0.0
        for solver in self.participating_solvers.values():
            self.time = max(self.time, solver.AdvanceInTime(current_time))

        return self.time

    def Predict(self):
        for predictor in self.predictors_list:
            predictor.Predict()

        for solver in self.participating_solvers.values():
            solver.Predict()

    def InitializeSolutionStep(self):
        for solver in self.participating_solvers.values():
            solver.InitializeSolutionStep()

        for predictor in self.predictors_list:
            predictor.InitializeSolutionStep()

    def FinalizeSolutionStep(self):
        for solver in self.participating_solvers.values():
            solver.FinalizeSolutionStep()
        for predictor in self.predictors_list:
            predictor.FinalizeSolutionStep()

    def OutputSolutionStep(self):
        for solver in self.participating_solvers.values():
            solver.OutputSolutionStep()

    def SolveSolutionStep(self):
        err_msg  = 'Calling "SolveSolutionStep" of the "CoSimulationBaseCouplingSolver"!\n'
        err_msg += 'This function has to be implemented in the derived class!'
        raise Exception(err_msg)

    def _SynchronizeInputData(self, solver_name):
        solver = self.participating_solvers[solver_name]
        input_data_list = self.coupling_sequence[solver_name]["input_data_list"]

        for i in range(input_data_list.size()):
            input_data = input_data_list[i]
            from_solver = self.participating_solvers[input_data["from_solver"].GetString()]
            data_name = input_data["data_name"].GetString()
            from_solver_data = from_solver.GetInterfaceData(data_name)
            solver_data = solver.GetInterfaceData(input_data["destination_data"].GetString())

            if input_data.Has("mapper_settings"):
                solver_data.origin_data = from_solver_data
                solver_data.mapper_settings = input_data["mapper_settings"]

            solver.ImportCouplingInterfaceData(solver_data, from_solver)

    def _SynchronizeOutputData(self, solver_name):
        solver = self.participating_solvers[solver_name]
        output_data_list = self.coupling_sequence[solver_name]["output_data_list"]

        for i in range(output_data_list.size()):
            output_data = output_data_list[i]
            to_solver = self.participating_solvers[output_data["to_solver"].GetString()]
            data_name = output_data["data_name"].GetString()
            to_solver_data = to_solver.GetInterfaceData(data_name)
            solver_data = solver.GetInterfaceData(output_data["origin_data"].GetString())

            if output_data.Has("mapper_settings"):
                solver_data.destination_data = to_solver_data
                solver_data.mapper_settings = output_data["mapper_settings"]

            solver.ExportCouplingInterfaceData(solver_data, to_solver)

    def PrintInfo(self):
        super(CoSimulationBaseCouplingSolver, self).PrintInfo()

        couplingsolverprint(self._Name(), "Has the following participants:")

        for solver in self.participating_solvers.values():
            solver.PrintInfo()

        # if self.predictor is not None:
        #     couplingsolverprint(self._Name(), "Uses a Predictor:")
        #     self.predictor.PrintInfo()

    def Check(self):
        super(CoSimulationBaseCouplingSolver, self).Check()

        for solver in self.participating_solvers.values():
            solver.Check()

        # if self.predictor is not None:
        #     self.predictor.Check()

    def IsDistributed(self):
        return True

    def _GetIOName(self):
        # the coupled-solvers always use the kratos-format, since there are no "external" coupled solvers
        return "kratos"

    def __CreateSolvers(self):
        ### ATTENTION, big flaw, also the participants can be coupled solvers !!!
        import KratosMultiphysics.CoSimulationApplication.solver_interfaces.co_simulation_solver_factory as solver_factory
        from collections import OrderedDict
        # first create all solvers
        solvers = {}
        for solver_name, solver_settings in self.cosim_solver_settings["solvers"].items():
            solvers[solver_name] = solver_factory.CreateSolverInterface(
                self.model, solver_settings, solver_name)

        # then order them according to the coupling-loop
        # NOTE solvers that are not used in the coupling-sequence will not participate
        solvers_map = OrderedDict()
        for i_solver_settings in range(self.cosim_solver_settings["coupling_sequence"].size()):
            solver_settings = self.cosim_solver_settings["coupling_sequence"][i_solver_settings]
            solver_name = solver_settings["name"].GetString()
            solvers_map[solver_name] = solvers[solver_name]

        return solvers_map

    def __GetSolverCoSimulationDetails(self):
        solver_cosim_details = {}
        for i_solver_settings in range(self.cosim_solver_settings["coupling_sequence"].size()):
            solver_settings = self.cosim_solver_settings["coupling_sequence"][i_solver_settings]
            solver_name = solver_settings["name"].GetString()
            solver_cosim_details[solver_name] = solver_settings
        return solver_cosim_details
