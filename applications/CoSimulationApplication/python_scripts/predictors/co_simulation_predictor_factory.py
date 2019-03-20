from __future__ import print_function, absolute_import, division  # makes these scripts backward compatible with python 2.6 and 2.7

def CreatePredictor(predictor_settings, solvers):
    """This function creates and returns the Predictor used for CoSimulation
    The predictor-module has to be on the PYTHONPATH
    Naming-Convention: The module-file has to end with "_predictor"
    """
    if (type(predictor_settings) != dict):
        raise Exception("Input is expected to be provided as a python dictionary")

    module_name = predictor_settings["type"]
    module_full = "KratosMultiphysics.CoSimulationApplication.predictors."+module_name
    module_full += "_predictor"

    predictor_module = __import__(module_full,fromlist=[module_name])
    return predictor_module.Create(predictor_settings, solvers)
