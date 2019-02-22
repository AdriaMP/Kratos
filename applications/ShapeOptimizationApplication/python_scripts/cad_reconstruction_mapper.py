# ==============================================================================
#  KratosShapeOptimizationApplication
#
#  License:         BSD License
#                   license: ShapeOptimizationApplication/license.txt
#
#  Main authors:    Baumgaertner Daniel, https://github.com/dbaumgaertner
#                   Oberbichler Thomas, https://github.com/oberbichler
#
# ==============================================================================

# importing the Kratos Library
import KratosMultiphysics
import KratosMultiphysics.ShapeOptimizationApplication as KratosShape
import KratosMultiphysics.MeshingApplication as KratosMeshingApp
import KratosMultiphysics.StructuralMechanicsApplication as KratosCSM
import KratosMultiphysics.MappingApplication as KratosMapping

# Import helper classes
import cad_reconstruction_conditions as clib
from cad_reconstruction_assembler import Assembler
from cad_reconstruction_condition_factory import ConditionFactory

# Additional imports
import ANurbs as an
import numpy as np
import numpy.linalg as la
import time, os, shutil

# ==============================================================================
class CADMapper:
    # --------------------------------------------------------------------------
    def __init__(self, fe_model, cad_model, parameters):
        default_parameters = KratosMultiphysics.Parameters("""
        {
            "input" :
            {
                "cad_filename"                  : "name.iga",
                "fem_filename"                  : "name.mdpa",
                "fe_refinement_level"           : 0
            },
            "conditions" :
            {
                "general" :
                {
                    "apply_integral_method" : false,
                    "mapping_cad_fem"       :
                    {
                        "mapper_type"   : "nearest_element",
                        "search_radius" : -1.0,
                        "echo_level"    : 0
                    }
                },
                "faces" :
                {
                    "curvature" :
                    {
                        "apply_curvature_minimization" : false,
                        "penalty_factor"               : 1e-1
                    },
                    "mechanical" :
                    {
                        "apply_KL_shell"      : false,
                        "exclusive_face_list" : [],
                        "penalty_factor"      : 1e3
                    },
                    "rigid" :
                    {
                        "apply_rigid_conditions" : false,
                        "exclusive_face_list"    : [],
                        "penalty_factor"         : 1e3
                    }
                },
                "edges" :
                {
                    "fe_based" :
                    {
                        "apply_enforcement_conditions"        : false,
                        "penalty_factor_position_enforcement" : 1e3,
                        "penalty_factor_tangent_enforcement"  : 1e3,
                        "apply_corner_enforcement_conditions" : false,
                        "penalty_factor_corner_enforcement"   : 1e4
                    },
                    "coupling" :
                    {
                        "apply_coupling_conditions"            : false,
                        "penalty_factor_displacement_coupling" : 1e3,
                        "penalty_factor_rotation_coupling"     : 1e3
                    }
                }
            },
            "drawing_parameters" :
            {
                "cad_drawing_tolerance"           : 1e-3,
                "boundary_tessellation_tolerance" : 1e-2,
                "patch_bounding_box_tolerance"    : 1.0,
                "min_span_length"                 : 1e-7
            },
            "solution" :
            {
                "iterations"    : 1,
                "test_solution" : true
            },
            "regularization" :
            {
                "alpha"             : 0.1,
                "beta"              : 0.001,
                "include_all_poles" : false
            },
            "refinement" :
            {
                "a_priori" :
                {
                    "apply_a_priori_refinement"         : false,
                    "max_levels_of_refinement"          : 3,
                    "min_knot_distance_at_max_gradient" : 1.0,
                    "exponent"                          : 2
                },
                "a_posteriori" :
                {
                    "apply_a_posteriori_refinement" : false,
                    "max_levels_of_refinement"      : 3,
                    "mininimum_knot_distance"       : 1.0,
                    "fe_point_distance_tolerance"   : 0.01,
                    "disp_coupling_tolerance"       : 0.01,
                    "rot_coupling_tolerance"        : 0.5
                }
            },
            "output":
            {
                "results_directory"                   : "01_Results",
                "resulting_geometry_filename"         : "reconstructed_geometry.iga",
                "filename_fem_for_reconstruction"     : "fe_model_used_for_reconstruction",
                "filename_fem_for_quality_evaluation" : "fe_model_with_reconstruction_quality"
            }
        }""")
        parameters.RecursivelyValidateAndAssignDefaults(default_parameters)

        self.fe_model = fe_model
        self.cad_model = cad_model
        self.parameters = parameters

        fe_model_part_name = "origin_part"
        if self.fe_model.HasModelPart(fe_model_part_name):
            self.fe_model_part = self.fe_model.GetModelPart(fe_model_part_name)
        else:
            self.fe_model_part = self.fe_model.CreateModelPart(fe_model_part_name)
            self.fe_model_part.ProcessInfo.SetValue(KratosMultiphysics.DOMAIN_SIZE, 3)
            self.fe_model_part.AddNodalSolutionStepVariable(KratosShape.SHAPE_CHANGE)
            self.fe_model_part.AddNodalSolutionStepVariable(KratosMultiphysics.NORMAL)
            self.fe_model_part.AddNodalSolutionStepVariable(KratosShape.NORMALIZED_SURFACE_NORMAL)
            self.fe_model_part.AddNodalSolutionStepVariable(KratosShape.FITTING_ERROR)
            self.fe_model_part.AddNodalSolutionStepVariable(KratosShape.GRAD_SHAPE_CHANGE)
            self.fe_model_part.AddNodalSolutionStepVariable(KratosShape.SHAPE_CHANGE_ABSOLUTE)

        self.condition_factory = ConditionFactory(self.fe_model_part, self.cad_model, self.parameters)
        self.assembler = Assembler(self.cad_model)

        self.conditions = {}
        self.absolute_pole_displacement = None

    # --------------------------------------------------------------------------
    def RunMappingProcess(self):
        apply_a_priori_refinement = self.parameters["refinement"]["a_priori"]["apply_a_priori_refinement"].GetBool()
        apply_a_posteriori_refinement = self.parameters["refinement"]["a_posteriori"]["apply_a_posteriori_refinement"].GetBool()
        max_iterations = self.parameters["refinement"]["a_posteriori"]["max_levels_of_refinement"].GetInt()
        output_dir = self.parameters["output"]["results_directory"].GetString()
        fem_filename = self.parameters["output"]["filename_fem_for_quality_evaluation"].GetString()
        cad_filename = self.parameters["output"]["resulting_geometry_filename"].GetString()

        # Initialize results folder
        if os.path.exists(output_dir):
            shutil.rmtree(output_dir)

        self.ReadModelData()

        if apply_a_priori_refinement:
            self.PerformAPrioriRefinement()

        if apply_a_posteriori_refinement:
            for itr in range(max_iterations):

                print("\n\n========================================================================================================")
                print("> Starting a posteriori refinement level " +  str(itr+1) + "...")
                print("========================================================================================================")

                iteration_tag = "_level_"+str(itr+1)
                self.parameters["output"]["filename_fem_for_quality_evaluation"].SetString(fem_filename + iteration_tag)
                self.parameters["output"]["resulting_geometry_filename"].SetString(cad_filename.replace(".iga", iteration_tag+".iga"))

                self.Initialize()
                self.Map()
                self.Finalize()

                nothing_to_refine = self.ResetDisplacementsAndRefineCadModel()

                if nothing_to_refine:
                    break
                else:
                    self.__OutputCadModel("a_posteriori_refinement"+ iteration_tag +".iga")
        else:
            self.Initialize()
            self.Map()
            self.Finalize()

    # --------------------------------------------------------------------------
    def ReadModelData(self):
        print("\n> Starting to read model data...")
        start_time = time.time()

        # Read FEM data
        if len(self.fe_model_part.Nodes) == 0:
            fem_input_filename = self.parameters["input"]["fem_filename"].GetString()
            model_part_io = KratosMultiphysics.ModelPartIO(fem_input_filename[:-5])
            model_part_io.ReadModelPart(self.fe_model_part)

        # Refine if specified
        prop_id = 1
        prop = self.fe_model_part.Properties[prop_id]
        mat = KratosCSM.LinearElasticPlaneStress2DLaw()
        prop.SetValue(KratosMultiphysics.CONSTITUTIVE_LAW, mat.Clone())

        # Refine FE if necessary
        for refinement_level in range(0,self.parameters["input"]["fe_refinement_level"].GetInt()):
            number_of_avg_elems = 10
            number_of_avg_nodes = 10

            nodal_neighbour_search = KratosMultiphysics.FindNodalNeighboursProcess(self.fe_model_part, number_of_avg_elems, number_of_avg_nodes)
            nodal_neighbour_search.Execute()

            neighbour_calculator = KratosMultiphysics.FindElementalNeighboursProcess(self.fe_model_part,2,10)
            neighbour_calculator.Execute()

            for elem in self.fe_model_part.Elements:
                elem.SetValue(KratosMultiphysics.SPLIT_ELEMENT,True)

            refine_on_reference = False
            interpolate_internal_variables = True
            Refine = KratosMeshingApp.LocalRefineTriangleMesh(self.fe_model_part)
            Refine.LocalRefineMesh(refine_on_reference, interpolate_internal_variables)

        # Compute average surface normals of target design
        for node in self.fe_model_part.Nodes:
            shape_update = node.GetSolutionStepValue(KratosShape.SHAPE_CHANGE)
            node.X += shape_update[0]
            node.Y += shape_update[1]
            node.Z += shape_update[2]

        KratosShape.GeometryUtilities(self.fe_model_part).ComputeUnitSurfaceNormals(True)

        for node in self.fe_model_part.Nodes:
            shape_update = node.GetSolutionStepValue(KratosShape.SHAPE_CHANGE)
            node.X -= shape_update[0]
            node.Y -= shape_update[1]
            node.Z -= shape_update[2]

        # Read CAD data
        cad_filename = self.parameters["input"]["cad_filename"].GetString()
        if len(self.cad_model.GetByType('BrepFace')) == 0:
            self.cad_model.Load(cad_filename)

        print("> Finished reading of model data in" ,round( time.time()-start_time, 3 ), " s.")

    # --------------------------------------------------------------------------
    def PerformAPrioriRefinement(self):
        print("\n> Initializing prior refinement...")
        start_time = time.time()

        max_iterations = self.parameters["refinement"]["a_priori"]["max_levels_of_refinement"].GetInt()
        min_knot_distance_at_max_gradient = self.parameters["refinement"]["a_priori"]["min_knot_distance_at_max_gradient"].GetDouble()
        exponent = self.parameters["refinement"]["a_priori"]["exponent"].GetInt()

        fe_point_parametric = self.condition_factory.GetFEPointParametrization()

        # Compute gradient of displacment field if necessary
        for node in self.fe_model_part.Nodes:
            shape_change = np.array(node.GetSolutionStepValue(KratosShape.SHAPE_CHANGE))
            node.SetSolutionStepValue(KratosShape.SHAPE_CHANGE_ABSOLUTE, la.norm(shape_change))

        local_gradient = KratosMultiphysics.ComputeNodalGradientProcess3D(self.fe_model_part, KratosShape.SHAPE_CHANGE_ABSOLUTE, KratosShape.GRAD_SHAPE_CHANGE, KratosMultiphysics.NODAL_AREA)
        local_gradient.Execute()

        l2_norms_grads = [np.array(entry["node"].GetSolutionStepValue(KratosShape.GRAD_SHAPE_CHANGE)) for entry in fe_point_parametric]
        l2_norms_grads = [ la.norm(grad) for grad in l2_norms_grads]
        max_norm_grads = max(l2_norms_grads)

        # Perform iterative refinement
        for refinement_itr in range(max_iterations):

            print("> Starting a priorie refinement level " +  str(refinement_itr+1) + "...")

            # Identify spots to refine
            intervals_along_u_to_refine = {geometry.Key(): [] for geometry in self.cad_model.GetByType('SurfaceGeometry3D')}
            intervals_along_v_to_refine = {geometry.Key(): [] for geometry in self.cad_model.GetByType('SurfaceGeometry3D')}

            exist_intervals_to_refine = False

            # First identify spots where distance to fe points exceeds limit
            for entry in fe_point_parametric:

                node = entry["node"]

                list_of_faces = entry["faces"]
                list_of_parameters = entry["parameters"]

                for face, (u,v) in zip(list_of_faces, list_of_parameters):
                    geometry = face.Data().Geometry()
                    geometry_data = geometry.Data()

                    grad_shape_change = np.array(node.GetSolutionStepValue(KratosShape.GRAD_SHAPE_CHANGE))
                    scaled_grad_norm = la.norm(grad_shape_change) / max_norm_grads
                    scaled_grad_norm = max(scaled_grad_norm,1e-6)

                    # hyperbolic increase of min_distance: 1 / x^exp : exp = 2 means that ca 70% reduced gradient leads to ca 10x increased min distance
                    required_knot_distance = min_knot_distance_at_max_gradient * (1 / scaled_grad_norm)**exponent

                    u_added = self.__AddUIntervalToList(geometry, u, v, intervals_along_u_to_refine, required_knot_distance)
                    v_added = self.__AddVIntervalToList(geometry, u, v, intervals_along_v_to_refine, required_knot_distance)

                    exist_intervals_to_refine = exist_intervals_to_refine or u_added or v_added

            # Perform actual refinement
            if exist_intervals_to_refine == False:
                print("> Maximum refinement for specified minimum knot distances reached!")
                break
            else:
                self.__RefineAtIntervalCenters(self.cad_model, intervals_along_u_to_refine, intervals_along_v_to_refine)

        self.__OutputCadModel("cad_model_after_prior_refinement.iga")

        print("> Prior refinement finished in" ,round( time.time()-start_time, 3 ), " s.")

    # --------------------------------------------------------------------------
    def Initialize(self):
        # Initialize (reset) class attributes
        self.conditions = {}
        self.absolute_pole_displacement = None

        nodal_variables = ["SHAPE_CHANGE", "GRAD_SHAPE_CHANGE", "NORMAL", "NORMALIZED_SURFACE_NORMAL"]
        filename = self.parameters["output"]["filename_fem_for_reconstruction"].GetString()
        self.__OutputFEData(self.fe_model_part, filename, nodal_variables)

        print("\n> Starting creation of conditions...")
        start_time = time.time()

        self.conditions = self.condition_factory.CreateConditions()

        print("> Finished creation of conditions in" ,round( time.time()-start_time, 3 ), " s.")
        print("\n> Initializing assembly...")
        start_time = time.time()

        self.assembler.Initialize(self.conditions)

        print("> Initialization of assembly finished in" ,round( time.time()-start_time, 3 ), " s.")

    # --------------------------------------------------------------------------
    def Map(self):
        # Nonlinear solution iterations
        for solution_itr in range(1,self.parameters["solution"]["iterations"].GetInt()+1):

            # Assemble
            lhs, rhs = self.assembler.AssembleSystem()

            if solution_itr == 1:
                total_num_conditions = sum( [len(list_of_values) for list_of_values in self.conditions.values()] )
                print("\n> Number of conditions =",total_num_conditions)
                print("> Number of equations =", lhs.shape[0])
                print("> Number of relevant control points =", lhs.shape[1])

            print("\n> ----------------------------------------------------")
            print("> Starting solution iteration", solution_itr,"...")
            start_time_iteration = time.time()

            print("\n> Starting system solution ....")
            start_time_solution = time.time()

            # Beta regularization
            beta = self.parameters["regularization"]["beta"].GetDouble()
            lhs_diag = np.diag(lhs)
            for i in range(lhs_diag.shape[0]):
                entry = lhs[i,i]
                # if abs(entry) < 1e-12:
                #     print("WARNING!!!!Zero on main diagonal found at position",i,". Make sure to include beta regularization.")
                lhs[i,i] += beta

            solution = la.solve(lhs,rhs)

            print("> Finished system solution in" ,round( time.time()-start_time_solution, 3 ), " s.")

            self.__UpdateCADModel(solution)
            self.__ComputeAbsolutePoleDisplacements(solution)

            if self.parameters["solution"]["test_solution"].GetBool():

                # Test solution quality
                test_rhs = np.zeros(rhs.shape)
                test_rhs[:] = lhs.dot(solution)

                delta = rhs-test_rhs
                error_norm = la.norm(delta)
                print("\n> Error in linear solution = ",error_norm)

                # Test residuals
                error_norm = la.norm(rhs)
                print("> RHS before current solution iteration = ",error_norm)

                # Test rhs after update
                rhs = self.assembler.AssembleRHS()

                error_norm = la.norm(rhs)
                print("\n> RHS after current solution iteration = ",error_norm)

                # Varying contribution of beta regularization is neglected as each solution iteration may be seen indendently
                # in terms of minimization of the control point displacement

            print("\n> Finished solution iteration in" ,round( time.time()-start_time_iteration, 3 ), " s.")
            print("> ----------------------------------------------------")

    # --------------------------------------------------------------------------
    def Finalize(self):
        print("\n> Finalizing mapping....")
        start_time = time.time()

        # Output cad model
        output_filename = self.parameters["output"]["resulting_geometry_filename"].GetString()
        self.__OutputCadModel(output_filename)

        print("> Finished finalization of mapping in" ,round( time.time()-start_time, 3 ), " s.")

    # --------------------------------------------------------------------------
    def ResetDisplacementsAndRefineCadModel(self):
        fe_point_distance_tolerance = self.parameters["refinement"]["a_posteriori"]["fe_point_distance_tolerance"].GetDouble() # in given length unit
        disp_coupling_tolerance = self.parameters["refinement"]["a_posteriori"]["disp_coupling_tolerance"].GetDouble() # in given length unit
        rot_coupling_tolerance = self.parameters["refinement"]["a_posteriori"]["rot_coupling_tolerance"].GetDouble() # in degree
        minimum_knot_distance = self.parameters["refinement"]["a_posteriori"]["mininimum_knot_distance"].GetDouble()

        fe_point_parametric = self.condition_factory.GetFEPointParametrization()

        exist_intervals_to_refine = False
        is_fe_point_distance_satisfied = True
        is_disp_coupling_satisfied = True
        is_rot_coupling_satisfied = True

        # Identify spots to refine
        intervals_along_u_to_refine = {geometry.Key(): [] for geometry in self.cad_model.GetByType('SurfaceGeometry3D')}
        intervals_along_v_to_refine = {geometry.Key(): [] for geometry in self.cad_model.GetByType('SurfaceGeometry3D')}

        # First identify spots where distance to fe points exceeds limit
        for entry in fe_point_parametric:

            node = entry["node"]
            shape_update = node.GetSolutionStepValue(KratosShape.SHAPE_CHANGE)
            node_target_position = np.array([node.X0 + shape_update[0], node.Y0 + shape_update[1], node.Z0 + shape_update[2]])

            list_of_faces = entry["faces"]
            list_of_parameters = entry["parameters"]

            for face, (u,v) in zip(list_of_faces, list_of_parameters):
                geometry = face.Data().Geometry()
                geometry_data = geometry.Data()

                distance = geometry_data.PointAt(u, v) - node_target_position

                node.SetSolutionStepValue(KratosShape.FITTING_ERROR, distance.tolist())

                if la.norm(distance) > fe_point_distance_tolerance:
                    u_added = self.__AddUIntervalToList(geometry, u, v, intervals_along_u_to_refine, minimum_knot_distance)
                    v_added = self.__AddVIntervalToList(geometry, u, v, intervals_along_v_to_refine, minimum_knot_distance)

                    exist_intervals_to_refine = exist_intervals_to_refine or u_added or v_added
                    is_fe_point_distance_satisfied = False

        # Output fitting error
        nodal_variables = ["SHAPE_CHANGE", "FITTING_ERROR"]
        filename = self.parameters["output"]["filename_fem_for_quality_evaluation"].GetString()
        self.__OutputFEData(self.fe_model_part, filename, nodal_variables)

        # Then identify spots where coupling or enforcement conditions are not met
        for conditions_face_i in self.conditions.values():
            for condition in conditions_face_i:
                if isinstance(condition, clib.DisplacementCouplingCondition) or isinstance(condition, clib.RotationCouplingConditionWithAD):
                    geometry_a = condition.geometry_a
                    geometry_b = condition.geometry_b
                    (u_a,v_a) = condition.parameters_a
                    (u_b,v_b) = condition.parameters_b

                    is_spot_to_be_refined = False

                    if isinstance(condition, clib.DisplacementCouplingCondition):
                        delta_disp = la.norm(condition.CalculateQualityIndicator())
                        if delta_disp > disp_coupling_tolerance:
                            is_disp_coupling_satisfied = False
                            is_spot_to_be_refined = True
                            print("delta_disp =", delta_disp)

                    if isinstance(condition, clib.RotationCouplingConditionWithAD):
                        delta_rot = condition.CalculateQualityIndicator() * 180 / np.pi
                        if delta_rot > rot_coupling_tolerance:
                            is_rot_coupling_satisfied = False
                            is_spot_to_be_refined = True
                            print("delta_rot =", delta_rot)

                    if is_spot_to_be_refined:
                        # Geometry a
                        u_a_added = self.__AddUIntervalToList(geometry_a, u_a, v_a, intervals_along_u_to_refine, minimum_knot_distance)
                        v_a_added = self.__AddVIntervalToList(geometry_a, u_a, v_a, intervals_along_v_to_refine, minimum_knot_distance)

                        # Geometry b
                        u_b_added = self.__AddUIntervalToList(geometry_b, u_b, v_b, intervals_along_u_to_refine, minimum_knot_distance)
                        v_b_added = self.__AddVIntervalToList(geometry_b, u_b, v_b, intervals_along_v_to_refine, minimum_knot_distance)

                        exist_intervals_to_refine = exist_intervals_to_refine or u_a_added or v_a_added or u_b_added or v_b_added

        self.ResetPoleDisplacements()

        # Perform actual refinement
        nothing_to_refine = False

        if is_fe_point_distance_satisfied and is_disp_coupling_satisfied and is_rot_coupling_satisfied:
            print("\n> Refinement reached convergence! Nothing to refine anymore.")
            nothing_to_refine = True

        elif exist_intervals_to_refine == False:
            print("\n> WARNING!!!! Refinement did not converge but finished as knot tolerance was reached for U- and V-Direction.")
            nothing_to_refine = True

        else:
            self.__RefineAtIntervalCenters(self.cad_model, intervals_along_u_to_refine, intervals_along_v_to_refine)

        return nothing_to_refine

    # --------------------------------------------------------------------------
    def ResetPoleDisplacements(self):
        dof_ids = self.assembler.GetDofIds()
        dofs = self.assembler.GetDofs()

        for surface_i in self.cad_model.GetByType('SurfaceGeometry3D'):
            surface_geometry = surface_i.Data()
            surface_geometry_key = surface_i.Key()

            for r in range(surface_geometry.NbPolesU()):
                for s in range(surface_geometry.NbPolesV()):
                    dof_i_x = (surface_geometry_key,r,s,"x")
                    dof_i_y = (surface_geometry_key,r,s,"y")
                    dof_i_z = (surface_geometry_key,r,s,"z")

                    if dof_i_x in dofs:
                        dof_id_x = dof_ids[dof_i_x]
                        dof_id_y = dof_ids[dof_i_y]
                        dof_id_z = dof_ids[dof_i_z]

                        pole_coords = surface_geometry.Pole(r,s)
                        pole_update = np.array([self.absolute_pole_displacement[dof_id_x], self.absolute_pole_displacement[dof_id_y], self.absolute_pole_displacement[dof_id_z]])

                        new_pole_coords = pole_coords - pole_update
                        surface_geometry.SetPole(r,s,new_pole_coords)

    # --------------------------------------------------------------------------
    def __UpdateCADModel(self, solution):
        print("\n> Updating cad database....")
        start_time = time.time()

        dof_ids = self.assembler.GetDofIds()
        dofs = self.assembler.GetDofs()

        for surface_i in self.cad_model.GetByType('SurfaceGeometry3D'):
            surface_geometry = surface_i.Data()
            surface_geometry_key = surface_i.Key()

            for r in range(surface_geometry.NbPolesU()):
                for s in range(surface_geometry.NbPolesV()):
                    dof_i_x = (surface_geometry_key,r,s,"x")
                    dof_i_y = (surface_geometry_key,r,s,"y")
                    dof_i_z = (surface_geometry_key,r,s,"z")

                    if dof_i_x in dofs:
                        dof_id_x = dof_ids[dof_i_x]
                        dof_id_y = dof_ids[dof_i_y]
                        dof_id_z = dof_ids[dof_i_z]

                        pole_coords = surface_geometry.Pole(r,s)
                        pole_update = np.array([solution[dof_id_x], solution[dof_id_y], solution[dof_id_z]])

                        new_pole_coords = pole_coords + pole_update
                        surface_geometry.SetPole(r,s,new_pole_coords)

        print("> Finished updating cad database in" ,round( time.time()-start_time, 3 ), " s.")

    # --------------------------------------------------------------------------
    def __ComputeAbsolutePoleDisplacements(self, delta):
        if self.absolute_pole_displacement is None:
            self.absolute_pole_displacement = delta
        else:
            self.absolute_pole_displacement += delta

    # --------------------------------------------------------------------------
    def __OutputCadModel(self, filename):
        output_dir = self.parameters["output"]["results_directory"].GetString()
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)

        output_filename_with_path = os.path.join(output_dir,filename)
        self.cad_model.Save(output_filename_with_path)

    # --------------------------------------------------------------------------
    def __OutputFEData(self, model_part, fem_output_filename, nodal_variables):
        output_dir = self.parameters["output"]["results_directory"].GetString()
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)

        fem_output_filename_with_path = os.path.join(output_dir,fem_output_filename)

        from gid_output import GiDOutput
        nodal_results=nodal_variables
        gauss_points_results=[]
        VolumeOutput = True
        GiDPostMode = "Binary"
        GiDWriteMeshFlag = False
        GiDWriteConditionsFlag = True
        GiDMultiFileFlag = "Single"

        gig_io = GiDOutput(fem_output_filename_with_path, VolumeOutput, GiDPostMode, GiDMultiFileFlag, GiDWriteMeshFlag, GiDWriteConditionsFlag)
        gig_io.initialize_results(model_part)
        gig_io.write_results(1, model_part, nodal_results, gauss_points_results)
        gig_io.finalize_results()

    # --------------------------------------------------------------------------
    @staticmethod
    def __AddUIntervalToList(geometry, u, v, interval_list, mininimum_knot_distance):
        intervall_added = False

        start_span = an.Knots.UpperSpan(geometry.Data().DegreeU(), geometry.Data().KnotsU(), u)
        end_span = start_span+1
        u_start = geometry.Data().KnotsU()[start_span]
        u_end = geometry.Data().KnotsU()[end_span]

        # Avoid refining knots that are close together within a specified tolerance
        # comparisons are made in given length unit
        p_start = geometry.Data().PointAt(u_start, v)
        p_end = geometry.Data().PointAt(u_end, v)
        distance = p_end - p_start

        if np.dot(distance, distance) >= mininimum_knot_distance**2 :
            intervall_added = True
            u_interval = (u_start, u_end)
            interval_list[geometry.Key()].append(u_interval)

        return intervall_added

    # --------------------------------------------------------------------------
    @staticmethod
    def __AddVIntervalToList(geometry, u , v, interval_list, minimum_knot_distance):
        intervall_added = False

        start_span = an.Knots.UpperSpan(geometry.Data().DegreeV(), geometry.Data().KnotsV(), v)
        end_span = start_span+1
        v_start = geometry.Data().KnotsV()[start_span]
        v_end = geometry.Data().KnotsV()[end_span]

        # Avoid refining knots that are close together within a specified tolerance
        # comparisons are made in given length unit
        p_start = geometry.Data().PointAt(u, v_start)
        p_end = geometry.Data().PointAt(u, v_end)
        distance = p_end - p_start

        if np.dot(distance, distance) >= minimum_knot_distance**2 :
            intervall_added = True
            v_interval = (v_start, v_end)
            interval_list[geometry.Key()].append(v_interval)

        return intervall_added

    # --------------------------------------------------------------------------
    @staticmethod
    def __RefineAtIntervalCenters(cad_model, intervals_along_u_to_refine, intervals_along_v_to_refine):
        for face in cad_model.GetByType('BrepFace'):
                # print("> Refining face ",face.Key())
                geometry = face.Data().Geometry()

                u_intervals = intervals_along_u_to_refine[geometry.Key()]
                if len(u_intervals) > 0:
                    u_interval_centers = [(interval[0]+interval[1])/2.0 for interval in u_intervals]
                    u_interval_centers = list(set(u_interval_centers)) # Remove duplicated entries
                    u_interval_centers.sort() # Sort in ascending order

                    refined_geometry = an.KnotRefinement.InsertKnotsU(geometry.Data(), u_interval_centers)
                    cad_model.Replace(geometry.Key(), refined_geometry)
                # else:
                #     print("> Nothing to refine in U-driection!")

                v_intervals = intervals_along_v_to_refine[geometry.Key()]
                if len(v_intervals) > 0:
                    v_interval_centers = [(interval[0]+interval[1])/2.0 for interval in v_intervals]
                    v_interval_centers = list(set(v_interval_centers)) # Remove duplicated entries
                    v_interval_centers.sort() # Sort in ascending order

                    refined_geometry = an.KnotRefinement.InsertKnotsV(geometry.Data(), v_interval_centers)
                    cad_model.Replace(geometry.Key(), refined_geometry)
                # else:
                #     print("> Nothing to refine in V-driection!")

# ==============================================================================