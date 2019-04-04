from __future__ import print_function, absolute_import, division  # makes KratosMultiphysics backward compatible with python 2.6 and 2.7

# Importing Kratos
import KratosMultiphysics as KM

# Other imports
import sys

# Import the base structural analysis
from KratosMultiphysics.StructuralMechanicsApplication.structural_mechanics_analysis import StructuralMechanicsAnalysis as BaseClass

# Import auxiliar methods
import KratosMultiphysics.auxiliar_methods_adaptative_remeshing as auxiliar_methods_adaptative_remeshing

class AdaptativeRemeshingStructuralMechanicsAnalysis(BaseClass):
    """
    This class is the main-script of the StructuralMechanicsApplication when using adaptative remeshing put in a class

    It can be imported and used as "black-box"
    """
    def __init__(self, model, project_parameters):

        # Construct the base analysis.
        default_params = KM.Parameters("""
        {
            "max_iteration" : 1,
            "analysis_type" : "linear"
        }
        """)
        if project_parameters["solver_settings"].Has("max_iteration"):
            self.non_linear_iterations = project_parameters["solver_settings"]["max_iteration"].GetInt()
        else:
            self.non_linear_iterations = 10
            project_parameters["solver_settings"].AddValue("max_iteration", default_params["max_iteration"])
        if project_parameters["solver_settings"].Has("analysis_type"):
            project_parameters["solver_settings"]["analysis_type"].SetString("linear")
        else:
            project_parameters["solver_settings"].AddValue("analysis_type", default_params["analysis_type"])
        self.process_remesh = False
        if project_parameters.Has("mesh_adaptivity_processes"):
            self.process_remesh = True
        if project_parameters.Has("processes"):
            if project_parameters["processes"].Has("mesh_adaptivity_processes"):
                self.process_remesh = True
        super(AdaptativeRemeshingStructuralMechanicsAnalysis, self).__init__(model, project_parameters)

    def Initialize(self):
        """ Initializing the Analysis """
        super(AdaptativeRemeshingStructuralMechanicsAnalysis, self).Initialize()
        auxiliar_methods_adaptative_remeshing.AdaptativeRemeshingDetectBoundary(self)

    def RunSolutionLoop(self):
        """This function executes the solution loop of the AnalysisStage
        It can be overridden by derived classes
        """

        # If we remesh using a process
        if self.process_remesh:
            auxiliar_methods_adaptative_remeshing.AdaptativeRemeshingRunSolutionLoop(self)
        else: # Remeshing adaptively
            auxiliar_methods_adaptative_remeshing.SPRAdaptativeRemeshingRunSolutionLoop(self)

    #### Internal functions ####
    def _CreateSolver(self):
        """ Create the Solver (and create and import the ModelPart if it is not alread in the model) """

        # To avoid many prints
        if self.echo_level == 0:
            KM.Logger.GetDefaultOutput().SetSeverity(KM.Logger.Severity.WARNING)

        ## Solver construction
        from KratosMultiphysics.StructuralMechanicsApplication import python_solvers_wrapper_adaptative_remeshing_structural
        return python_solvers_wrapper_adaptative_remeshing_structural.CreateSolver(self.model, self.project_parameters)

    def _CreateProcesses(self, parameter_name, initialization_order):
        """Create a list of Processes
        This method is TEMPORARY to not break existing code
        It will be removed in the future
        """
        list_of_processes = super(AdaptativeRemeshingStructuralMechanicsAnalysis, self)._CreateProcesses(parameter_name, initialization_order)

        if parameter_name == "processes":
            processes_block_names = ["mesh_adaptivity_processes"]
            if len(list_of_processes) == 0: # Processes are given in the old format
                KM.Logger.PrintWarning("AdaptativeRemeshingStructuralMechanicsAnalysis", "Using the old way to create the processes, this will be removed!")
                from KratosMultiphysics.process_factory import KratosProcessFactory
                factory = KratosProcessFactory(self.model)
                for process_name in processes_block_names:
                    if self.project_parameters.Has(process_name):
                        list_of_processes += factory.ConstructListOfProcesses(self.project_parameters[process_name])
            else: # Processes are given in the new format
                for process_name in processes_block_names:
                    if self.project_parameters.Has(process_name):
                        raise Exception("Mixing of process initialization is not allowed!")
        elif parameter_name == "output_processes":
            pass # Already added
        else:
            raise NameError("wrong parameter name")

        return list_of_processes

if __name__ == "__main__":
    from sys import argv

    if len(argv) > 2:
        err_msg =  'Too many input arguments!\n'
        err_msg += 'Use this script in the following way:\n'
        err_msg += '- With default ProjectParameters (read from "ProjectParameters.json"):\n'
        err_msg += '    "python3 adaptative_remeshing_structural_mechanics_analysis.py"\n'
        err_msg += '- With custom ProjectParameters:\n'
        err_msg += '    "python3 adaptative_remeshing_structural_mechanics_analysis.py CustomProjectParameters.json"\n'
        raise Exception(err_msg)

    if len(argv) == 2: # ProjectParameters is being passed from outside
        project_parameters_file_name = argv[1]
    else: # using default name
        project_parameters_file_name = "ProjectParameters.json"

    AdaptativeRemeshingStructuralMechanicsAnalysis(project_parameters_file_name).Run()
