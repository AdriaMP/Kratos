from __future__ import print_function, absolute_import, division
import KratosMultiphysics

import KratosMultiphysics.StructuralMechanicsApplication as StructuralMechanicsApplication
import KratosMultiphysics.KratosUnittest as KratosUnittest

from math import sqrt, sin, cos, pi, exp, atan

class TestComputeCenterOfGravity(KratosUnittest.TestCase):
    #KratosMultiphysics.Logger.GetDefaultOutput().SetSeverity(KratosMultiphysics.Logger.Severity.WARNING)

    def _add_dofs(self,mp):
        # Adding dofs AND their corresponding reactions
        KratosMultiphysics.VariableUtils().AddDof(KratosMultiphysics.DISPLACEMENT_X, KratosMultiphysics.REACTION_X,mp)
        KratosMultiphysics.VariableUtils().AddDof(KratosMultiphysics.DISPLACEMENT_Y, KratosMultiphysics.REACTION_Y,mp)
        KratosMultiphysics.VariableUtils().AddDof(KratosMultiphysics.DISPLACEMENT_Z, KratosMultiphysics.REACTION_Z,mp)
        KratosMultiphysics.VariableUtils().AddDof(KratosMultiphysics.ROTATION_X, KratosMultiphysics.REACTION_MOMENT_X,mp)
        KratosMultiphysics.VariableUtils().AddDof(KratosMultiphysics.ROTATION_Y, KratosMultiphysics.REACTION_MOMENT_Y,mp)
        KratosMultiphysics.VariableUtils().AddDof(KratosMultiphysics.ROTATION_Z, KratosMultiphysics.REACTION_MOMENT_Z,mp)

    def _add_variables(self,mp):
        mp.AddNodalSolutionStepVariable(KratosMultiphysics.DISPLACEMENT)
        mp.AddNodalSolutionStepVariable(KratosMultiphysics.REACTION)
        mp.AddNodalSolutionStepVariable(KratosMultiphysics.ROTATION)
        mp.AddNodalSolutionStepVariable(KratosMultiphysics.REACTION_MOMENT)

    

    def _apply_shell_material_properties(self,mp):
        #define properties
        mp.GetProperties()[1].SetValue(KratosMultiphysics.YOUNG_MODULUS,100e3)
        mp.GetProperties()[1].SetValue(KratosMultiphysics.POISSON_RATIO,0.3)
        mp.GetProperties()[1].SetValue(KratosMultiphysics.THICKNESS,1.0)
        mp.GetProperties()[1].SetValue(KratosMultiphysics.DENSITY,1.0)

        cl = StructuralMechanicsApplication.LinearElasticPlaneStress2DLaw()

        mp.GetProperties()[1].SetValue(KratosMultiphysics.CONSTITUTIVE_LAW,cl)

    def _apply_orthotropic_shell_material_properties(self,mp):
        #define properties
        # we specify only the properties we need (others are youngs modulus etc)
        num_plies = 3
        orthotropic_props = KratosMultiphysics.Matrix(num_plies,16)
        for row in range(num_plies):
            for col in range(16):
                orthotropic_props[row,col] = 0.0

        # Orthotropic mechanical moduli
        orthotropic_props[0,0] = 0.005 # lamina thickness
        orthotropic_props[0,2] = 2200  # density
        orthotropic_props[1,0] = 0.01  # lamina thickness
        orthotropic_props[1,2] = 1475  # density
        orthotropic_props[2,0] = 0.015 # lamina thickness
        orthotropic_props[2,2] = 520   # density

        mp.GetProperties()[1].SetValue(StructuralMechanicsApplication.SHELL_ORTHOTROPIC_LAYERS,orthotropic_props)

        cl = StructuralMechanicsApplication.LinearElasticOrthotropic2DLaw()

        mp.GetProperties()[1].SetValue(KratosMultiphysics.CONSTITUTIVE_LAW,cl)

    

    def _create_shell_nodes_4node_elem(self,mp):
        mp.CreateNewNode(6,0.0,0.0,0.0)
        mp.CreateNewNode(7,4.8,0.0,1.8)
        mp.CreateNewNode(8,5.9,0.0,2.5)
        mp.CreateNewNode(9,0.0,1.2,0.0)
        mp.CreateNewNode(10,4.8,1.2,1.8)
        mp.CreateNewNode(11,5.9,1.2,2.5)
        mp.CreateNewNode(12,0.0,3.7,0.0)
        mp.CreateNewNode(13,4.8,3.7,1.8)
        mp.CreateNewNode(14,5.9,3.7,2.5)

    def _create_shell_elements_4nodes(self,mp,element_name = "ShellThinElementCorotational3D4N"):
        mp.CreateNewElement(element_name, 5, [6,7,10,9], mp.GetProperties()[1])
        mp.CreateNewElement(element_name, 6, [7,8,11,10], mp.GetProperties()[1])
        mp.CreateNewElement(element_name, 7, [9,10,13,12], mp.GetProperties()[1])
        mp.CreateNewElement(element_name, 8, [10,11,14,13], mp.GetProperties()[1])

    def _set_and_fill_buffer(self,mp,buffer_size,delta_time):
        # Set buffer size
        mp.SetBufferSize(buffer_size)

        # Fill buffer
        time = mp.ProcessInfo[KratosMultiphysics.TIME]
        time = time - delta_time * (buffer_size)
        mp.ProcessInfo.SetValue(KratosMultiphysics.TIME, time)
        for size in range(0, buffer_size):
            step = size - (buffer_size -1)
            mp.ProcessInfo.SetValue(KratosMultiphysics.STEP, step)
            time = time + delta_time
            #delta_time is computed from previous time in process_info
            mp.CloneTimeStep(time)

        mp.ProcessInfo[KratosMultiphysics.IS_RESTARTED] = False

  

    def test_orthotropic_shell_mass(self):
        dim = 3
        current_model = KratosMultiphysics.Model()
        mp = current_model.CreateModelPart("structural_part")
        mp.ProcessInfo[KratosMultiphysics.DOMAIN_SIZE] = dim
        mp.SetBufferSize(2)

        self._add_variables(mp)
        self._apply_orthotropic_shell_material_properties(mp)
        self._create_shell_nodes_4node_elem(mp)
        self._add_dofs(mp)
        self._create_shell_elements_4nodes(mp)

        mass_process = StructuralMechanicsApplication.ComputeCenterOfGravityProcess(mp)
        print("printing cog for orthotropic shell mass")
        mass_process.Execute()
        COG_test_4nodes = mp.ProcessInfo[StructuralMechanicsApplication.CENTER_OF_GRAVITY]
        self.assertAlmostEqual(2.99816, COG_test_4nodes[0], 5)
        self.assertAlmostEqual(1.85, COG_test_4nodes[1] , 5)
        self.assertAlmostEqual(1.153458, COG_test_4nodes[2] , 5)

       

if __name__ == '__main__':
    KratosUnittest.main()


