# This script couples DEM and Nvidia Flex...

from __future__ import print_function, absolute_import, division #makes KratosMultiphysics backward compatible with python 2.6 and 2.7

import sys

# Kratos
from KratosMultiphysics import *
from KratosMultiphysics.DEMApplication import *
from KratosMultiphysics.NvidiaFlexApplication import *

import main_script

class Solution(main_script.Solution):

    def Run(self):

        self.nvidia_flex_wrapper = FlexWrapper(self.spheres_model_part, self.rigid_face_model_part, self.creator_destructor)
        super(Solution, self).Run()

    def SolverSolve(self):
        if self.step < 2:
            self._CheckNvidiaParameters()
            self.nvidia_flex_wrapper.UpdateFlex()

        self.nvidia_flex_wrapper.SolveTimeSteps(self.dt, 1) #DO NOT CHANGE THIS 1, OR INSTABILITIES MAY APPEAR
        self.nvidia_flex_wrapper.TransferDataFromFlexToKratos()

    def _CheckNvidiaParameters(self):
        min_time_step = 1e-3
        if self.DEM_parameters["MaxTimeStep"].GetDouble()< min_time_step:
            Logger.PrintWarning("NVIDIA APP", "Too small time step. Please use values over", str(min_time_step), ". Exiting.")
            sys.exit()

if __name__=="__main__":
    Solution().Run()