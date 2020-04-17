from __future__ import absolute_import, division  # makes KratosMultiphysics backward compatible with python 2.6 and 2.7

# Importing the Kratos Library
import KratosMultiphysics as Kratos
from KratosMultiphysics import IsDistributedRun
from KratosMultiphysics.kratos_utilities import CheckIfApplicationsAvailable
from KratosMultiphysics.python_solver import PythonSolver

# Import applications
import KratosMultiphysics.FluidDynamicsApplication as KratosCFD
import KratosMultiphysics.RANSApplication as KratosRANS

# Import application specific modules
from KratosMultiphysics.RANSApplication.formulations import Factory as FomulationFactory
from KratosMultiphysics.RANSApplication import RansVariableUtilities
from KratosMultiphysics.FluidDynamicsApplication.check_and_prepare_model_process_fluid import CheckAndPrepareModelProcess

# case specific imports
if (IsDistributedRun() and CheckIfApplicationsAvailable("TrilinosApplication")):
    import KratosMultiphysics.TrilinosApplication as KratosTrilinos     # MPI solvers
    from KratosMultiphysics.mpi.distributed_import_model_part_utility import DistributedImportModelPartUtility
elif (IsDistributedRun()):
    raise Exception("Distributed run requires TrilinosApplication")

class CoupledRANSSolver(PythonSolver):
    @classmethod
    def GetDefaultSettings(cls):
        ##settings string in json format
        default_settings = Kratos.Parameters("""
        {
            "solver_type": "CoupledRANS",
            "model_part_name": "FluidModelPart",
            "domain_size": -1,
            "model_import_settings": {
                "input_type": "mdpa",
                "input_filename": "unknown_name",
                "reorder": false
            },
            "material_import_settings": {
                "materials_filename": ""
            },
            "max_iterations": 1,
            "consider_periodic_conditions": false,
            "formulation_settings": {},
            "echo_level": 0,
            "volume_model_part_name": "volume_model_part",
            "skin_parts"   : [""],
            "no_skin_parts": [""],
            "assign_neighbour_elements_to_conditions": true,
            "move_mesh": false,
            "time_scheme_settings":{
                "scheme_type": "steady"
            },
            "time_stepping": {
                "automatic_time_step" : false,
                "CFL_number"          : 1,
                "minimum_delta_time"  : 1e-4,
                "maximum_delta_time"  : 0.01,
                "time_step"           : 0.0
            },
            "constants": {}
        }""")

        default_settings.AddMissingParameters(
            super(CoupledRANSSolver, cls).GetDefaultSettings())
        return default_settings

    def __init__(self, model, custom_settings):
        self._validate_settings_in_baseclass = True  # To be removed eventually
        super(CoupledRANSSolver, self).__init__(model, custom_settings)

        model_part_name = self.settings["model_part_name"].GetString()
        if model_part_name == "":
            raise Exception(
                'Please provide the model part name as the "model_part_name" (string) parameter!'
            )

        if self.model.HasModelPart(model_part_name):
            self.main_model_part = self.model.GetModelPart(model_part_name)
        else:
            self.main_model_part = self.model.CreateModelPart(model_part_name)

        self.domain_size = self.settings["domain_size"].GetInt()
        if self.domain_size == -1:
            raise Exception(
                'Please provide the domain size as the "domain_size" (int) parameter!'
            )
        self.main_model_part.ProcessInfo.SetValue(Kratos.DOMAIN_SIZE,
                                                  self.domain_size)

        self.formulation = FomulationFactory(self.main_model_part,
                                self.settings["formulation_settings"])

        self.formulation.SetConstants(self.settings["constants"])
        self.formulation.SetIsPeriodic(self.settings["consider_periodic_conditions"].GetBool())
        self.formulation.SetMaxCouplingIterations(self.settings["max_iterations"].GetInt())

        self.is_periodic = self.formulation.IsPeriodic()

        self.formulation.SetTimeSchemeSettings(self.settings["time_scheme_settings"])
        scheme_type = self.settings["time_scheme_settings"]["scheme_type"].GetString()
        if (scheme_type == "steady"):
            self.is_steady = True
        else:
            self.is_steady = False

        self.is_converged = False
        self.min_buffer_size = self.formulation.GetMinimumBufferSize()
        self.move_mesh = self.settings["move_mesh"].GetBool()

        Kratos.Logger.PrintInfo(self.__class__.__name__,
                                            "Solver construction finished.")

    def AddVariables(self):
        self.formulation.AddVariables()

        if self.is_periodic:
            self.GetBaseModelPart().AddNodalSolutionStepVariable(KratosCFD.PATCH_INDEX)

        if (IsDistributedRun()):
            self.main_model_part.AddNodalSolutionStepVariable(Kratos.PARTITION_INDEX)

        Kratos.Logger.PrintInfo(
            self.__class__.__name__, "Solver variables added correctly.")

    def AddDofs(self):
        self.formulation.AddDofs()

        Kratos.Logger.PrintInfo(self.__class__.__name__,
                                            "Solver dofs added correctly.")

    def ImportModelPart(self):
        if (IsDistributedRun()):
            ## Construct the MPI import model part utility
            self.distributed_model_part_importer = DistributedImportModelPartUtility(self.main_model_part, self.settings)
            ## Execute the Metis partitioning and reading
            self.distributed_model_part_importer.ImportModelPart()
        else:
            # we can use the default implementation in the base class
            self._ImportModelPart(self.main_model_part,
                                  self.settings["model_import_settings"])

    def PrepareModelPart(self):
        if not self.main_model_part.ProcessInfo[
                Kratos.IS_RESTARTED]:
            ## Set fluid properties from materials json file
            materials_imported = self._SetPhysicalProperties()
            if not materials_imported:
                Kratos.Logger.PrintWarning(
                    self.__class__.__name__,
                    "Material properties have not been imported. Check \'material_import_settings\' in your ProjectParameters.json."
                )
            ## Executes the check and prepare model process
            self._ExecuteCheckAndPrepare()
            ## Set buffer size
            self.main_model_part.SetBufferSize(self.min_buffer_size)

        if (IsDistributedRun()):
            self.distributed_model_part_importer.CreateCommunicators()

        self.formulation.PrepareModelPart()

        Kratos.Logger.PrintInfo(self.__class__.__name__,
                                            "Model reading finished.")

    def ExportModelPart(self):
        ## Model part writing
        name_out_file = self.settings["model_import_settings"][
            "input_filename"].GetString() + ".out"
        Kratos.ModelPartIO(
            name_out_file,
            Kratos.IO.WRITE).WriteModelPart(self.main_model_part)

        Kratos.Logger.PrintInfo(self.__class__.__name__,
                                            "Model export finished.")

    def GetMinimumBufferSize(self):
        return self.min_buffer_size

    def Initialize(self):
        if (IsDistributedRun()):
            self.EpetraComm = KratosTrilinos.CreateCommunicator()
            self.formulation.SetCommunicator(self.EpetraComm)
        else:
            self.formulation.SetCommunicator(None)

        # If needed, create the estimate time step utility
        if (self.settings["time_stepping"]["automatic_time_step"].GetBool()):
            self.EstimateDeltaTimeUtility = self._GetAutomaticTimeSteppingUtility(
            )

        RansVariableUtilities.AssignBoundaryFlagsToGeometries(self.main_model_part)
        self.formulation.Initialize()

        Kratos.Logger.PrintInfo(self.__class__.__name__, self.formulation.GetInfo())

        Kratos.Logger.PrintInfo(self.__class__.__name__,
                                            "Solver initialization finished.")

    def AdvanceInTime(self, current_time):
        dt = self._ComputeDeltaTime()
        new_time = current_time + dt

        self.main_model_part.CloneTimeStep(new_time)
        self.main_model_part.ProcessInfo[Kratos.STEP] += 1

        return new_time

    def InitializeSolutionStep(self):
        self.formulation.InitializeSolutionStep()

    def SolveSolutionStep(self):
        self.formulation.SolveCouplingStep()
        self.is_converged = self.formulation.IsConverged()

        if not self.is_converged and not self.IsSteadySimulation():
            msg = "Fluid solver did not converge for step " + str(self.main_model_part.ProcessInfo[Kratos.STEP]) + "\n"
            msg += "corresponding to time " + str(self.main_model_part.ProcessInfo[Kratos.TIME]) + "\n"
            Kratos.Logger.PrintWarning(self.__class__.__name__, msg)
        return self.is_converged

    def FinalizeSolutionStep(self):
        self.formulation.FinalizeSolutionStep()

    def Check(self):
        self.formulation.Check()

    def Clear(self):
        self.formulation.Clear()

    def IsSteadySimulation(self):
        return self.is_steady

    def GetComputingModelPart(self):
        if not self.main_model_part.HasSubModelPart(
                "fluid_computational_model_part"):
            raise Exception("The ComputingModelPart was not created yet!")
        return self.main_model_part.GetSubModelPart(
            "fluid_computational_model_part")

    def _ComputeDeltaTime(self):
        # Automatic time step computation according to user defined CFL number
        if (self.settings["time_stepping"]["automatic_time_step"].GetBool()):
            delta_time = self.EstimateDeltaTimeUtility.EstimateDt()
        # User-defined delta time
        else:
            delta_time = self.settings["time_stepping"]["time_step"].GetDouble(
            )

        return delta_time

    def _GetAutomaticTimeSteppingUtility(self):
        if (self.GetComputingModelPart().ProcessInfo[
                Kratos.DOMAIN_SIZE] == 2):
            EstimateDeltaTimeUtility = KratosCFD.EstimateDtUtility2D(
                self.GetComputingModelPart(), self.settings["time_stepping"])
        else:
            EstimateDeltaTimeUtility = KratosCFD.EstimateDtUtility3D(
                self.GetComputingModelPart(), self.settings["time_stepping"])
        return EstimateDeltaTimeUtility

    def _ExecuteCheckAndPrepare(self):
        ## Check that the input read has the shape we like
        prepare_model_part_settings = Kratos.Parameters("{}")
        prepare_model_part_settings.AddValue(
            "volume_model_part_name", self.settings["volume_model_part_name"])
        prepare_model_part_settings.AddValue("skin_parts",
                                             self.settings["skin_parts"])
        if (self.settings.Has("assign_neighbour_elements_to_conditions")):
            prepare_model_part_settings.AddValue(
                "assign_neighbour_elements_to_conditions",
                self.settings["assign_neighbour_elements_to_conditions"])
        else:
            warn_msg = "\"assign_neighbour_elements_to_conditions\" should be added to defaults of " + self.__class__.__name__
            Kratos.Logger.PrintWarning(
                '\n\x1b[1;31mDEPRECATION-WARNING\x1b[0m', warn_msg)

        CheckAndPrepareModelProcess(self.main_model_part,
                                    prepare_model_part_settings).Execute()

    def _SetPhysicalProperties(self):
        # Check if the fluid properties are provided using a .json file
        materials_filename = self.settings["material_import_settings"][
            "materials_filename"].GetString()
        if (materials_filename != ""):
            # Add constitutive laws and material properties from json file to model parts.
            material_settings = Kratos.Parameters(
                """{"Parameters": {"materials_filename": ""}} """)
            material_settings["Parameters"]["materials_filename"].SetString(
                materials_filename)
            Kratos.ReadMaterialsUtility(material_settings,
                                                    self.model)
            materials_imported = True
        else:
            materials_imported = False

        return materials_imported

    def Finalize(self):
        self.formulation.Finalize()