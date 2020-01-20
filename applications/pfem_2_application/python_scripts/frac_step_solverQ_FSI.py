from __future__ import print_function, absolute_import, division #makes KratosMultiphysics backward compatible with python 2.6 and 2.7
from KratosMultiphysics import *

#from KratosMultiphysics.IncompressibleFluidApplication import *
from KratosMultiphysics.PFEM2Application import *
from KratosMultiphysics.MeshingApplication import *
#from KratosMultiphysics.StructuralMechanicsApplication import *
#from KratosMultiphysics.PFEMApplication import *
#from KratosMultiphysics.ConvectionDiffusionApplication import *
from KratosMultiphysics.ULFApplication import *
from KratosMultiphysics.ULFApplication import *

from KratosMultiphysics.ExternalSolversApplication import *

#from KratosMultiphysics.StructuralApplication import *



def AddVariables(model_part,p_model_part):
    model_part.AddNodalSolutionStepVariable(VELOCITY);
    model_part.AddNodalSolutionStepVariable(MESH_VELOCITY);
    model_part.AddNodalSolutionStepVariable(PRESSURE);
    model_part.AddNodalSolutionStepVariable(PRESS_PROJ);
    model_part.AddNodalSolutionStepVariable(NODAL_MASS);
    model_part.AddNodalSolutionStepVariable(NODAL_AREA)
    model_part.AddNodalSolutionStepVariable(BODY_FORCE);
    model_part.AddNodalSolutionStepVariable(DENSITY);
    model_part.AddNodalSolutionStepVariable(VISCOSITY);
    model_part.AddNodalSolutionStepVariable(ACCELERATION);
    model_part.AddNodalSolutionStepVariable(ANGULAR_VELOCITY);
    model_part.AddNodalSolutionStepVariable(DISPLACEMENT);
    model_part.AddNodalSolutionStepVariable(DISTANCE);
    model_part.AddNodalSolutionStepVariable(FORCE)
    model_part.AddNodalSolutionStepVariable(FORCE_CM)
    model_part.AddNodalSolutionStepVariable(MOMENTUM_CM)
    model_part.AddNodalSolutionStepVariable(IS_BOUNDARY)
    #model_part.AddNodalSolutionStepVariable(TO_ERASE)
    model_part.AddNodalSolutionStepVariable(MATERIAL_VARIABLE);
    model_part.AddNodalSolutionStepVariable(IS_INTERFACE);
    model_part.AddNodalSolutionStepVariable(FLAG_VARIABLE);
    model_part.AddNodalSolutionStepVariable(IS_FLUID)
    model_part.AddNodalSolutionStepVariable(IS_WATER_ELEMENT)
    model_part.AddNodalSolutionStepVariable(IS_WATER)
    model_part.AddNodalSolutionStepVariable(IS_STRUCTURE);
    model_part.AddNodalSolutionStepVariable(IS_FREE_SURFACE)
    model_part.AddNodalSolutionStepVariable(IS_VISITED)
    model_part.AddNodalSolutionStepVariable(NODAL_H)
    model_part.AddNodalSolutionStepVariable(IS_POROUS)
    model_part.AddNodalSolutionStepVariable(IS_EROSIONABLE)
    model_part.AddNodalSolutionStepVariable(PRESSUREAUX)
    model_part.AddNodalSolutionStepVariable(BULK_MODULUS)
    model_part.AddNodalSolutionStepVariable(NORMAL)
    model_part.AddNodalSolutionStepVariable(VELOCITY)
    model_part.AddNodalSolutionStepVariable(CAUCHY_STRESS_TENSOR)
    p_model_part.AddNodalSolutionStepVariable(VELOCITY);
    #p_model_part.AddNodalSolutionStepVariable(FRACT_VEL);
    p_model_part.AddNodalSolutionStepVariable(MESH_VELOCITY);
    p_model_part.AddNodalSolutionStepVariable(PRESSURE);
    p_model_part.AddNodalSolutionStepVariable(PRESSURE_OLD_IT);
    p_model_part.AddNodalSolutionStepVariable(PRESS_PROJ);
    p_model_part.AddNodalSolutionStepVariable(NODAL_MASS);
    p_model_part.AddNodalSolutionStepVariable(NODAL_AREA)
    p_model_part.AddNodalSolutionStepVariable(BODY_FORCE);
    p_model_part.AddNodalSolutionStepVariable(DENSITY);
    p_model_part.AddNodalSolutionStepVariable(VISCOSITY);
    p_model_part.AddNodalSolutionStepVariable(ACCELERATION);
    p_model_part.AddNodalSolutionStepVariable(ANGULAR_VELOCITY);
    p_model_part.AddNodalSolutionStepVariable(DISPLACEMENT);
    p_model_part.AddNodalSolutionStepVariable(DISTANCE);
    p_model_part.AddNodalSolutionStepVariable(FORCE)
    p_model_part.AddNodalSolutionStepVariable(FORCE_CM)
    p_model_part.AddNodalSolutionStepVariable(MOMENTUM_CM)
    p_model_part.AddNodalSolutionStepVariable(IS_BOUNDARY)
    #p_model_part.AddNodalSolutionStepVariable(ERASE_FLAG)
    p_model_part.AddNodalSolutionStepVariable(MATERIAL_VARIABLE);
    p_model_part.AddNodalSolutionStepVariable(IS_INTERFACE);
    p_model_part.AddNodalSolutionStepVariable(FLAG_VARIABLE);
    p_model_part.AddNodalSolutionStepVariable(IS_FLUID)
    p_model_part.AddNodalSolutionStepVariable(IS_WATER_ELEMENT)
    p_model_part.AddNodalSolutionStepVariable(IS_WATER)
    p_model_part.AddNodalSolutionStepVariable(IS_STRUCTURE);
    p_model_part.AddNodalSolutionStepVariable(IS_FREE_SURFACE)
    p_model_part.AddNodalSolutionStepVariable(IS_VISITED)
    p_model_part.AddNodalSolutionStepVariable(NODAL_H)
    p_model_part.AddNodalSolutionStepVariable(IS_POROUS)
    p_model_part.AddNodalSolutionStepVariable(IS_EROSIONABLE)
    p_model_part.AddNodalSolutionStepVariable(PRESSUREAUX)
    p_model_part.AddNodalSolutionStepVariable(BULK_MODULUS)
    p_model_part.AddNodalSolutionStepVariable(NORMAL)
    p_model_part.AddNodalSolutionStepVariable(VELOCITY)



def AddDofs(model_part,p_model_part):
  
    for node in model_part.Nodes:
        node.AddDof(PRESSURE); 
        node.AddDof(VELOCITY_X);
        node.AddDof(VELOCITY_Y);
        node.AddDof(VELOCITY_Z);

   

def ReadRestartFile(FileName,nodes):
   NODES = nodes
   aaa = open(FileName)
   for line in aaa:
       exec(line)
       
  

class FracStepSolver:
    
    def __init__(self, fluid_model_part, structure_model_part, combined_model_part,p_model_part,box_corner1,box_corner2,domain_size):

        #
	# saving the different model parts
        self.fluid_model_part = fluid_model_part;  # contains only fluid elements, but all nodes!
        self.combined_model_part = combined_model_part;  # contains both structure and fluid
        self.structure_model_part = structure_model_part;  # contains only structural elements
        
        self.p_model_part = p_model_part
        self.domain_size = domain_size

        #neighbour search
        number_of_avg_elems = 10
        number_of_avg_nodes = 10
        self.neighbour_search = FindNodalNeighboursProcess(fluid_model_part,number_of_avg_elems,number_of_avg_nodes)



        #assignation of parameters to be used
        self.vel_toll = 0.0001;
        self.press_toll = 0.001;
        self.max_vel_its = 6;
        self.max_press_its = 5;
        self.time_order = 2;
        self.CalculateReactions = False;
        self.ReformDofAtEachIteration = False; 
        self.CalculateNormDxFlag = True;
        self.laplacian_form = 2; #1 = laplacian, 2 = Discrete Laplacian
        self.predictor_corrector = False;

        self.echo_level = 0

        self.box_corner1 = box_corner1
        self.box_corner2 = box_corner2

        #definition of the solvers
        pDiagPrecond = DiagonalPreconditioner()
        #self.velocity_linear_solver = BICGSTABSolver(1e-6, 5000, pDiagPrecond)
        
        self.velocity_linear_solver =  SuperLUSolver()

        self.pressure_linear_solver =  BICGSTABSolver(1e-6, 5000,pDiagPrecond)
        self.velocity_linear_solver =  CGSolver(1e-4, 5000,pDiagPrecond)

        self.dynamic_tau = 0.001
        self.activate_tau2 = False


        ##handling slip condition
        self.slip_conditions_initialized = False
        self.neigh_finder = FindNodalNeighboursProcess(fluid_model_part,9,18)
        self.compute_reactions=False
        self.timer=Timer()    
 

        self.Mesher1 = TriGenPFEMModeler()

        self.node_erase_process = NodeEraseProcess(fluid_model_part);

        self.Pfem2_apply_bc_process = Pfem2ApplyBCProcess(fluid_model_part);
        self.Pfem2Utils = Pfem2Utils()
        self.mark_outer_nodes_process = MarkOuterNodesProcess(fluid_model_part);

        # tools to save and merge the structural contributions
        self.save_structure_model_part_process = SaveStructureModelPartProcess();
        self.merge_model_parts_process = MergeModelPartsProcess();
        # this will save fluid only model part

        
        if(domain_size == 2):
            #self.particle_utils = ParticleUtils2D()	
            self.Mesher = TriGenPFEMModeler()
            
            self.fluid_neigh_finder = FindNodalNeighboursProcess(self.fluid_model_part,9,18)
            #this is needed if we want to also store the conditions a node belongs to
            self.condition_neigh_finder = FindConditionsNeighboursProcess(self.fluid_model_part,2, 10)
            self.elem_neighbor_finder = FindElementalNeighboursProcess(self.fluid_model_part, 2, 10)	
            self.combined_neigh_finder = FindNodalNeighboursProcess(self.combined_model_part, 9, 18)
            
        elif (domain_size == 3):
            #self.Mesher = TetGenModeler()
            #improved mesher
            self.particle_utils = ParticleUtils3D()
            
            self.Mesher = TetGenPfemModeler()
            self.fluid_neigh_finder = FindNodalNeighboursProcess(self.fluid_model_part,20,30)
            #this is needed if we want to also store the conditions a node belongs to
            self.condition_neigh_finder = FindConditionsNeighboursProcess(self.fluid_model_part,3, 20)
            self.elem_neighbor_finder = FindElementalNeighboursProcess(self.fluid_model_part, 20, 30)
     
        self.mark_fluid_process = MarkFluidProcess(self.fluid_model_part);
        self.mark_structure_process = MarkStructureProcess(self.fluid_model_part);
        self.hypoelastic_solid_stress_tensor_calculate_process=HypoelasticStressCalculateProcess(self.combined_model_part, self.domain_size)


    def Initialize(self):
        (self.neighbour_search).Execute()

        self.domain_size = int(self.domain_size)
        self.laplacian_form = int(self.laplacian_form)

        self.ReformDofAtEachIteration = bool(self.ReformDofAtEachIteration)
        self.vel_toll = float(self.vel_toll)
        self.press_toll = float(self.press_toll)
        self.max_vel_its = int(self.max_vel_its)
        self.max_press_its = int(self.max_press_its)
        self.time_order = int(self.time_order)
        self.domain_size = int(self.domain_size)

        self.predictor_corrector = bool(self.predictor_corrector)
        self.solver = FracStepStrategy( self.combined_model_part, self.velocity_linear_solver,self.pressure_linear_solver, self.ReformDofAtEachIteration, self.vel_toll, self.press_toll, self.max_vel_its, self.max_press_its, self.time_order, self.domain_size,self.predictor_corrector)

        (self.fluid_neigh_finder).Execute();
        (self.Pfem2_apply_bc_process).Execute();  

        #for node in self.fluid_model_part.Nodes:
        #    node.SetSolutionStepValue(IS_FREE_SURFACE,0,0.0)

        self.mark_structure_process.Execute()
        self.mark_fluid_process.Execute()

        (self.save_structure_model_part_process).SaveStructure(self.fluid_model_part, self.structure_model_part);
        print("STRUCTURE PART!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!", self.structure_model_part)
        self.hypoelastic_solid_stress_tensor_calculate_process.Execute()
        #because of the changes in trigen_pfem_refine, where is_interface is used to identify the structure
        for node in self.fluid_model_part.Nodes:
            if(node.GetSolutionStepValue(IS_STRUCTURE)==1.0):
               node.SetSolutionStepValue(IS_INTERFACE,1.0)
        self.RemeshAux(); #solo multifluido
        
    def Solve(self):
        (self.solver).Solve()
        self.hypoelastic_solid_stress_tensor_calculate_process.Execute()


    


    def Clear(self):
        (self.solver).Clear()
        self.slip_conditions_initialized = True




    def RemeshAux(self):
        ((self.combined_model_part).Elements).clear();
        ((self.combined_model_part).Conditions).clear();
        ((self.combined_model_part).Nodes).clear();

        ((self.fluid_model_part).Elements).clear();
        ((self.fluid_model_part).Conditions).clear();
        
        h_factor=0.1 
        alpha_shape=1.4;

        
        for node in (self.fluid_model_part).Nodes:
            node.Set(TO_ERASE, False)

        for node in (self.fluid_model_part).Nodes: 
            node.SetSolutionStepValue(NODAL_H,0,0.0011) 

        if(self.domain_size == 2):

            for node in (self.fluid_model_part).Nodes: 
                node.SetSolutionStepValue(NODAL_H,0,0.005) 
        
        self.node_erase_process = NodeEraseProcess(self.fluid_model_part);

        

        if(self.domain_size == 2):
        
            h_factor=0.30 
            alpha_shape=5.0;

        self.Pfem2Utils.MarkLonelyNodesForErasing(self.fluid_model_part)

        
        (self.mark_outer_nodes_process).MarkOuterNodes(self.box_corner1, self.box_corner2);
        
        self.node_erase_process.Execute()
        

        if (self.domain_size == 2):
            (self.Mesher).ReGenerateMesh("FSFluid2D","Condition2D", self.fluid_model_part, self.node_erase_process, False, False, alpha_shape, h_factor)
        elif (self.domain_size == 3):
            
            (self.Mesher).ReGenerateMesh("QFluid3D","Condition3D", self.fluid_model_part, self.node_erase_process, True, False, alpha_shape, h_factor)            

        
        (self.fluid_neigh_finder).Execute();
        (self.elem_neighbor_finder).Execute()
        (self.condition_neigh_finder).Execute();

        (self.Pfem2_apply_bc_process).Execute();
        (self.mark_fluid_process).Execute();

        # merging the structural elements back (they are saved in the Initialize)
        (self.merge_model_parts_process).MergeParts(self.fluid_model_part, self.structure_model_part, self.combined_model_part);
        print("Combined model part is...")
        print (self.combined_model_part)
        (self.combined_neigh_finder).Execute();
        
    def FindNeighbours(self):
        (self.neigh_finder).Execute();
   

    def WriteRestartFile(self,FileName):
        restart_file = open(FileName + ".mdpa",'w')
        import new_restart_utilities
        new_restart_utilities.PrintProperties(restart_file)
        new_restart_utilities.PrintNodes(self.combined_model_part.Nodes,restart_file)
        new_restart_utilities.PrintElements("Fluid2DGLS",self.combined_model_part.Elements,restart_file)
        new_restart_utilities.PrintRestart_ScalarVariable(PRESSURE,"PRESSURE",self.combined_model_part.Nodes,restart_file)
        new_restart_utilities.PrintRestart_ScalarVariable(VISCOSITY,"VISCOSITY",self.combined_model_part.Nodes,restart_file)
        new_restart_utilities.PrintRestart_ScalarVariable(DENSITY,"DENSITY",self.combined_model_part.Nodes,restart_file)
        new_restart_utilities.PrintRestart_ScalarVariable(TEMPERATURE,"TEMPERATURE",self.combined_model_part.Nodes,restart_file)
        new_restart_utilities.PrintRestart_ScalarVariable(IS_STRUCTURE,"IS_STRUCTURE",self.combined_model_part.Nodes,restart_file)
        new_restart_utilities.PrintRestart_ScalarVariable(IS_FREE_SURFACE,"IS_FREE_SURFACE",self.combined_model_part.Nodes,restart_file)
        new_restart_utilities.PrintRestart_ScalarVariable(IS_BOUNDARY,"IS_BOUNDARY",self.combined_model_part.Nodes,restart_file)
        restart_file.close() 

     









