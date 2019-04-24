from __future__ import print_function, absolute_import, division  # makes these scripts backward compatible with python 2.6 and 2.7

# Importing the base class
from KratosMultiphysics.CoSimulationApplication.base_classes.co_simulation_base_predictor import CosimulationBasePredictor

# Other imports
import numpy as np
import KratosMultiphysics.CoSimulationApplication.co_simulation_tools as cs_tools

# Predictor implemented according to:
# "A new staggered scheme for fluid-structure interaction"; W.G. Dettmer and D. Peric
# Numerical Methods in Engineering 2013; 93; 1-22

def Create(predictor_settings, solver):
    return AverageValuePredictor(predictor_settings, solver)

class AverageValuePredictor(CosimulationBasePredictor):
    # @param beta factor for weighting last and current value of the predicted values. Can be set in interval: [0, 1.0]
    def __init__(self, settings, solver):
        super(AverageValuePredictor, self).__init__(settings, solver)
        if "beta" in self.settings:
            self.beta = self.settings["beta"]
            if self.beta > 1.0 or self.beta < 0:
                raise Exception("Wrong value for beta. Admissible interval [0.0, 1.0]")
        else:
            self.beta = 0.5
        # TODO check buffer_size

    def Predict(self):
        current_data  = self.interface_data.GetNumpyArray(0)
        previous_data = self.interface_data.GetNumpyArray(1)

        self.predicted_data = 2*current_data - previous_data

        self._UpdateData(self.predicted_data)

    def FinalizeSolutionStep(self):
        current_data  = self.interface_data.GetNumpyArray(0)

        self.predicted_data = self.beta * current_data + (1-self.beta) * self.predicted_data

        self._UpdateData(self.predicted_data)


    def _Name(self):
        return self.__class__.__name__
