from __future__ import print_function, absolute_import, division #makes KratosMultiphysics backward compatible with python 2.6 and 2.7
# Importing the Kratos Library
import KratosMultiphysics
import KratosMultiphysics.IgaApplication as KratosIga


def Factory(settings, model):
    if(type(settings) != KratosMultiphysics.Parameters):
        raise Exception("Expected input shall be a Parameters object, encapsulating a json string")
    return IGA_IO_Process(model, settings["Parameters"])

class IGA_IO_Process(KratosMultiphysics.Process):
  
    def __init__(self, model, params):

        KratosMultiphysics.Process.__init__(self)

        ## Settings string in json format
        default_parameters = KratosMultiphysics.Parameters("""
        {
            "nodal_results"            : [],
            "gauss_point_results": [],
            "output_file_name"         : "",
            "model_part_name"          : "",
            "file_label"               : "step",
            "output_control_type"      : "step",
            "output_frequency"         : 1.0
        }
        """)
        
        ## Overwrite the default settings with user-provided parameters
        self.params = params
        self.params.ValidateAndAssignDefaults(default_parameters)

        self.model_part = model[self.params["model_part_name"].GetString()]

        self.output_file_name = self.params["output_file_name"].GetString()

        self.nodal_results_scalar, self.nodal_results_vector = \
            CreateVariablesListFromInput(self.params["nodal_results"])

        self.integration_point_results_scalar, self.integration_point_results_vector = \
            CreateVariablesListFromInput(self.params["gauss_point_results"])

        # Set up output frequency and format
        output_file_label = self.params["file_label"].GetString()
        if output_file_label == "time":
            self.output_label_is_time = True
        elif output_file_label == "step":
            self.output_label_is_time = False
        else:
            msg = "{0} Error: Unknown value \"{1}\" read for parameter \"{2}\"".format(self.__class__.__name__,output_file_label,"file_label")
            raise Exception(msg)

        output_control_type = self.params["output_control_type"].GetString()
        if output_control_type == "time":
            self.output_control_is_time = True
        elif output_control_type == "step":
            self.output_control_is_time = False
        else:
            msg = "{0} Error: Unknown value \"{1}\" read for parameter \"{2}\"".format(self.__class__.__name__,output_file_label,"file_label")
            raise Exception(msg)

        self.frequency = self.params["output_frequency"].GetDouble()

        self.step_count = 0
        self.printed_step_count = 0
        self.next_output = 0.0

        print("initializeProcess IGA_IO_PROCESS")

    def ExecuteInitialize(self):
        with open(self.output_file_name, 'w') as output_file:
            output_file.write("Rhino Post Results File 1.0\n") 

    def ExecuteFinalizeSolutionStep(self):
        print("jaa")
        if this._IsOutputStep:
            # Print the output
            time = self.__get_pretty_time(self.model_part.ProcessInfo[TIME])
            self.printed_step_count += 1
            self.model_part.ProcessInfo[PRINTED_STEP] = self.printed_step_count
            if self.output_label_is_time:
                label = time
            else:
                label = self.printed_step_count

            with open(self.output_file_name, 'a') as output_file:
                for variable in range(self.nodal_results_scalar.size()):
                    output_file.write("Result \"" + variable.Name() + "\" \"Load Case\" " + str(label) + " Scalar OnNodes\n" + "Values\n")
                    for node in self.model_part.Nodes:
                        value = node.GetSolutionStepValue(variable, 0)
                        output_file.write(str(node.Id) + "  " + str(value) + "\n")
                    output_file.write("End Values\n")

                for variable in range(self.nodal_results_vector.size()):
                    output_file.write("Result \"" + variable.Name() + "\" \"Load Case\" " + str(label) + " Vector OnNodes\n" + "Values\n")
                    for node in self.model_part.Nodes:
                        value = node.GetSolutionStepValue(variable, 0)
                        output_file.write(str(node.Id) + "  " + str(value[0]) + "  " +  str(value[1]) + "  " + str(value[2]) + "\n")
                    output_file.write("End Values\n")

                for variable in range(self.integration_point_results_scalar.size()):
                    output_file.write("Result \"" + variable.Name() + "\" \"Load Case\" " + str(label) + " Scalar OnGaussPoints\n" + "Values\n")
                    for element in self.model_part.Elements:
                        value = element.Calculate(variable, self.model_part.ProcessInfo)
                        output_file.write(str(element.Id) + "  " + str(value) + "\n")
                    output_file.write("End Values\n")

                for variable in range(self.integration_point_results_vector.size()):
                    output_file.write("Result \"" + variable.Name() + "\" \"Load Case\" " + str(label) + " Vector OnGaussPoints\n" + "Values\n")
                    for element in self.model_part.Elements:
                        value = element.Calculate(variable, self.model_part.ProcessInfo)
                        output_file.write(str(element.Id) + "  " + str(value[0]) + "  " +  str(value[1]) + "  " + str(value[2]) + "\n")
                    output_file.write("End Values\n")

        # Schedule next output
        if self.output_frequency > 0.0: # Note: if == 0, we'll just always print
            if self.output_control_is_time:
                while self.__get_pretty_time(self.next_output) <= time:
                    self.next_output += self.output_frequency
            else:
                while self.next_output <= self.step_count:
                    self.next_output += self.output_frequency


    def _IsOutputStep(self):
        if self.output_control_is_time:
            time = self.__get_pretty_time(self.model_part.ProcessInfo[TIME])
            return (time >= self.__get_pretty_time(self.next_output))
        else:
            return ( self.step_count >= self.next_output )

    def __get_pretty_time(self,time):
        pretty_time = "{0:.12g}".format(time)
        pretty_time = float(pretty_time)
        return pretty_time

def CreateVariablesListFromInput(param):
    scalar_variables = []
    vector_variables = []
    '''Parse a list of variables from input.'''
    # Retrieve variable name from input (a string) and request the corresponding C++ object to the kernel
    for i in range(param.size()):
        variable = KratosMultiphysics.KratosGlobals.GetVariable(param[i].GetString())
        var_type = KratosMultiphysics.KratosGlobals.GetVariableType(variable.Name())
        if var_type in ["Matrix", "Flag", "NONE"]:
            raise Exception("unsupported variables type")
        elif var_type in ["Array", "Vector"]:
            vector_variables.append(variable)
        else:
            scalar_variables.append(variable)

    return scalar_variables, vector_variables
