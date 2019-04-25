import KratosMultiphysics
import KratosMultiphysics.CompressiblePotentialFlowApplication as CompressiblePotentialFlow
import KratosMultiphysics.MeshingApplication as MeshingApplication
import math
import time


def Factory(settings, Model):
    if( not isinstance(settings,KratosMultiphysics.Parameters) ):
        raise Exception("expected input shall be a Parameters object, encapsulating a json string")
    return InitializeGeometryProcess(Model, settings["Parameters"])
def MoveModelPart(origin, skin_model_part):
    for node in skin_model_part.Nodes:
        node.X=origin[0]+node.X
        node.Y=origin[1]+node.Y
def RotateModelPart(origin, angle, model_part):
    ox,oy=origin
    for node in model_part.Nodes:
        node.X = ox+math.cos(angle)*(node.X - ox)-math.sin(angle)*(node.Y - oy)
        node.Y = oy+math.sin(angle)*(node.X - ox)+math.cos(angle)*(node.Y - oy)
## All the processes python should be derived from "Process"
class InitializeGeometryProcess(KratosMultiphysics.Process):
    def __init__(self, Model, settings ):
        KratosMultiphysics.Process.__init__(self)
        KratosMultiphysics.Logger.PrintInfo('InitializeGeometry','Initialize Geometry process')
        default_parameters = KratosMultiphysics.Parameters( """
            {
                "model_part_name": "insert_model_part",
                "skin_model_part_name": "insert_skin_model_part",
                "geometry_parameter": 0.0,
                "maximum_iterations": 1,
                "print_output": false,
                "remeshing_flag": false,
                "initial_point": [0,0],
                "metric_parameters":  {
                    "minimal_size"                         : 5e-3,
                    "maximal_size"                         : 1.0,
                    "sizing_parameters": {
                        "reference_variable_name"               : "DISTANCE",
                        "boundary_layer_max_distance"           : 1.0,
                        "interpolation"                         : "constant"
                    },
                    "enforce_current"                      : true,
                    "anisotropy_remeshing"                 : false,
                    "anisotropy_parameters": {
                        "reference_variable_name"          : "DISTANCE",
                        "hmin_over_hmax_anisotropic_ratio"      : 0.5,
                        "boundary_layer_max_distance"           : 1,
                        "interpolation"                         : "Linear"
                    }
                }
            }  """ );
        settings.ValidateAndAssignDefaults(default_parameters)
        self.model=Model
        self.main_model_part = Model.GetModelPart(settings["model_part_name"].GetString()).GetRootModelPart()
        self.geometry_parameter = settings["geometry_parameter"].GetDouble()
        self.do_remeshing = settings["remeshing_flag"].GetBool()
        self.initial_point = settings["initial_point"].GetVector()
        self.skin_model_part=self.model.CreateModelPart("skin")
        self.skin_model_part_name=settings["skin_model_part_name"].GetString()
        self.boundary_model_part = self.main_model_part.CreateSubModelPart("boundary_model_part")
        KratosMultiphysics.ModelPartIO(self.skin_model_part_name).ReadModelPart(self.skin_model_part)

        '''Loop parameters'''
        self.step = 0
        self.max_iter = settings["maximum_iterations"].GetInt()
        self.print_output_flag = settings["print_output"].GetBool()

        self.MetricParameters = settings["metric_parameters"]
        # We set to zero the metric
        ZeroVector = [0.0,0.0,0.0]
        KratosMultiphysics.VariableUtils().SetVectorVar(MeshingApplication.METRIC_TENSOR_2D, ZeroVector, self.main_model_part.Nodes)


        ''' Defining linear solver to be used by the variational distance process'''
        import python_linear_solver_factory #Linear solver for variational distance process
        linear_solver_settings=KratosMultiphysics.Parameters("""
        {
            "solver_type": "amgcl",
            "max_iteration": 400,
            "gmres_krylov_space_dimension": 100,
            "smoother_type":"ilu0",
            "coarsening_type":"ruge_stuben",
            "coarse_enough" : 5000,
            "krylov_type": "lgmres",
            "tolerance": 1e-9,
            "verbosity": 0,
            "scaling": false
        }""")

        self.linear_solver = python_linear_solver_factory.ConstructSolver(linear_solver_settings)

    def Execute(self):
        KratosMultiphysics.Logger.PrintInfo('Executing Initialize Geometry')
        self.InitializeSkinModelPart()
        self.CalculateDistance()

        ini_time=time.time()
        while self.step < self.max_iter and self.do_remeshing:
            self.step += 1
            KratosMultiphysics.Logger.PrintInfo('InitializeGeometry','##### Executing refinement #', self.step, ' #####')
            self.ExtendDistance()
            self.RefineMesh()
            self.CalculateDistance()
            self.UpdateParameters()

        KratosMultiphysics.Logger.PrintInfo('InitializeGeometry','Elapsed time: ',time.time()-ini_time)

        KratosMultiphysics.VariableUtils().CopyScalarVar(KratosMultiphysics.DISTANCE,CompressiblePotentialFlow.LEVEL_SET, self.main_model_part.Nodes)
        KratosMultiphysics.VariableUtils().SetScalarVar(KratosMultiphysics.DISTANCE, 0.0, self.main_model_part.Nodes)
        self.ApplyFlags()

        ''' ############################################################################################## '''
        ''' THESE FUNCTION CALLS ARE TEMPORARY AND WILL BE REMOVED ONCE THE EMBEDDED WAKE PROCESS IS DEFINED '''
        KratosMultiphysics.NormalCalculationUtils().CalculateOnSimplex(self.main_model_part,self.main_model_part.ProcessInfo[KratosMultiphysics.DOMAIN_SIZE])
        # Find nodal neigbours util call
        avg_elem_num = 10
        avg_node_num = 10
        KratosMultiphysics.FindNodalNeighboursProcess(
            self.main_model_part, avg_elem_num, avg_node_num).Execute()
        ''' ############################################################################################## '''

        KratosMultiphysics.Logger.PrintInfo('InitializeGeometry','Level Set geometry initialized')

    def ExecuteInitialize(self):
        self.Execute()

    def InitializeSkinModelPart(self):
        ''' This function moves the skin_model_part in the main_model_part to the desired initial point (self.initial_point).
            It also rotates the skin model part around the initial point according to the (self.geometry_parameter)'''
        ini_time=time.time()
        if self.skin_model_part_name=='naca0012':
            MoveModelPart(self.initial_point, self.skin_model_part)
            self.origin=[0.25+self.initial_point[0],0+self.initial_point[1]]
            angle=math.radians(-self.geometry_parameter)
            RotateModelPart(self.origin,angle,self.skin_model_part)
        elif self.skin_model_part_name=='ellipse':
            MoveModelPart(self.initial_point, self.skin_model_part)
            self.origin=[self.initial_point[0],self.initial_point[1]]
            angle=math.radians(-self.geometry_parameter)
            RotateModelPart(self.origin,angle,self.skin_model_part)
        KratosMultiphysics.Logger.PrintInfo('InitializeGeometry','InitializeSkin time: ',time.time()-ini_time)

    def CalculateDistance(self):
        ''' This function calculate the distance to skin for every node in the main_model_part.'''
        ini_time=time.time()
        KratosMultiphysics.CalculateDistanceToSkinProcess2D(self.main_model_part, self.skin_model_part).Execute()
        KratosMultiphysics.Logger.PrintInfo('InitializeGeometry','CalculateDistance time: ',time.time()-ini_time)

    def ExtendDistance(self):
        ''' This function extends the distance field to all the nodes of the main_model_part in order to
            remesh the background mesh.'''
        ini_time=time.time()
        # Construct the variational distance calculation process
        maximum_iterations = 2 #TODO: Make this user-definable
        if self.main_model_part.ProcessInfo[KratosMultiphysics.DOMAIN_SIZE] == 2:
            variational_distance_process = KratosMultiphysics.VariationalDistanceCalculationProcess2D(
                self.main_model_part,
                self.linear_solver,
                maximum_iterations)
        else:
            variational_distance_process = KratosMultiphysics.VariationalDistanceCalculationProcess3D(
                self.main_model_part,
                self.linear_solver,
                maximum_iterations)
        variational_distance_process.Execute()
        KratosMultiphysics.Logger.PrintInfo('InitializeGeometry','Variational distance process time: ',time.time()-ini_time)


    def RefineMesh(self):
        ''' This function remeshes the main_model_part according to the distance, using the MMG process from the MeshingApplication.
            In order to perform the refinement, it is needed to calculate the distance gradient, the initial nodal_h and the level_set metric.
        '''
        ini_time=time.time()
        local_gradient = KratosMultiphysics.ComputeNodalGradientProcess2D(self.main_model_part, KratosMultiphysics.DISTANCE, KratosMultiphysics.DISTANCE_GRADIENT, KratosMultiphysics.NODAL_AREA)
        local_gradient.Execute()

        find_nodal_h = KratosMultiphysics.FindNodalHNonHistoricalProcess(self.main_model_part)
        find_nodal_h.Execute()

        metric_process = MeshingApplication.ComputeLevelSetSolMetricProcess2D(self.main_model_part,  KratosMultiphysics.DISTANCE_GRADIENT, self.MetricParameters)
        metric_process.Execute()

        self.PrintOutput('metric_output'+str(self.step))

        mmg_parameters = KratosMultiphysics.Parameters("""
        {
            "discretization_type"                  : "STANDARD",
            "save_external_files"              : false,
            "initialize_entities"              : false,
            "echo_level"                       : 0
        }
        """)

        mmg_process = MeshingApplication.MmgProcess2D(self.main_model_part, mmg_parameters)
        mmg_process.Execute()

        KratosMultiphysics.Logger.PrintInfo('InitializeGeometry','Remesh time: ',time.time()-ini_time)
        self.PrintOutput('remeshed_output'+str(self.step))

    def UpdateParameters(self):
        ''' This process updates remeshing parameters in case more than one iteration is performed'''
        previous_size=self.MetricParameters["minimal_size"].GetDouble()
        self.MetricParameters["minimal_size"].SetDouble(previous_size*0.5)

    def ApplyFlags(self):
        ''' This process finds the elements that are cut and the elements that lie inside the geometry.
        '''
        ini_time = time.time()
        KratosMultiphysics.CompressiblePotentialFlowApplication.ApplyEmbeddedFlagsProcess(self.main_model_part).Execute()
        KratosMultiphysics.Logger.PrintInfo('InitializeGeometry','Apply flags time: ',time.time()-ini_time)

    def PrintOutput(self,filename):
        if self.print_output_flag:
            from gid_output_process import GiDOutputProcess
            gid_output = GiDOutputProcess(self.main_model_part,
                                        filename,
                                        KratosMultiphysics.Parameters("""
                                            {
                                                "result_file_configuration" : {
                                                    "gidpost_flags": {
                                                        "GiDPostMode": "GiD_PostBinary",
                                                        "WriteDeformedMeshFlag": "WriteUndeformed",
                                                        "WriteConditionsFlag": "WriteConditions",
                                                        "MultiFileFlag": "SingleFile"
                                                    },
                                                    "nodal_results" : ["DISTANCE"],
                                                    "nodal_nonhistorical_results": ["METRIC_TENSOR_2D"]

                                                }
                                            }
                                            """)
                                        )

            gid_output.ExecuteInitialize()
            gid_output.ExecuteBeforeSolutionLoop()
            gid_output.ExecuteInitializeSolutionStep()
            gid_output.PrintOutput()
            gid_output.ExecuteFinalizeSolutionStep()
            gid_output.ExecuteFinalize()



