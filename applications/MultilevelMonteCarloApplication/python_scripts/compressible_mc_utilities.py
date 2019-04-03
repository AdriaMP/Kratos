from __future__ import absolute_import, division # makes KratosMultiphysics backward compatible with python 2.6 and 2.7

# Importing the Kratos Library
import KratosMultiphysics

# Import packages
import numpy as np
from scipy.stats import norm
from math import *
import copy
import time

# Importing the analysis stage classes of the different problems
from simulation_definition import SimulationScenario

# Import the StatisticalVariable class
from KratosMultiphysics.MultilevelMonteCarloApplication.compressible_statistical_variable_utilities import StatisticalVariable

# Import random variable generator
import KratosMultiphysics.MultilevelMonteCarloApplication.compressible_generator_utilities as generator

# Import exaqute
from exaqute.ExaquteTaskPyCOMPSs import *   # to execute with pycompss
# from exaqute.ExaquteTaskHyperLoom import *  # to execute with the IT4 scheduler
# from exaqute.ExaquteTaskLocal import *      # to execute with python3
"""
get_value_from_remote is the equivalent of compss_wait_on: a synchronization point
in future, when everything is integrated with the it4i team, importing exaqute.ExaquteTaskHyperLoom you can launch your code with their scheduler instead of BSC
"""

# Import cpickle to pickle the serializer
try:
    import cpickle as pickle  # Use cPickle on Python 2.7
except ImportError:
    import pickle

"""
This utility contains the functions to perform the Monte Carlo (CMLMC) algorithm
References:
M. Pisaroni, F. Nobile, P. Leyland; A Continuation Multi Level Monte Carlo (C-MLMC) method for uncertainty quantification in compressible inviscid aerodynamics; Computer Methods in Applied Mechanics and Engineering, vol 326, pp 20-50, 2017. DOI : 10.1016/j.cma.2017.07.030.
M. Pisaroni, S. Krumscheid, F. Nobile; Quantifying uncertain system outputs via the multilevel Monte Carlo method - Part I: Central moment estimation ;  available as MATHICSE technical report no. 23.2017
C. Bayer, H. Hoel, E. von Schwerin, R. Tempone; On NonAsymptotyc optimal stopping criteria in Monte Carlo simulations; avaiable at  SIAM Journal on Scientific Computing, 2014, Vol. 36, No. 2 : pp. A869-A885
"""


# TODO: check in CheckConvergence if I can use only sample variance-h2 and not two of them


"""
auxiliary function of AddResults of the MonteCarlo class
input:  simulation_results: an instance of the monte carlo result class
        level:              working level
output: QoI_value: qoi value to be added
"""
@ExaquteTask(returns=1,priority=True)
def AddResultsAux_Task(level,old_values,*simulation_results):
    """
    if (level == 0):
        # each value is inside the relative level list, and only one value per level is computed
        # i.e. results = [[value_level_0],[value_level_1],...]
        QoI_value = simulation_results.QoI[level][0]
    else:
        raise Exception("level not equal to 0, in MC we should have only level zero")
    return QoI_value
    """
    old_values.extend(
        list(map(lambda x: x.QoI[level][0], simulation_results)))
    return old_values


"""
auxiliary function of CheckConvergence of the MonteCarlo class
input:  current_number_samples:                   current number of samples computed
        current_mean:                             current mean
        current_sample_variance:                  current sample variance
        current_h2:                               current h2 statistics
        current_h3:                               current h3 statistics
        current_sample_central_moment_3_absolute: current third absolute central moment
        current_h4:                               current h4 statistics
        current_tol:                              current tolerance
        current_delta:                            current error probability (1 - confidence)
        convergence_criteria:                     convergence criteria exploited to check convergence
output: convergence_boolean: boolean setting if convergence is achieved
"""
@ExaquteTask(returns=1,priority=True)
def CheckConvergenceAux_Task(current_number_samples,current_mean,current_h2,current_h3,current_sample_central_moment_3_absolute,current_h4,current_tol,current_delta,convergence_criteria):
    convergence_boolean = False
    # TODO: rempve below lines later
    cphi_confidence = norm.ppf(1.0-current_delta) # confidence + error probability = 1.0
    statistical_error = cphi_confidence*np.sqrt(current_h2/current_number_samples)
    bias = 0.0 # hypothesis bias = 0 since we can't compute
    total_error = bias + statistical_error
    print("[TOT-ERR] total error = bias (hyp MC == 0) + statistical error =",total_error)
    # TODO: remove above lines later
    if(convergence_criteria == "MC_sample_variance_sequential_stopping_rule"):
        # define local variables
        current_convergence_coefficient = np.sqrt(current_number_samples) * current_tol / np.sqrt(current_h2)
        # evaluate probability of failure
        main_contribute = 2*(1-_ComputeCDFStandardNormalDistribution(current_convergence_coefficient))
        if(main_contribute < current_delta):
            convergence_boolean = True
    elif(convergence_criteria == "MC_higher_moments_sequential_stopping_rule"):
        # define local variables
        current_moment_2_coefficient = np.sqrt(current_h2)
        current_moment_3_absolute_coefficient = current_sample_central_moment_3_absolute / (current_moment_2_coefficient**3)
        current_moment_3_coefficient = current_h3 / (current_moment_2_coefficient**3)
        current_moment_4_coefficient = (current_h4 / (current_moment_2_coefficient**4)) - 3
        current_convergence_coefficient = np.sqrt(current_number_samples) * current_tol / np.sqrt(current_moment_2_coefficient)
        # evaluate probability of failure and penalty term
        main_contribute = 2 * (1 - _ComputeCDFStandardNormalDistribution(current_convergence_coefficient))
        penalty_contribute = 2 * np.minimum(4 * (2/(current_number_samples - 1) + (current_moment_4_coefficient / current_number_samples)), 1) * \
            _ComputeBoundFunction(current_convergence_coefficient,current_moment_3_absolute_coefficient) / np.sqrt(current_number_samples) + \
            (1 - np.minimum(4 * (2/(current_number_samples - 1) + (current_moment_4_coefficient / current_number_samples)), 1)) * \
            np.abs((current_number_samples * current_tol**2 / (current_moment_2_coefficient**2)) - 1) * \
            np.exp(- current_number_samples * (current_tol**2) / (current_moment_2_coefficient**2)) * np.abs(current_moment_3_coefficient) / \
            (3 * np.sqrt(2 * np.pi * current_number_samples))
        if (main_contribute + penalty_contribute < current_delta):
            convergence_boolean = True
    elif(convergence_criteria == "total_error_stopping_rule"):
        cphi_confidence = norm.ppf(1.0 - current_delta) # this stopping criteria checks total error like MLMC, thus need to use confidence and not error probability
        statistical_error = cphi_confidence*sqrt(current_h2/current_number_samples)
        bias = 0.0 # hypothesis bias = 0 since we can't compute
        total_error = bias + statistical_error
        if (total_error<current_tol):
            convergence_boolean = True
    else:
        convergence_boolean = False
        raise Exception ("The selected convergence criteria is not yet implemented, plese select one of the following: \n i)  MC_sample_variance_sequential_stopping_rule \n ii) MC_higher_moments_sequential_stopping_rule")
    return convergence_boolean


"""
auxiliary function of ExecuteInstance of the MonteCarlo class
input:  model:      serialization of the model
        parameters: serialization of the Project Parameters
output: QoI: Quantity of Interest
"""
@constraint(ComputingUnits="${computing_units_mc_execute}")
@ExaquteTask(returns=1)
def ExecuteInstanceAux_Task(pickled_model,pickled_project_parameters,current_analysis_stage,current_level):
# def ExecuteInstanceAux_Task(serialized_model,serialized_project_parameters,current_analysis_stage,current_level):
    time_0 = time.time()
    # # overwrite the old model serializer with the unpickled one
    serialized_model = pickle.loads(pickled_model)
    current_model = KratosMultiphysics.Model()
    serialized_model.Load("ModelSerialization",current_model)
    del(serialized_model)
    # # overwrite the old parameters serializer with the unpickled one
    serialized_project_parameters = pickle.loads(pickled_project_parameters)
    current_project_parameters = KratosMultiphysics.Parameters()
    serialized_project_parameters.Load("ParametersSerialization",current_project_parameters)
    del(serialized_project_parameters)
    time_1 = time.time()
    # initialize the MonteCarloResults class
    mc_results_class = MonteCarloResults(current_level)
    sample = generator.GenerateSample()
    simulation = current_analysis_stage(current_model,current_project_parameters,sample)
    simulation.Run()
    QoI = simulation.EvaluateQuantityOfInterest()
    time_2 = time.time()
    mc_results_class.QoI[current_level].append(QoI) # saving results in the corresponding list, for MC only list of level 0

    # post process times of the task
    print("\n","#"*50," TIMES EXECUTE TASK ","#"*50,"\n")
    deserialization_time = time_1 - time_0
    Kratos_run_time = time_2 - time_1
    total_task_time = time_2 - time_0
    print("[LEVEL] current level:",current_level)
    print("[TIMER] total task time:", total_task_time)
    print("[TIMER] Kratos Run time:",Kratos_run_time)
    print("[TIMER] Deserialization time:",deserialization_time)
    print("RATIOs: time of interest / total task time")
    print("[RATIO] Relative deserialization times:",deserialization_time/total_task_time)
    print("[RATIO] Relative Kratos run time:",Kratos_run_time/total_task_time)
    print("\n","#"*50," END TIMES EXECUTE TASK ","#"*50,"\n")


    return mc_results_class


class MonteCarlo(object):
    """The base class for the MonteCarlo-classes"""
    def __init__(self,custom_parameters_path,project_parameters_path,custom_analysis):
        """constructor of the MonteCarlo-Object
        Keyword arguments:
        self:                    an instance of the class
        custom_parameters_path:  settings of the Monte Carlo simulation
        project_parameters_path: path of the project parameters file
        custom_analysis:         analysis stage of the problem
        """

        # analysis: analysis stage of the current problem
        if (custom_analysis is not None):
            self.SetAnalysis(custom_analysis)
        else:
            raise Exception ("Please provide a Kratos specific application analysis stage for the current problem")
        # project_parameters_path: path to the project parameters json file
        if(project_parameters_path is not None):
            self.project_parameters_path = project_parameters_path
        else:
            raise Exception ("Please provide the path of the project parameters json file")
        # default settings of the Monte Carlo algorithm
        # tolerance:            tolerance
        # confidence:           confidence on tolerance
        # batch_size:           number of samples per batch size
        # convergence_criteria: convergence criteria to get if convergence is achieved
        default_settings = KratosMultiphysics.Parameters("""
        {
            "run_monte_carlo" : "True",
            "tolerance"  : 1e-1,
            "confidence" : 9e-1,
            "batch_size" : 50,
            "initial_number_batches" : 1,
            "convergence_criteria" : "MC_higher_moments_sequential_stopping_rule"
        }
        """)
        self.custom_parameters_path = custom_parameters_path
        self.SetXMCParameters()
        # validate and assign default parameters
        self.settings.ValidateAndAssignDefaults(default_settings)
        # convergence: boolean variable defining if MC algorithm has converged
        self.convergence = False
        # handle confidence = 1.0
        if (self.settings["confidence"].GetDouble()==1.0):
                self.settings["confidence"].SetDouble(0.999) # reduce confidence to not get +inf for cphi_confidence
        # set error probability = 1.0 - confidence on given tolerance
        self.settings.AddEmptyValue("error_probability")
        self.settings["error_probability"].SetDouble(1-self.settings["confidence"].GetDouble())
        # current_number_levels: number of levels of MC by default = 0 (we only have level 0)
        self.current_number_levels = 0
        # current_level: current level of work, current_level = 0 for MC
        self.current_level = 0
        # theta_i: splitting parameter \in (0,1), this affects bias and statistical error in the computation of the total error
        self.theta_i = None
        # TErr: total error of MC algorithm, the sum of bias and statistical error is an overestmation of the real total error
        #       TErr := \abs(E^MC[QoI] - E[QoI])"""
        self.TErr = None
        # QoI: Quantity of Interest of the considered problem
        self.QoI = StatisticalVariable(self.current_number_levels)
        # initialize all the variables of the StatisticalVariable class: MC has only one level, i.e. level 0
        self.QoI.InitializeLists(self.current_number_levels+1,self.settings["initial_number_batches"].GetInt())

        # batches_samples: total number of samples, organized in level and batches
        # batches_samples = [ [ [level0_batch1] [level1_batch1] .. ] [ [level0_batch2] [level1_batch2] ..] .. ]
        # for MC: number_samples = [ [ level0_batch1 ] [ level0_batch2 ] [ level0_batch3 ] .. ]
        self.batches_samples = []
        # number_samples: total number of samples
        # number_samples = [total_level0 total_level1 ..]
        self.number_samples = []
        # # difference_number_samples: difference between number of samples of current and previous iterations
        # self.difference_number_samples = [self.settings["batch_size"].GetInt() for _ in range (self.current_number_levels+1)]
        # # previous_number_samples: total number of samples of previous iteration
        # self.previous_number_samples = [0 for _ in range (self.current_number_levels+1)]
        # running_number_samples: total number of samples running
        self.running_number_samples = []
        # batches_launched: boolean true or false if batch launched or not
        self.batches_launched = []
        # batches_execution_finished: boolean true or false if batch finished or not
        self.batches_execution_finished = []
        # batches_analysis_finished: boolean true or false if statistical analysis of batch is finished or not
        self.batches_analysis_finished = []

        # batch_size: number of iterations of each epoch
        # TODO: for now batch_size = difference_number_samples, in future it may have flags for different behaviours
        self.batch_size = [self.settings["batch_size"].GetInt() for _ in range (self.current_number_levels+1)]
        # iteration counter
        self.iteration_counter = 0
        # set convergence criteria
        self.SetConvergenceCriteria()

        # pickled_model: serialization of model Kratos object of the problem
        self.pickled_model = None
        # pickled_project_parameters: serialization of project parameters Kratos object of the problem
        self.pickled_project_parameters = None
        # construct the pickled model and pickled project parameters of the problem
        self.is_project_parameters_pickled = False
        self.is_model_pickled = False
        self.SerializeModelParameters()

    """
    function executing the Monte Carlo algorithm
    input: self: an instance of the class
    """
    def Run(self):
        if (self.settings["run_monte_carlo"].GetString() == "True"):
            self.InitializeMCPhase()
            self.ScreeningInfoInitializeMCPhase()
            self.LaunchEpoch()
            print(self.QoI.values)
            self.FinalizeMCPhase()
            stop
            self.ScreeningInfoFinalizeMCPhase()
            while self.convergence is not True:
                self.InitializeMCPhase()
                self.ScreeningInfoInitializeMCPhase()
                self.LaunchEpoch()
                self.FinalizeMCPhase()
                self.ScreeningInfoFinalizeMCPhase()
        else:
            print("\n","#"*50,"Not running Monte Carlo algorithm","#"*50)
            pass

    """
    function running one Monte Carlo epoch
    input: self: an instance of the class
    """
    def LaunchEpoch(self):
        for batch in range (len(self.batches_samples)):
            if (self.batches_launched[batch] is not True):
                self.batches_launched[batch] = True
                batch_results = []
                for level in range (self.current_number_levels+1):
                    for instance in range (self.batches_samples[batch][level]):
                        self.running_number_samples[level] = self.running_number_samples[level] + 1
                        batch_results.append(self.ExecuteInstance())
                        self.running_number_samples[level] = self.running_number_samples[level] - 1
                self.AddResults(batch_results,batch)
                self.batches_execution_finished[batch] = True

    """
    function executing an instance of the Monte Carlo algorithm
    requires:  self.pickled_model:              pickled model
               self.pickled_project_parameters: pickled parameters
               self.current_analysis_stage:     analysis stage of the problem
    input:  self: an instance of the class
    output: MonteCarloResults class: an instance og the MonteCarloResults class
            current_level:           level of the current MC simulation (= 0)
    """
    def ExecuteInstance(self):
        # ensure working level is level 0
        current_level = self.current_level
        if (current_level != 0):
            raise Exception ("current work level must be = 0 in the Monte Carlo algorithm")
        return (ExecuteInstanceAux_Task(self.pickled_model,self.pickled_project_parameters,self.GetAnalysis(),self.current_level),current_level)
        # return (ExecuteInstanceAux_Task(self.serialized_model,self.serialized_project_parameters,self.GetAnalysis(),self.current_level),current_level)

    """
    function serializing and pickling the model and the project parameters of the problem
    the serialization-pickling process is the following:
    i)   from Model/Parameters Kratos object to StreamSerializer Kratos object
    ii)  from StreamSerializer Kratos object to pickle string
    iii) from pickle string to StreamSerializer Kratos object
    iv)  from StreamSerializer Kratos object to Model/Parameters Kratos object
    requires: self.project_parameters_path: path of the Project Parameters file
    builds: self.pickled_model:              pickled model
            self.pickled_project_parameters: pickled project parameters
    input:  self: an instance of the class
    """
    def SerializeModelParameters(self):
        with open(self.project_parameters_path,'r') as parameter_file:
            parameters = KratosMultiphysics.Parameters(parameter_file.read())
        model = KratosMultiphysics.Model()
        fake_sample = generator.GenerateSample() # only used to serialize
        simulation = self.analysis(model,parameters,fake_sample)
        simulation.Initialize()
        serialized_model = KratosMultiphysics.StreamSerializer()
        serialized_model.Save("ModelSerialization",simulation.model)
        serialized_project_parameters = KratosMultiphysics.StreamSerializer()
        serialized_project_parameters.Save("ParametersSerialization",simulation.project_parameters)
        self.serialized_model = serialized_model
        self.serialized_project_parameters = serialized_project_parameters
        # pickle dataserialized_data
        pickled_model = pickle.dumps(serialized_model, 2) # second argument is the protocol and is NECESSARY (according to pybind11 docs)
        pickled_project_parameters = pickle.dumps(serialized_project_parameters, 2)
        self.pickled_model = pickled_model
        self.pickled_project_parameters = pickled_project_parameters
        self.is_project_parameters_pickled = True
        self.is_model_pickled = True
        print("\n","#"*50," SERIALIZATION COMPLETED ","#"*50,"\n")

    """
    function reading the XMC parameters passed from json file
    input:  self: an instance of the class
    """
    def SetXMCParameters(self):
        with open(self.custom_parameters_path,'r') as parameter_file:
            parameters = KratosMultiphysics.Parameters(parameter_file.read())
        self.settings = parameters["monte_carlo"]

    """
    function defining the Kratos specific application analysis stage of the problem
    input:  self: an instance of the class
            application_analysis_stage: working analysis stage Kratos class
    """
    def SetAnalysis(self,application_analysis_stage):
        self.analysis = application_analysis_stage

    """
    function getting the Kratos specific application analysis stage of the problem previously defined
    input:  self: an instance of the class
    output: self.analysis: working analysis stage Kratos class
    """
    def GetAnalysis(self):
        if (self.analysis is not None):
            return self.analysis
        else:
            print("Please provide a Kratos specific application analysis stage for the current problem.")

    """
    function checking the convergence of the MC algorithm, with respect to the selected convergence criteria
    input:  self:  an instance of the class
            level: working level
    """
    def CheckConvergence(self,level):
        current_number_samples = self.QoI.number_samples[level]
        current_mean = self.QoI.raw_moment_1[level]
        current_h2 = self.QoI.h_statistics_2[level]
        current_h3 = self.QoI.h_statistics_3[level]
        current_sample_central_moment_3_absolute = self.QoI.central_moment_from_scratch_3_absolute[level]
        current_h4 = self.QoI.h_statistics_4[level]
        current_tol = self.settings["tolerance"].GetDouble()
        current_error_probability = self.settings["error_probability"].GetDouble() # the "delta" in [3] in the convergence criteria is the error probability
        convergence_criteria = self.convergence_criteria
        convergence_boolean = CheckConvergenceAux_Task(current_number_samples,current_mean,current_h2,\
            current_h3,current_sample_central_moment_3_absolute,current_h4,current_tol,current_error_probability,convergence_criteria)
        self.convergence = convergence_boolean

    """
    function adding QoI values to the corresponding level
    input:  self:               an instance of the class
            simulation_results: tuple=(instance of MonteCarloResults class, working level)
            batch_number      : number of working batch
            batch_size        : compute add result for with this size
    """
    def AddResults(self,simulation_results,batch_number,batch_size=1):
        """
        for simulation_result in simulation_results:
            simulation_results_class = simulation_result[0]
            current_level = simulation_result[1] # not compss future object
            if (current_level != 0):
                raise Exception ("current work level must be = 0 in the Monte Carlo algorithm")
            for lev in range (current_level+1):
                QoI_value = AddResultsAux_Task(simulation_results_class,lev)
                # update values of difference QoI and time ML per each level
                self.QoI.values[lev] = np.append(self.QoI.values[lev],QoI_value)
        """
        simulation_results_class = simulation_results[0][0]
        current_level = simulation_results[0][1]  # not compss future object
        if (current_level != 0):
            raise Exception("current work level must be = 0 in the Monte Carlo algorithm")
        simulation_results = list(map(lambda x: x[0], simulation_results))
        for i in range(0, len(simulation_results), batch_size):
            current_simulations = simulation_results[i:i+batch_size]
            self.QoI.values[batch_number][0] = AddResultsAux_Task(0,self.QoI.values[batch_number][0],*current_simulations)

    """
    function initializing the MC phase
    input:  self: an instance of the class
    """
    def InitializeMCPhase(self):
        current_level = self.current_level
        if (current_level != 0):
            raise Exception ("current work level must be = 0 in the Monte Carlo algorithm")
        # update iteration counter
        self.iteration_counter = self.iteration_counter + 1
        # update number of samples (MonteCarlo.batches_samples) and batch size
        if (self.iteration_counter == 1):
            self.batches_samples = [[self.settings["batch_size"].GetInt() for _ in range (self.current_number_levels+1)] for _ in range (self.settings["initial_number_batches"].GetInt())]
            self.number_samples = [self.settings["batch_size"].GetInt()*self.settings["initial_number_batches"].GetInt() for _ in range (self.current_number_levels+1)]
            self.running_number_samples = [0 for _ in range (self.current_number_levels+1)]
            self.batches_launched = [False for _ in range (self.settings["initial_number_batches"].GetInt())]
            self.batches_execution_finished = [False for _ in range (self.settings["initial_number_batches"].GetInt())]
            self.batches_analysis_finished = [False for _ in range (self.settings["initial_number_batches"].GetInt())]
        elif (self.iteration_counter > 1):
            """APPEND NEW LISTS TO QOI.VALUES AND QOI.power_sum_batches_1 IF LAUCHING BATCHES"""
            self.previous_number_samples[current_level] = self.batches_samples[current_level]
            self.batches_samples[current_level] = self.batches_samples[current_level] * 2 + self.previous_number_samples[current_level]
            self.difference_number_samples[current_level] = self.batches_samples[current_level] - self.previous_number_samples[current_level]
            self.UpdateBatch()
        else:
            pass

    """
    function updating batch size
    input:  self: an instance of the class
    TODO: for now batch_size = difference_number_samples, in future flags can be added to have different behaviours
    """
    def UpdateBatch(self):
        self.batch_size = copy.copy(self.difference_number_samples)

    """
    function finalizing the MC phase
    input:  self: an instance of the class
    """
    def FinalizeMCPhase(self):
        current_level = self.current_level
        if (current_level != 0):
            raise Exception ("current work level must be = 0 in the Monte Carlo algorithm")
        # update power sums batches
        for batch in range (len(self.batches_samples)):
            if (self.batches_execution_finished[batch] is True and self.batches_analysis_finished[batch] is not True): # consider batches completed and not already analysed
                print(self.QoI.values[batch])
                self.QoI.UpdateBatchesPassPowerSum(current_level,batch,batch_size=2)
                self.batches_analysis_finished[batch]


            # here update global power sums from local power sums
            self.batches_analysis_finished[batch] = True

        stop

        # update statistics of the QoI
        self.QoI.UpdateOnePassPowerSums(current_level,self.previous_number_samples[current_level],self.batches_samples[current_level])
        #for i_sample in range(self.previous_number_samples[current_level],self.batches_samples[current_level]):

        # compute the central moments we can't derive from the unbiased h statistics
        # we compute from scratch the absolute central moment because we can't retrieve it from the h statistics
        # self.QoI.central_moment_from_scratch_3_absolute_to_compute = True # by default set to true
        if (self.convergence_criteria == "MC_higher_moments_sequential_stopping_rule"):
            self.QoI.central_moment_from_scratch_3_absolute_to_compute = True
            self.QoI.ComputeSampleCentralMomentsFromScratch(current_level,self.batches_samples[current_level]) # not possible to use self.StatisticalVariable.number_samples[current_level]
                                                                                                               # inside the function because it is a pycompss.runtime.binding.Future object
        self.QoI.ComputeHStatistics(current_level)
        self.QoI.ComputeSkewnessKurtosis(current_level)
        self.CheckConvergence(current_level)
        # synchronization point needed to launch new tasks if convergence is false
        # put the synchronization point as in the end as possible
        self.convergence = get_value_from_remote(self.convergence)
        # bring to master what is needed to print
        self.batches_samples = get_value_from_remote(self.batches_samples)
        self.QoI.h_statistics_1 = get_value_from_remote(self.QoI.h_statistics_1)
        self.QoI.h_statistics_2 = get_value_from_remote(self.QoI.h_statistics_2)

    """
    function printing informations about initializing MLMC phase
    input:  self: an instance of the class
    """
    def ScreeningInfoInitializeMCPhase(self):
        print("\n","#"*50," MC iter =  ",self.iteration_counter,"#"*50,"\n")

    """
    function printing informations about finalizing MC phase
    input:  self: an instance of the class
    """
    def ScreeningInfoFinalizeMCPhase(self):
        # print("values computed of QoI = ",self.QoI.values)
        print("samples computed in this iteration",self.difference_number_samples)
        print("current number of samples = ",self.batches_samples)
        print("previous number of samples = ",self.previous_number_samples)
        print("monte carlo mean and variance QoI estimators = ",self.QoI.h_statistics_1,self.QoI.h_statistics_2)
        print("convergence = ",self.convergence)

    """
    function setting the convergence criteria the algorithm will exploit
    input:  self: an instance of the class
    """
    def SetConvergenceCriteria(self):
        convergence_criteria = self.settings["convergence_criteria"].GetString()
        if (convergence_criteria != "MC_sample_variance_sequential_stopping_rule" and convergence_criteria != "MC_higher_moments_sequential_stopping_rule" and convergence_criteria != "total_error_stopping_rule"):
            raise Exception ("The selected convergence criteria is not yet implemented, plese select one of the following: \n i)  MC_sample_variance_sequential_stopping_rule \n ii) MC_higher_moments_sequential_stopping_rule")
        self.convergence_criteria = convergence_criteria


"""
auxiliary function of CheckConvergence for the MC_higher_moments_sequential_stopping_rule criteria
input:  x:    parameter of the function
        beta: parameter of the function
"""
def _ComputeBoundFunction(x,beta):
    return np.minimum(0.3328 * (beta + 0.429), 18.1139 * beta / (1 + (np.abs(x)**3)))


"""
function computing the cumulative distribution function of the standard normal distribution
input:  x: probability that real-valued random variable X, or just distribution function of X, will take a value less than or equal to x
"""
def _ComputeCDFStandardNormalDistribution(x):
    # cumulative distribution function (CDF) for the standard normal distribution
    return (1.0 + erf(x / sqrt(2.0))) / 2.0


class MonteCarloResults(object):
    """The base class for the MonteCarloResults-classes"""
    def __init__(self,number_levels):
        """constructor of the MonteCarloResults-Object
        Keyword arguments:
        self: an instance of the class
        """
        # Quantity of Interest
        self.QoI = [[] for _ in range (number_levels+1)]
        # time cost
        self.time_ML = [[] for _ in range (number_levels+1)]
        # level of QoI and time_ML
        self.finer_level = number_levels
