import KratosMultiphysics
import KratosMultiphysics.KratosUnittest as UnitTest
import KratosMultiphysics.ChimeraApplication
from fluid_chimera_analysis import FluidChimeraAnalysis
import KratosMultiphysics.kratos_utilities as kratos_utilities
import os
try:
    import KratosMultiphysics.ExternalSolversApplication
    have_external_solvers = True
except ImportError:
    have_external_solvers = False

def GetFilePath(fileName):
    return os.path.join(os.path.dirname(os.path.realpath(__file__)), fileName)

@UnitTest.skipUnless(have_external_solvers,"Missing required application: ExternalSolversApplication")
class ChimeraAnalysisBaseTest(UnitTest.TestCase):

    def setUp(self):
        # Set to true to get post-process files for the test
        self.print_output = True

    def _run_test(self,settings_file_name):
        model = KratosMultiphysics.Model()
        with open(settings_file_name,'r') as settings_file:
            settings = KratosMultiphysics.Parameters(settings_file.read())
        # to check the results: add output settings block if needed
        if self.print_output:
            settings.AddValue("output_processes", KratosMultiphysics.Parameters(r'''{
                "vtk_output" : [{
                    "python_module" : "vtk_output_process",
                    "kratos_module" : "KratosMultiphysics",
                    "process_name"  : "VtkOutputProcess",
                    "help"          : "This process writes postprocessing files for Paraview",
                    "Parameters"    : {
                        "model_part_name"                    : "FluidModelPart",
                        "output_control_type"                : "step",
                        "output_frequency"                   : 1,
                        "file_format"                        : "ascii",
                        "output_precision"                   : 3,
                        "output_sub_model_parts"             : false,
                        "folder_name"                        : "test_vtk_output",
                        "save_output_files_in_folder"        : true,
                        "nodal_solution_step_data_variables" : ["VELOCITY","PRESSURE","DISTANCE"],
                        "nodal_data_value_variables"         : [],
                        "element_flags"                      : ["ACTIVE"],
                        "element_data_value_variables"       : [],
                        "condition_data_value_variables"     : []
                    }
                }]
            }'''))

        analysis = FluidChimeraAnalysis(model,settings)
        analysis.Run()
